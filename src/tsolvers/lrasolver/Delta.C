/*********************************************************************
 Author: Aliaksei Tsitovich <aliaksei.tsitovich@lu.unisi.ch>
 Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

 OpenSMT -- Copyright (C) 2007, Roberto Bruttomesso

 OpenSMT is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 OpenSMT is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with OpenSMT. If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************/

#include "Delta.h"

//
// Default constructor (true for +inf; false for -inf)
//
Delta::Delta( deltaType p = UPPER )
{
  infinite = ( p != ZERO );
  positive = ( p == UPPER );
  if( !infinite )
  {
    r = new Real( 0 );
    d = new Real( 0 );
  }
  else
  {
    r = NULL;
    d = NULL;
  }
  shared = false;
}

//
// Constructor for Real delta
//
Delta::Delta( const Real &v )
{
  infinite = false;
  positive = false;
  r = new Real( v );
  d = new Real( 0 );
  shared = false;
}

//
// Constructor for Real delta with strict bit
//
Delta::Delta( const Real &v_r, const Real &v_d )
{
  infinite = false;
  positive = false;
  r = new Real( v_r );
  d = new Real( v_d );
  shared = false;
}

//
// Copy constructor
//
Delta::Delta( const Delta &a )
{
  infinite = a.infinite;
  positive = a.positive;
  r = a.r;
  d = a.d;
  if( !infinite )
    shared = true;
  else
    shared = false;
}

// Assign operator.
Delta& Delta::operator=( const Delta &a )
{
  if( this != &a )
  {
    if( !( shared || infinite ) )
    {
      delete ( r );
      delete ( d );
    }
    infinite = a.infinite;
    positive = a.positive;
    r = new Real( a.R( ) );
    d = new Real( a.D( ) );
    shared = false;
  }
  return *this;
}

Delta::~Delta( )
{
  if( !( shared || infinite ) )
  {
    delete ( r );
    delete ( d );
  }
}

//
// prints the Delta
//
void Delta::print( ostream & out ) const
{
  if( isPlusInf( ) )
    out << "+inf";
  else if( isMinusInf( ) )
    out << "-inf";
  else
    out << R( ) << "|" << D( );
}

//
// basic function to use in comparison with Real
//
bool Delta::isLess( const Real &c ) const
{
  if( isPlusInf( ) )
    return false;
  else if( isMinusInf( ) )
    return true;
  else if( R( ) < c )
    return true;
  else if( R( ) == c && D( ) < 0 )
    return true;
  else
    return false;
}

//
// basic function to use in comparison with Real
//
bool Delta::isGreater( const Real &c ) const
{
  if( isMinusInf( ) )
    return false;
  else if( isPlusInf( ) )
    return true;
  else if( R( ) > c )
    return true;
  else if( R( ) == c && D( ) > 0 )
    return true;
  else
    return false;
}

