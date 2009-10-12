/*********************************************************************
 Author: Aliaksei Tsitovich <aliaksei.tsitovich@usi.ch>
 , Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

 OpenSMT -- Copyright (C) 2009, Roberto Bruttomesso

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

#include "LARow.h"
//
// Find the elements iterator by its key
//
LARow::iterator LARow::find( int key )
{
  // Check if element is in the row
  if( key >= ( int )is_there.size( ) || !is_there[key] )
    return this->end( );

  for( LARow::iterator it = this->begin( ); it != this->end( ); ++it )
  {
    if( key == it->first )
      return it;
  }

  return this->end( );
}

//
// Erase the element by its key
//
void LARow::erase( int key )
{
  LARow::iterator it = find( key );
  if( it != this->end( ) )
  {
    list<pair<int, Real *> >::erase( it );
    is_there[key] = false;
    return;
  }
  else
  {
    return;
  }
  assert( false );
}

//
// Erase the element by its iterator
//
void LARow::erase( LARow::iterator it )
{
  // Check if element is in the row
  const int key = it->first;
  assert( key < ( int )is_there.size( ) );
  assert( is_there[key] );

  is_there[key] = false;
  list<pair<int, Real *> >::erase( it );
}

//
// Delete all elements from the row
//
void LARow::clear( )
{
  list<pair<int, Real *> >::clear( );
  is_there.clear( );
}

//
// Assign value a to an element identified by key
//
void LARow::assign( const int & key, Real * a )
{
  LARow::iterator it = find( key );
  if( it != this->end( ) )
  {
    assert( is_there[key] );
    it->second = a;
  }
  else
  {
    // perform is_there resize if necessary
    if( key >= ( int )is_there.size( ) )
      is_there.resize( key + 1, false );
    assert( !is_there[key] );
    is_there[key] = true;
    this->push_back( make_pair( key, a ) );
  }
}

//
// Assign value a to an element identified by iterator
//
void LARow::assign( LARow::iterator it, Real * a )
{
  const int key = it->first;
  // perform is_there resize if necessary
  if( key >= ( int )is_there.size( ) )
    is_there.resize( key + 1, false );
  assert( !is_there[key] );
  is_there[key] = true;
  it->second = a;
}
