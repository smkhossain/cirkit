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
 * @author Mathias Soeken
 */

#include <iostream>
#include <thread>

#include <core/io/read_pla_to_bdd.hpp>
#include <core/utils/program_options.hpp>
#include <core/utils/timeout.hpp>
#include <core/utils/timer.hpp>

#include <reversible/truth_table.hpp>
#include <reversible/functions/approximate_additional_lines.hpp>
#include <reversible/functions/calculate_additional_lines.hpp>
#include <reversible/functions/extend_pla.hpp>
#include <reversible/io/read_pla.hpp>
#include <reversible/io/write_pla.hpp>

using namespace cirkit;

int main( int argc, char ** argv )
{
  using boost::program_options::value;

  std::string filename;
  unsigned    mode           = 0u;
  auto        post_compact   = true;
  auto        timeout        = 5000u;
  auto        dumpadd        = false;
  auto        explicit_zeros = false;
  std::string tmpname        = "/tmp/test.pla";
  std::string dotname;

  program_options opts;
  opts.add_options()
    ( "filename",       value( &filename ),                    "PLA filename" )
    ( "mode",           value_with_default( &mode ),           "Mode (0: extend, 1: BDD, 2: approximate)" )
    ( "post_compact",   value_with_default( &post_compact ),   "Compress PLA after extending (only for mode = 0)" )
    ( "timeout",        value_with_default( &timeout ),        "Timeout in seconds" )
    ( "tmpname",        value_with_default( &tmpname ),        "Temporary filename for extended PLA" )
    ( "dotname",        value( &dotname ),                     "If non-empty and mode = 1, the BDD is dumped to that file" )
    ( "dumpadd",        value_with_default( &dumpadd ),        "Dump ADD instead of BDD, if dotname is set and mode = 1" )
    ( "explicit_zeros", value_with_default( &explicit_zeros ), "Have explicit zeros in characteristic BDD, if mode = 1" )
    ( "verbose,v",                                             "Be verbose" )
    ;

  opts.parse( argc, argv );

  if ( !opts.good() || !opts.is_set( "filename" ) )
  {
    std::cout << opts << std::endl;
    return 1;
  }

  /* settings and statistics */
  unsigned additional = 0u;
  properties::ptr settings( new properties );
  settings->set( "verbose", opts.is_set( "verbose" ) );
  properties::ptr statistics( new properties );

  /* timeout */
  std::thread t1( [&timeout]() { timeout_after( timeout ); } );

  /* stop time */
  print_timer t;

  if ( mode == 0u )
  {
    binary_truth_table pla, extended;
    read_pla_settings rp_settings;
    rp_settings.extend = false;
    read_pla( pla, filename, rp_settings );
    extend_pla_settings ep_settings;
    ep_settings.post_compact = post_compact;
    ep_settings.verbose = opts.is_set( "verbose" );
    extend_pla( pla, extended, ep_settings );

    write_pla( extended, tmpname );

    additional = approximate_additional_lines( tmpname, settings, statistics );
  }
  else if ( mode == 1u )
  {
    settings->set( "dotname",        dotname );
    settings->set( "dumpadd",        dumpadd );
    settings->set( "explicit_zeros", explicit_zeros );
    additional = calculate_additional_lines( filename, settings, statistics );
  }
  else if ( mode == 2u )
  {
    additional = approximate_additional_lines( filename, settings, statistics );
  }

  std::cout << "Inputs:     " << statistics->get<unsigned>( "num_inputs" ) << std::endl
            << "Outputs:    " << statistics->get<unsigned>( "num_outputs" ) << std::endl
            << "Additional: " << additional << std::endl;

  t1.detach();

  return 0;
}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
