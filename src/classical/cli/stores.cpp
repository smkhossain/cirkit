/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stores.hpp"

#include <fstream>
#include <sstream>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/graph/depth.hpp>
#include <core/utils/range_utils.hpp>

#include <classical/functions/aig_from_truth_table.hpp>
#include <classical/functions/aig_to_mig.hpp>
#include <classical/functions/compute_levels.hpp>
#include <classical/functions/simulate_aig.hpp>
#include <classical/io/read_aiger.hpp>
#include <classical/io/read_bench.hpp>
#include <classical/io/read_symmetries.hpp>
#include <classical/io/read_unateness.hpp>
#include <classical/io/write_aiger.hpp>
#include <classical/io/write_verilog.hpp>
#include <classical/mig/mig_to_aig.hpp>
#include <classical/mig/mig_from_string.hpp>
#include <classical/mig/mig_utils.hpp>
#include <classical/mig/mig_verilog.hpp>

namespace cirkit
{

/******************************************************************************
 * aig_graph                                                                  *
 ******************************************************************************/

template<>
std::string store_entry_to_string<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );
  const auto& name = info.model_name;
  return boost::str( boost::format( "%s i/o = %d/%d" ) % ( name.empty() ? "(unnamed)" : name ) % info.inputs.size() % info.outputs.size() );
}

show_store_entry<aig_graph>::show_store_entry( command& cmd )
{
  boost::program_options::options_description aig_options( "AIG options" );

  aig_options.add_options()
    ( "levels", boost::program_options::value<unsigned>()->default_value( 0u ), "Compute and annotate levels for dot\n0: don't compute\n1: push to inputs\n2: push to outputs" )
    ;

  cmd.opts.add( aig_options );
}

bool show_store_entry<aig_graph>::operator()( aig_graph& aig, const std::string& dotname, const command& cmd )
{
  auto settings = std::make_shared<properties>();
  settings->set( "verbose", cmd.is_set( "verbose" ) );
  const auto levels = cmd.vm["levels"].as<unsigned>();
  if ( levels > 0u )
  {
    auto cl_settings = std::make_shared<properties>();
    cl_settings->set( "verbose", cmd.is_set( "verbose" ) );
    cl_settings->set( "push_to_outputs", levels == 2u );
    auto annotation = get( boost::vertex_annotation, aig );

    const auto vertex_levels = compute_levels( aig, cl_settings );
    for ( const auto& p : vertex_levels )
    {
      annotation[p.first]["level"] = std::to_string( p.second );
    }

    settings->set( "vertex_levels", boost::optional<std::map<aig_node, unsigned>>( vertex_levels ) );
  }

  write_dot( aig, dotname, settings );

  return true;
}

command::log_opt_t show_store_entry<aig_graph>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<aig_graph>( std::ostream& os, const aig_graph& aig )
{
  aig_print_stats( aig );
}

template<>
command::log_opt_t log_store_entry_statistics<aig_graph>( const aig_graph& aig )
{
  const auto& info = aig_info( aig );

  std::vector<aig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( aig, outputs, depths );

  return command::log_opt_t({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( aig ) - info.inputs.size() - 1u )},
      {"depth", static_cast<int>( depth )}});
}

template<>
aig_graph store_convert<tt, aig_graph>( const tt& t )
{
  return aig_from_truth_table( t );
}

template<>
bdd_function_t store_convert<aig_graph, bdd_function_t>( const aig_graph& aig )
{
  Cudd mgr;
  bdd_simulator simulator( mgr );
  auto values = simulate_aig( aig, simulator );

  std::vector<BDD> bdds;

  for ( const auto& o : aig_info( aig ).outputs )
  {
    bdds.push_back( values[o.first] );
  }

  return {mgr, bdds};
}

template<>
bool store_can_read_io_type<aig_graph, io_aiger_tag_t>( command& cmd )
{
  cmd.opts.add_options()
    ( "nosym",    "do not read symmetry file if existing" )
    ( "nounate",  "do not read unateness file if existing" )
    ( "nostrash", "do not strash the AIG when reading (in binary AIGER format)" )
    ;
  return true;
}

template<>
aig_graph store_read_io_type<aig_graph, io_aiger_tag_t>( const std::string& filename, const command& cmd )
{
  aig_graph aig;

  try
  {
    if ( boost::ends_with( filename, "aag" ) )
    {
      read_aiger( aig, filename );
    }
    else
    {
      read_aiger_binary( aig, filename, cmd.is_set( "nostrash" ) );
    }
  }
  catch ( const char *e )
  {
    std::cerr << e << std::endl;
    assert( false );
  }

  /* auto-find symmetry file */
  const auto symname = filename.substr( 0, filename.size() - 3 ) + "sym";
  if ( !cmd.is_set( "nosym" ) && boost::filesystem::exists( symname ) )
  {
    /* read symmetries */
    std::cout << "[i] found and read symmetries file" << std::endl;
    read_symmetries( aig, symname );
  }

  /* auto-find unateness file */
  const auto depname = filename.substr( 0, filename.size() - 3 ) + "dep";
  if ( !cmd.is_set( "nounate" ) && boost::filesystem::exists( depname ) )
  {
    /* read unateness */
    std::cout << "[i] found and read unateness dependency file" << std::endl;
    read_unateness( aig, depname );
  }

  return aig;
}

template<>
aig_graph store_read_io_type<aig_graph, io_bench_tag_t>( const std::string& filename, const command& cmd )
{
  aig_graph aig;
  read_bench( aig, filename );
  return aig;
}

template<>
void store_write_io_type<aig_graph, io_aiger_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  write_aiger( aig, filename );
}

template<>
void store_write_io_type<aig_graph, io_verilog_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  write_verilog( aig, filename );
}

template<>
void store_write_io_type<aig_graph, io_edgelist_tag_t>( const aig_graph& aig, const std::string& filename, const command& cmd )
{
  std::ofstream os( filename.c_str(), std::ofstream::out );

  for ( const auto& e : boost::make_iterator_range( edges( aig ) ) )
  {
    os << source( e, aig ) << " " << target( e, aig ) << std::endl;
  }
}

/******************************************************************************
 * mig_graph                                                                  *
 ******************************************************************************/

template<>
aig_graph store_convert<mig_graph, aig_graph>( const mig_graph& mig )
{
  return mig_to_aig( mig );
}

template<>
mig_graph store_convert<aig_graph, mig_graph>( const aig_graph& aig )
{
  return aig_to_mig( aig );
}

template<>
std::string store_entry_to_string<mig_graph>( const mig_graph& mig )
{
  const auto& info = mig_info( mig );
  const auto& name = info.model_name;
  return boost::str( boost::format( "%s i/o = %d/%d" ) % ( name.empty() ? "(unnamed)" : name ) % info.inputs.size() % info.outputs.size() );
}

show_store_entry<mig_graph>::show_store_entry( command& cmd )
{
}

bool show_store_entry<mig_graph>::operator()( mig_graph& mig, const std::string& dotname, const command& cmd )
{
  write_dot( mig, dotname );

  return true;
}

command::log_opt_t show_store_entry<mig_graph>::log() const
{
  return boost::none;
}

template<>
void print_store_entry_statistics<mig_graph>( std::ostream& os, const mig_graph& mig )
{
  mig_print_stats( mig, os );
}

template<>
command::log_opt_t log_store_entry_statistics<mig_graph>( const mig_graph& mig )
{
  const auto& info = mig_info( mig );

  std::vector<mig_node> outputs;
  for ( const auto& output : info.outputs )
  {
    outputs += output.first.node;
  }

  std::vector<unsigned> depths;
  const auto depth = compute_depth( mig, outputs, depths );

  return command::log_opt_t({
      {"inputs", static_cast<int>( info.inputs.size() )},
      {"outputs", static_cast<int>( info.outputs.size() )},
      {"size", static_cast<int>( boost::num_vertices( mig ) - info.inputs.size() - 1u )},
      {"depth", depth},
      {"complemented_edges", number_of_complemented_edges( mig )},
      {"inverters", number_of_inverters( mig )}
    });
}

template<>
expression_t::ptr store_convert<mig_graph, expression_t::ptr>( const mig_graph& mig )
{
  return mig_to_expression( mig, mig_info( mig ).outputs.front().first );
}

template<>
mig_graph store_convert<expression_t::ptr, mig_graph>( const expression_t::ptr& expr )
{
  mig_graph mig;
  mig_initialize( mig );
  std::vector<mig_function> pis;
  mig_create_po( mig, mig_from_expression( mig, pis, expr ), "f" );
  return mig;
}

template<>
void store_write_io_type<mig_graph, io_verilog_tag_t>( const mig_graph& mig, const std::string& filename, const command& cmd )
{
  write_verilog( mig, filename );
}

template<>
mig_graph store_read_io_type<mig_graph, io_verilog_tag_t>( const std::string& filename, const command& cmd )
{
  return read_mighty_verilog( filename );
}

/******************************************************************************
 * counterexample_t                                                           *
 ******************************************************************************/

template<>
std::string store_entry_to_string<counterexample_t>( const counterexample_t& cex )
{
  std::stringstream os;
  os << cex;
  return os.str();
}

/******************************************************************************
 * simple_fanout_graph_t                                                      *
 ******************************************************************************/

template<>
std::string store_entry_to_string<simple_fanout_graph_t>( const simple_fanout_graph_t& nl )
{
  return "";
}

/******************************************************************************
 * std::vector<aig_node>                                                      *
 ******************************************************************************/

template<>
std::string store_entry_to_string<std::vector<aig_node>>( const std::vector<aig_node>& g )
{
  return ( boost::format( "{ %s }" ) % any_join( g, ", " ) ).str();
}

template<>
void print_store_entry<std::vector<aig_node>>( std::ostream& os, const std::vector<aig_node>& g )
{
  os << boost::format( "{ %s }" ) % any_join( g, ", " ) << std::endl;
}

/******************************************************************************
 * tt                                                                         *
 ******************************************************************************/

template<>
std::string store_entry_to_string<tt>( const tt& t )
{
  std::stringstream os;
  os << t;
  return os.str();
}

template<>
void print_store_entry<tt>( std::ostream& os, const tt& t )
{
  os << t << std::endl;
}

/******************************************************************************
 * expression_t::ptr                                                          *
 ******************************************************************************/

template<>
std::string store_entry_to_string<expression_t::ptr>( const expression_t::ptr& expr )
{
  std::stringstream s;
  s << expr;
  return s.str();
}

template<>
void print_store_entry_statistics<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr )
{
  os << std::endl;
}

template<>
command::log_opt_t log_store_entry_statistics<expression_t::ptr>( const expression_t::ptr& expr )
{
  return command::log_opt_t({
      {"expression", expression_to_string( expr )}
    });
}


template<>
void print_store_entry<expression_t::ptr>( std::ostream& os, const expression_t::ptr& expr )
{
  os << expr << std::endl;
}

template<>
tt store_convert<expression_t::ptr, tt>( const expression_t::ptr& expr )
{
  return tt_from_expression( expr );
}

template<>
bdd_function_t store_convert<expression_t::ptr, bdd_function_t>( const expression_t::ptr& expr )
{
  Cudd manager;
  return bdd_from_expression( manager, expr );
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
