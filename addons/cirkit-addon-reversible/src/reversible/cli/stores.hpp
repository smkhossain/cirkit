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
 * @file stores.hpp
 *
 * @brief Meta-data for stores
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef STORES_HPP
#define STORES_HPP

#include <lscli/command.hpp>

#include <core/cli/stores.hpp>
#include <classical/aig.hpp>
#include <reversible/circuit.hpp>
#include <reversible/rcbdd.hpp>
#include <reversible/truth_table.hpp>

namespace cirkit
{

struct io_real_tag_t {};
struct io_spec_tag_t {};
struct io_quipper_tag_t {};
struct io_tikz_tag_t {};
struct io_qpic_tag_t {};

/******************************************************************************
 * circuit                                                                    *
 ******************************************************************************/

template<>
struct store_info<circuit>
{
  static constexpr const char* key         = "circuits";
  static constexpr const char* option      = "circuit";
  static constexpr const char* mnemonic    = "c";
  static constexpr const char* name        = "circuit";
  static constexpr const char* name_plural = "circuits";
};

template<>
std::string store_entry_to_string<circuit>( const circuit& circ );

template<>
void print_store_entry<circuit>( std::ostream& os, const circuit& circ );

template<>
void print_store_entry_statistics<circuit>( std::ostream& os, const circuit& circ );

template<>
command::log_opt_t log_store_entry_statistics<circuit>( const circuit& circ );

template<>
inline bool store_can_convert<circuit, aig_graph>() { return true; }

template<>
aig_graph store_convert<circuit, aig_graph>( const circuit& circ );

template<>
inline bool store_can_write_io_type<circuit, io_qpic_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<circuit, io_qpic_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_quipper_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_quipper_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_read_io_type<circuit, io_real_tag_t>( command& cmd );

template<>
circuit store_read_io_type<circuit, io_real_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<circuit, io_real_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<circuit, io_real_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

template<>
bool store_can_write_io_type<circuit, io_tikz_tag_t>( command& cmd );

template<>
void store_write_io_type<circuit, io_tikz_tag_t>( const circuit& circ, const std::string& filename, const command& cmd );

/******************************************************************************
 * binary_truth_table                                                         *
 ******************************************************************************/

template<>
struct store_info<binary_truth_table>
{
  static constexpr const char* key         = "spec";
  static constexpr const char* option      = "spec";
  static constexpr const char* mnemonic    = "s";
  static constexpr const char* name        = "specification";
  static constexpr const char* name_plural = "specifications";
};

template<>
std::string store_entry_to_string<binary_truth_table>( const binary_truth_table& spec );

template<>
void print_store_entry<binary_truth_table>( std::ostream& os, const binary_truth_table& spec );

template<>
inline bool store_can_convert<circuit, binary_truth_table>() { return true; }

template<>
binary_truth_table store_convert<circuit, binary_truth_table>( const circuit& circ );

template<>
bool store_can_read_io_type<binary_truth_table, io_spec_tag_t>( command& cmd );

template<>
binary_truth_table store_read_io_type<binary_truth_table, io_spec_tag_t>( const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<binary_truth_table, io_spec_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<binary_truth_table, io_spec_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd );

template<>
inline bool store_can_write_io_type<binary_truth_table, io_pla_tag_t>( command& cmd ) { return true; }

template<>
void store_write_io_type<binary_truth_table, io_pla_tag_t>( const binary_truth_table& spec, const std::string& filename, const command& cmd );

/******************************************************************************
 * rcbdd                                                                      *
 ******************************************************************************/

template<>
struct store_info<rcbdd>
{
  static constexpr const char* key         = "rcbdds";
  static constexpr const char* option      = "rcbdd";
  static constexpr const char* mnemonic    = "r";
  static constexpr const char* name        = "RCBDD";
  static constexpr const char* name_plural = "RCBDDs";
};

template<>
std::string store_entry_to_string<rcbdd>( const rcbdd& bdd );

template<>
struct show_store_entry<rcbdd>
{
  show_store_entry( const command& cmd );

  bool operator()( rcbdd& bdd, const std::string& dotname, const command& cmd );

  command::log_opt_t log() const;
};

template<>
void print_store_entry<rcbdd>( std::ostream& os, const rcbdd& bdd );

template<>
inline bool store_can_convert<circuit, rcbdd>() { return true; }

template<>
rcbdd store_convert<circuit, rcbdd>( const circuit& circ );

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
