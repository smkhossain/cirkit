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

#include "expression_parser.hpp"

#include <cassert>
#include <sstream>
#include <stack>

namespace cirkit
{

/******************************************************************************
 * Types                                                                      *
 ******************************************************************************/

/******************************************************************************
 * Private functions                                                          *
 ******************************************************************************/

void push_expression( std::stack<expression_t::ptr>& stack, const expression_t::ptr& expr )
{
  if ( !stack.empty() && stack.top()->type == expression_t::_inv && stack.top()->children.empty() )
  {
    auto top = stack.top();
    stack.pop();
    top->children.push_back( expr );
    push_expression( stack, top );
  }
  else
  {
    stack.push( expr );
  }
}

void prepare_operation( std::stack<expression_t::ptr>& stack, expression_t::type_t type )
{
  auto expr = std::make_shared<expression_t>();
  expr->type = type;
  expr->value = 0u;
  stack.push( expr );
}

void push_operation( std::stack<expression_t::ptr>& stack, expression_t::type_t type, unsigned num_ops )
{
  std::deque<expression_t::ptr> children;

  for ( auto i = 0u; i < num_ops; ++i )
  {
    assert( !stack.empty() );
    children.push_front( stack.top() );
    stack.pop();
  }

  /* get placeholder */
  assert( !stack.empty() );
  auto top = stack.top();
  stack.pop();
  assert( top->children.empty() );
  assert( top->type == type );
  top->children = children;

  push_expression( stack, top );
}

std::ostream& print_operation( std::ostream& os, const expression_t::ptr& expr, char open, char closed )
{
  os << open;
  for ( const auto& c : expr->children )
  {
    os << c;
  }
  os << closed;
  return os;
}

/******************************************************************************
 * Public functions                                                           *
 ******************************************************************************/

expression_t::ptr parse_expression( const std::string& expression )
{
  std::stack<expression_t::ptr> stack;

  for ( auto c : expression )
  {
    switch ( c )
    {
    case '!':
      {
        /* place an empty inverter on the stack */
        auto expr = std::make_shared<expression_t>();
        expr->type = expression_t::_inv;
        expr->value = 0u;
        stack.push( expr );
      } break;

    case '0':
    case '1':
      {
        auto expr = std::make_shared<expression_t>();
        expr->type = expression_t::_const;
        expr->value = ( c == '0' ) ? 0u : 1u;
        push_expression( stack, expr );
      } break;

    case '(':
      prepare_operation( stack, expression_t::_and );
      break;

    case '{':
      prepare_operation( stack, expression_t::_or );
      break;

    case '<':
      prepare_operation( stack, expression_t::_maj );
      break;

    case '[':
      prepare_operation( stack, expression_t::_xor );
      break;

    case ')':
      push_operation( stack, expression_t::_and, 2u );
      break;

    case '}':
      push_operation( stack, expression_t::_or, 2u );
      break;

    case '>':
      push_operation( stack, expression_t::_maj, 3u );
      break;

    case ']':
      push_operation( stack, expression_t::_xor, 2u );
      break;

    default:
      if ( c >= 'a' && c <= 'z' )
      {
        auto expr = std::make_shared<expression_t>();
        expr->type = expression_t::_var;
        expr->value = static_cast<unsigned>( c - 'a' );
        push_expression( stack, expr );
      }
      else
      {
        std::cout << "cannot parse " << c << std::endl;
        assert( false );
      }
      break;
    };
  }

  const auto result = stack.top();
  stack.pop();
  assert( stack.empty() );
  return result;
}

std::ostream& operator<<( std::ostream& os, const expression_t::ptr& expr )
{
  switch ( expr->type )
  {
  case expression_t::_const:
    os << expr->value;
    break;

  case expression_t::_var:
    os << static_cast<char>( 'a' + expr->value );
    break;

  case expression_t::_inv:
    os << '!' << expr->children.front();
    break;

  case expression_t::_and:
    print_operation( os, expr, '(', ')' );
    break;

  case expression_t::_or:
    print_operation( os, expr, '{', '}' );
    break;

  case expression_t::_maj:
    print_operation( os, expr, '<', '>' );
    break;

  case expression_t::_xor:
    print_operation( os, expr, '[', ']' );
    break;

  default:
    assert( false );
  }

  return os;
}

std::string expression_to_string( const expression_t::ptr& expr )
{
  std::stringstream s;
  s << expr;
  return s.str();
}

}

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End: