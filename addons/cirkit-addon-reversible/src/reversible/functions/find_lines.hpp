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
 * @file find_lines.hpp
 *
 * @brief Finds empty and non-empty lines in circuits and gates
 *
 * @author Mathias Soeken
 * @since  1.0
 */

#ifndef FIND_EMPTY_LINES_HPP
#define FIND_EMPTY_LINES_HPP

#include <reversible/circuit.hpp>

#include <set>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>

namespace cirkit
{

  class gate;

  /**
   * @brief Finds non empty lines in a gate
   *
   * This function inserts all lines, which are
   * either control or target line into an iterator.
   *
   * @section sec_find_empty_lines_example Example
   *
   * @code
   * set::set<unsigned> non_empty_lines;
   * revkit::find_non_empty_lines( circ[0], circ.lines(), std::insert_iterator<set::set<unsigned> >( non_empty_lines, non_empty_lines.begin() ) );
   * @endcode
   *
   * @param src       Gate
   * @param result    Iterator to add non empty lines of type gate::line
   *
   * @return The iterator after adding the lines (pointing behind the end)
   *
   * @since  1.0
   */
  template<typename Iterator>
  Iterator find_non_empty_lines( const gate& src, Iterator result )
  {
    using boost::adaptors::transformed;
    boost::copy( src.controls() | transformed( []( variable v ) { return v.line(); } ), result );
    return boost::copy( src.targets(), result );
  }

  /**
   * @brief Finds non empty lines in a range of gates
   *
   * This function finds lines in a range of gates [\p first, \p last) which
   * are used by at least one gate, meaning that they
   * are control or target line for at least one gate.
   *
   * @param first     Begin iterator of the gates (inclusive)
   * @param last      End iterator of the gates (exclusive)
   * @param result    Iterator to add non empty lines of type gate::line
   *
   * @return The iterator after adding the lines (pointing behind the end)
   *
   * @since  1.0
   */
  template<typename GateIterator, typename Iterator>
  Iterator find_non_empty_lines( GateIterator first, GateIterator last, Iterator result )
  {
    std::for_each( first, last, [&result]( const gate& g ){ find_non_empty_lines( g, result ); } );
    return result;
  }

  /**
   * @brief Finds non empty lines in a circuit
   *
   * This function finds lines in a circuit which
   * are used by at least one gate, meaning that they
   * are control or target line for at least one gate.
   *
   * @param circ   Circuit
   * @param result Iterator to add non empty lines of type gate::line
   *
   * @return The iterator after adding the lines (pointing behind the end)
   *
   * @since  1.0
   */
  template<typename Iterator>
  Iterator find_non_empty_lines( const circuit& circ, Iterator result )
  {
    return find_non_empty_lines( circ.begin(), circ.end(), result );
  }

  /**
   * @brief Finds empty lines in a gate
   *
   * This function inserts all empty lines, which are
   * neither control or target line into an iterator.
   *
   * @section sec_find_empty_lines_example Example
   *
   * @code
   * set::set<unsigned> empty_lines;
   * revkit::find_empty_lines( circ[0], circ.lines(), std::insert_iterator<set::set<unsigned> >( empty_lines, empty_lines.begin() ) );
   * @endcode
   *
   * @param src       Gate
   * @param line_size Number of lines in the gate
   * @param result    Iterator to add empty lines of type gate::line
   *
   * @return The iterator after adding the lines (pointing behind the end)
   *
   * @since  1.0
   */
  template<typename Iterator>
  Iterator find_empty_lines( const gate& src, unsigned line_size, Iterator result )
  {
    std::set<unsigned> all_lines;
    find_non_empty_lines( src, std::insert_iterator<std::set<unsigned> >( all_lines, all_lines.begin() ) );

    return std::set_difference( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( line_size ),
                                all_lines.begin(), all_lines.end(), result );
  }

  /**
   * @brief Finds empty lines in a gate
   *
   * This function inserts all empty lines, which are
   * neither control or target line into an line container.
   *
   * @section sec_find_empty_lines_example2 Example
   *
   * @code
   * set::set<unsigned> empty_lines;
   * revkit::find_empty_lines( *circ[0], circ.lines(), empty_lines );
   * @endcode
   *
   * @param src       Gate
   * @param line_size Number of lines in the gate
   * @param lines     A set::set<unsigned> to insert the empty lines
   *
   * @since  1.0
   */
  void find_empty_lines( const gate& src, unsigned line_size, std::set<unsigned>& lines );

  /**
   * @brief Finds empty lines in a range of gates
   *
   * This function finds lines in a range of gates [\p first, \p last) which
   * are never used by any gate, meaning that they
   * are no control or target line for any gate.
   *
   * @param first     Begin iterator of the gates (inclusive)
   * @param last      End iterator of the gates (exclusive)
   * @param line_size Number of lines for the gates
   * @param result    Iterator to add empty lines of type gate::line
   *
   * @return The iterator after adding the lines (pointing behind the end)
   *
   * @since  1.0
   */
  template<typename GateIterator, typename Iterator>
  Iterator find_empty_lines( GateIterator first, GateIterator last, unsigned line_size, Iterator result )
  {
    std::set<unsigned> all_lines;
    find_non_empty_lines( first, last, std::insert_iterator<std::set<unsigned> >( all_lines, all_lines.begin() ) );

    return std::set_difference( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( line_size ),
                                all_lines.begin(), all_lines.end(), result );
  }

  /**
   * @brief Finds empty lines in a circuit
   *
   * This function finds lines in a circuit which
   * are never used by any gate, meaning that they
   * are no control or target line for any gate.
   *
   * @param circ   Circuit
   * @param result Iterator to add empty lines of type gate::line
   *
   * @return The iterator after adding the lines (pointing behind the end)
   *
   * @since  1.0
   */
  template<typename Iterator>
  Iterator find_empty_lines( const circuit& circ, Iterator result )
  {
    return find_empty_lines( circ.begin(), circ.end(), circ.lines(), result );
  }
}

#endif /* FIND_EMPTY_LINES_HPP */

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
