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

/**
 * @file simulate_aig.hpp
 *
 * @brief AIG simuation
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef SIMULATE_AIG_HPP
#define SIMULATE_AIG_HPP

#include <functional>
#include <map>
#include <unordered_map>

#include <boost/graph/depth_first_search.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <core/properties.hpp>
#include <core/utils/timer.hpp>
#include <classical/aig.hpp>
#include <classical/utils/aig_dfs.hpp>
#include <classical/utils/aig_utils.hpp>
#include <classical/utils/truth_table_utils.hpp>

#include <cuddObj.hh>

namespace cirkit
{

/******************************************************************************
 * Abstract class for simulators                                              *
 ******************************************************************************/

template<typename T>
class aig_simulator
{
public:
  /**
   * @brief Simulator routine when input is encountered
   *
   * @param node AIG node reference in the `aig' graph
   * @param name Name of that node
   * @param pos  Position of that node (usually wrt. to `aig' inputs vector)
   * @param aig  AIG graph
   */
  virtual T get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const = 0;
  virtual T get_constant() const = 0;
  virtual T invert( const T& v ) const = 0;
  virtual T and_op( const aig_node& node, const T& v1, const T& v2 ) const = 0;

  virtual bool terminate( const aig_node& node, const aig_graph& aig ) const
  {
    return false;
  }
};

/******************************************************************************
 * Several simulator implementations                                          *
 ******************************************************************************/

class pattern_simulator : public aig_simulator<bool>
{
public:
  pattern_simulator( const boost::dynamic_bitset<>& pattern );

  bool get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool and_op( const aig_node& node, const bool& v1, const bool& v2 ) const;

private:
  boost::dynamic_bitset<> pattern;
};

class simple_assignment_simulator : public aig_simulator<bool>
{
public:
  using aig_name_value_map = std::unordered_map<std::string, bool>;

  simple_assignment_simulator( const aig_name_value_map& assignment );

  bool get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool and_op( const aig_node& node, const bool& v1, const bool& v2 ) const;

private:
  const aig_name_value_map& assignment;
};

class simple_node_assignment_simulator : public aig_simulator<bool>
{
public:
  using aig_node_value_map = std::unordered_map<aig_node, bool>;

  simple_node_assignment_simulator( const aig_node_value_map& assignment );

  bool get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  bool get_constant() const;
  bool invert( const bool& v ) const;
  bool and_op( const aig_node& node, const bool& v1, const bool& v2 ) const;
  bool terminate( const aig_node& node, const aig_graph& aig ) const;

private:
  const aig_node_value_map& assignment;
};

class word_assignment_simulator : public aig_simulator<boost::dynamic_bitset<>>
{
public:
  using aig_name_value_map = std::unordered_map<std::string, boost::dynamic_bitset<>>;

  word_assignment_simulator( const aig_name_value_map& assignment );

  boost::dynamic_bitset<> get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  boost::dynamic_bitset<> get_constant() const;
  boost::dynamic_bitset<> invert( const boost::dynamic_bitset<>& v ) const;
  boost::dynamic_bitset<> and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const;

private:
  const aig_name_value_map& assignment;
};

class word_node_assignment_simulator : public aig_simulator<boost::dynamic_bitset<>>
{
public:
  using aig_node_value_map = std::unordered_map<aig_node, boost::dynamic_bitset<>>;

  word_node_assignment_simulator( const aig_node_value_map& assignment );

  boost::dynamic_bitset<> get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  boost::dynamic_bitset<> get_constant() const;
  boost::dynamic_bitset<> invert( const boost::dynamic_bitset<>& v ) const;
  boost::dynamic_bitset<> and_op( const aig_node& node, const boost::dynamic_bitset<>& v1, const boost::dynamic_bitset<>& v2 ) const;
  bool terminate( const aig_node& node, const aig_graph& aig ) const;

private:
  const aig_node_value_map& assignment;
};

class tt_simulator : public aig_simulator<tt>
{
public:
  tt get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  tt get_constant() const;
  tt invert( const tt& v ) const;
  tt and_op( const aig_node& node, const tt& v1, const tt& v2 ) const;
};

class bdd_simulator : public aig_simulator<BDD>
{
public:
  bdd_simulator() : mgr( Cudd() ) {}
  bdd_simulator( const Cudd& mgr ) : mgr( mgr ) {}

  BDD get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  BDD get_constant() const;
  BDD invert( const BDD& v ) const;
  BDD and_op( const aig_node& node, const BDD& v1, const BDD& v2 ) const;

protected:
  Cudd mgr;
};

class depth_simulator : public aig_simulator<unsigned>
{
public:
  unsigned get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const;
  unsigned get_constant() const;
  unsigned invert( const unsigned& v ) const;
  unsigned and_op( const aig_node& node, const unsigned& v1, const unsigned& v2 ) const;
};

template<typename T>
class partial_simulator : public aig_simulator<T>
{
public:
  partial_simulator( const aig_simulator<T>& total_simulator,
                     const std::map<aig_node, T>& assignment,
                     const aig_graph& aig )
    : total_simulator( total_simulator ),
      assignment( assignment )
  {
    for ( const auto& n : aig_info( aig ).inputs )
    {
      if ( assignment.find( n ) == assignment.end() )
      {
        real_inputs.push_back( n );
      }
    }
  }

  T get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    /* Is in assignment? */
    auto it = assignment.find( node );
    if ( it != assignment.end() )
    {
      return it->second;
    }
    else
    {
      unsigned real_pos = std::distance( real_inputs.begin(), boost::find( real_inputs, node ) );
      return total_simulator.get_input( node, name, real_pos, aig );
    }
  }

  T get_constant() const { return total_simulator.get_constant(); }
  T invert( const T& v ) const { return total_simulator.invert( v ); }
  T and_op( const aig_node& node, const T& v1, const T& v2 ) const { return total_simulator.and_op( node, v1, v2 ); }

private:
  const aig_simulator<T>& total_simulator;
  const std::map<aig_node, T>& assignment;

  std::vector<aig_node> real_inputs;
};

template<typename T>
class aig_partial_node_assignment_simulator : public aig_simulator<T>
{
public:
  aig_partial_node_assignment_simulator( const aig_simulator<T>& total_simulator,
                                         const std::map<aig_node, T>& assignment,
                                         const T& default_value )
    : total_simulator( total_simulator ),
      assignment( assignment ),
      default_value( default_value ) {}

  T get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    auto it = assignment.find( node );
    if ( it == assignment.end() )
    {
      std::cout << "[w] no assignment given for '" << node << "', assume default" << std::endl;
      return default_value;
    }
    else
    {
      return it->second;
    }
  }

  T get_constant() const { return total_simulator.get_constant(); }
  T invert( const T& v ) const { return total_simulator.invert( v ); }
  T and_op( const aig_node& node, const T& v1, const T& v2 ) const
  {
    auto it = assignment.find( node );
    if ( it == assignment.end() )
    {
      return total_simulator.and_op( node, v1, v2 );
    }
    else
    {
      return it->second;
    }
  }

  bool terminate( const aig_node& node, const aig_graph& aig ) const
  {
    return assignment.find( node ) != assignment.end();
  }

private:
  const aig_simulator<T>& total_simulator;
  const std::map<aig_node, T>& assignment;
  T default_value;
};

template<typename T>
class aig_lambda_simulator : public aig_simulator<T>
{
public:
  aig_lambda_simulator( const std::function<T(const aig_node&, const std::string&, unsigned, const aig_graph&)>& get_input_func,
                        const std::function<T()>& get_constant_func,
                        const std::function<T(const T&)>& invert_func,
                        const std::function<T(const aig_node&, const T&, const T&)>& and_op_func )
    : get_input_func( get_input_func ),
      get_constant_func( get_constant_func ),
      invert_func( invert_func ),
      and_op_func( and_op_func )
  {
  }

  T get_input( const aig_node& node, const std::string& name, unsigned pos, const aig_graph& aig ) const
  {
    return get_input_func( node, name, pos, aig );
  }

  T get_constant() const
  {
    return get_constant_func();
  }

  T invert( const T& v ) const
  {
    return invert_func( v );
  }

  T and_op( const aig_node& node, const T& v1, const T& v2 ) const
  {
    return and_op_func( node, v1, v2 );
  }

private:
  std::function<T(const aig_node&, const std::string&, unsigned, const aig_graph&)> get_input_func;
  std::function<T()> get_constant_func;
  std::function<T(const T&)> invert_func;
  std::function<T(const aig_node&, const T&, const T&)> and_op_func;
};

/******************************************************************************
 * DFS visitor for actual simulation                                          *
 ******************************************************************************/

using aig_node_color_map = circuit_traits<aig_graph>::node_color_map;

template<typename T>
struct simulate_aig_node_visitor : public aig_dfs_visitor
{
public:
  simulate_aig_node_visitor( const aig_graph& aig, const aig_simulator<T>& simulator, std::map<aig_node, T>& node_values )
    : aig_dfs_visitor( aig ),
      simulator( simulator ),
      node_values( node_values ) {}

  void finish_input( const aig_node& node, const std::string& name, const aig_graph& aig )
  {
    unsigned pos = std::distance( graph_info.inputs.begin(), boost::find( graph_info.inputs, node ) );
    node_values[node] = simulator.get_input( node, name, pos, aig );
  }

  void finish_constant( const aig_node& node, const aig_graph& aig )
  {
    node_values[node] = simulator.get_constant();
  }

  void finish_aig_node( const aig_node& node, const aig_function& left, const aig_function& right, const aig_graph& aig )
  {
    T tleft, tright;

    const auto itleft  = node_values.find( left.node );
    const auto itright = node_values.find( right.node );

    if ( itleft != node_values.end() )
    {
      tleft = left.complemented ? simulator.invert( itleft->second ) : itleft->second;
    }
    if ( itright != node_values.end() )
    {
      tright = right.complemented ? simulator.invert( itright->second ) : itright->second;
    }

    node_values[node] = simulator.and_op( node, tleft, tright );
  }

private:
  const aig_simulator<T>& simulator;
  std::map<aig_node, T>& node_values;
};

/******************************************************************************
 * Methods to trigger simulation                                              *
 ******************************************************************************/

template<typename T>
T simulate_aig_node( const aig_graph& aig, const aig_node& node,
                     const aig_simulator<T>& simulator,
                     aig_node_color_map& colors,
                     std::map<aig_node, T>& node_values )
{
  boost::depth_first_visit( aig, node,
                            simulate_aig_node_visitor<T>( aig, simulator, node_values ),
                            boost::make_assoc_property_map( colors ),
                            [&simulator]( const aig_node& node, const aig_graph& aig ) { return simulator.terminate( node, aig ); } );

  return node_values[node];
}

template<typename T>
T simulate_aig_node( const aig_graph& aig, const aig_node& node,
                     const aig_simulator<T>& simulator )
{
  aig_node_color_map colors;
  std::map<aig_node, T> node_values;

  return simulate_aig_node<T>( aig, node, simulator, colors, node_values );
}

template<typename T>
T simulate_aig_function( const aig_graph& aig, const aig_function& f,
                         const aig_simulator<T>& simulator,
                         aig_node_color_map& colors,
                         std::map<aig_node, T>& node_values )
{
  T value = simulate_aig_node<T>( aig, f.node, simulator, colors, node_values );
  return f.complemented ? simulator.invert( value ) : value;
}

template<typename T>
T simulate_aig_function( const aig_graph& aig, const aig_function& f,
                         const aig_simulator<T>& simulator )
{
  T value = simulate_aig_node<T>( aig, f.node, simulator );
  return f.complemented ? simulator.invert( value ) : value;
}

template<typename T>
std::map<aig_function, T> simulate_aig( const aig_graph& aig, const aig_simulator<T>& simulator,
                                        const properties::ptr& settings = properties::ptr(),
                                        const properties::ptr& statistics = properties::ptr() )
{
  /* settings */
  auto verbose = get( settings, "verbose", false );

  /* timer */
  properties_timer t( statistics );

  aig_node_color_map colors;
  std::map<aig_node, T> node_values;

  std::map<aig_function, T> results;

  for ( const auto& o : aig_info( aig ).outputs )
  {
    if ( verbose )
    {
      std::cout << "[i] simulate '" << o.second << "'" << std::endl;
    }
    T value = simulate_aig_node<T>( aig, o.first.node, simulator, colors, node_values );

    /* value may need to be inverted */
    results[o.first] = o.first.complemented ? simulator.invert( value ) : value;
  }

  set( statistics, "node_values", node_values );

  return results;
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
