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
 * @file foreach_function.hpp
 *
 * @brief Iterates through RevLib functions
 *
 * @author Mathias Soeken
 * @since  2.0
 */

#ifndef FOREACH_FUNCTION_HPP
#define FOREACH_FUNCTION_HPP

#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

#include <core/utils/bitset_utils.hpp>
#include <reversible/truth_table.hpp>
#include <reversible/functions/truth_table_from_bitset.hpp>
#include <reversible/io/read_pla.hpp>

namespace cirkit
{

template<typename Cond, typename _Fn>
void foreach_function_if( _Fn&& __fn, Cond&& cond )
{
  using namespace boost::filesystem;

  path p( "../ext/functions/" );

  for ( const auto& entry : boost::make_iterator_range( directory_iterator( p ), directory_iterator() ) )
  {
    if ( entry.path().extension() != ".pla" || !cond( entry ) ) continue;

    __fn( entry.path() );
  }
}

template<typename Range, typename _Fn>
void foreach_function_with_blacklist( const Range& blacklist, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_function_if( __fn, [&blacklist]( const directory_entry& e ) { return boost::find( blacklist, e.path().stem() ) == boost::end( blacklist ); } );
}

template<typename Range, typename _Fn>
void foreach_function_with_whitelist( const Range& whitelist, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_function_if( __fn, [&whitelist]( const directory_entry& e ) { return boost::find( whitelist, e.path().stem() ) != boost::end( whitelist ); } );
}

template<typename _Fn>
void foreach_function_with_max_variables( unsigned num_variables, _Fn&& __fn )
{
  using namespace boost::filesystem;
  foreach_function_if( __fn, [&num_variables]( const directory_entry& e ) {
      binary_truth_table spec;
      read_pla_settings settings;
      settings.extend = false;
      settings.skip_after_first_cube = true;
      read_pla( spec, e.path().relative_path().string(), settings );
      return ( spec.num_inputs() + spec.num_outputs() ) <= num_variables;
    });

}

/**
 * @brief Creates truth tables from irreversible functions
 *
 * The function iterates over all functions of $n$ variables and
 * creates a truth table, where the function is embedded in the
 * first column.  If the function is not balanced an additional
 * line is added, and the function is embedded at constant 0.
 */
template<typename _Fn>
void foreach_function_as_truth_table( unsigned num_variables, _Fn&& __fn )
{
  boost::dynamic_bitset<> bs( 1u << num_variables );

  do {
    __fn( bs, truth_table_from_bitset( bs ) );
    inc( bs );
  } while ( bs.any() );
}

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
