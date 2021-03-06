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
 * @file quit.hpp
 *
 * @brief Quits the program
 *
 * @author Mathias Soeken
 * @since  2.3
 */

#ifndef CLI_QUIT_COMMAND_HPP
#define CLI_QUIT_COMMAND_HPP

#include <sys/utsname.h>

#include <thread>

#include <lscli/command.hpp>

namespace cirkit
{

class quit_command : public command
{
public:
  quit_command( const environment::ptr& env )
    : command( env, "Quits the program" ) {}

protected:
  bool execute()
  {
    env->quit = true;
    return true;
  }

public:
  log_opt_t log() const
  {
    utsname u;
    uname( &u );
    return log_opt_t({
        {"sysname", std::string( u.sysname )},
        {"nodename", std::string( u.nodename )},
        {"release", std::string( u.release )},
        {"version", std::string( u.version )},
        {"machine", std::string( u.machine )},
        {"supported_threads", static_cast<int>( std::thread::hardware_concurrency() )}
      });
  }
};

}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
