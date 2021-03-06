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
 * @file alias.hpp
 *
 * @brief Create command aliases
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_ALIAS_COMMAND_HPP
#define CLI_ALIAS_COMMAND_HPP

#include <boost/program_options.hpp>

#include <lscli/command.hpp>

using namespace boost::program_options;

namespace cirkit
{

class alias_command : public command
{
public:
  alias_command( const environment::ptr& env )
    : command( env, "Create command aliases" )
  {
    add_positional_option( "alias" );
    add_positional_option( "expansion" );
    opts.add_options()
      ( "alias",     value( &alias ),     "regular expression for the alias" )
      ( "expansion", value( &expansion ), "expansion for the alias" )
      ;
  }

protected:
  rules_t validity_rules() const
  {
    return {
      { [this]() { return is_set( "alias" ) && is_set( "expansion" ); }, "both alias and expansion need to be set" }
    };
  }

  bool execute()
  {
    env->aliases[alias] = expansion;
    return true;
  }

private:
  std::string alias;
  std::string expansion;
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
