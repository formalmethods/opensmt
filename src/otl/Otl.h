/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

OpenSMT -- Copyright (C) 2008, Roberto Bruttomesso

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

//
// OpenSMT Template Library
//
#ifndef OTL_H
#define OTL_H

#include "global.h"

#define Pair( T ) pair< T, T >

// TODO: move to config
#define USE_HASH

namespace __gnu_cxx
{
  // Hash function for pairs of integer
  template<>
  class hash< Pair( int ) >
  {
  public:
    size_t operator( )( const Pair( int ) & p ) const
    {
      return p.first ^ p.second;
    }
  };
}

struct strEq { inline bool operator( )( const char * s1, const char * s2 ) const { assert( s1 && s2 ); return strcmp( s1, s2 ) == 0; } };

class Enode;
class EnodeSymbol;

#ifdef USE_HASH
#define Set( T )              hash_set< T >
#define Map( T1, T2 )         hash_map< T1, T2 > 
#else
#define Set( T )              set< T >
#define Map( T1, T2 )         map< T1, T2 > 
#endif

#define MapNameInt            map< string, int >
#define MapNameEnode          map< string, Enode * >
#define MapPairEnode          Map( Pair( int ), Enode * )

#endif
