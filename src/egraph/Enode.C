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

#include "Enode.h"

//
// Constructor for ENIL
//
Enode::Enode( ) 
  : id        ( ENODE_ID_ENIL )
  , etype     ( ENODE_TYPE_LIST )
  , car       ( NULL )
  , cdr       ( NULL )
  , cong_data ( NULL )
  , atom_data ( NULL )
{ }
//
// Constructor for new Symbols/Numbers
//
Enode::Enode( const int    id_ 
            , const char   etype_ 
	    , const char   arity_
	    , const int    type_
	    , const char * name_ ) 
  : id        ( id_ )
  , etype     ( etype_ )
  , car       ( NULL )
  , cdr       ( NULL )
  , atom_data ( NULL )
{ 
  if ( etype_ == ENODE_TYPE_NUMBER )
    symb_data = new SymbData( type_, name_ );
  else
    symb_data = new SymbData( arity_, type_, name_ );
}
//
// Constructor for new Terms/Lists
//
Enode::Enode( const int  id_
            , const char etype_
            , Enode *    car_ 
            , Enode *    cdr_ )
  : id        ( id_ )
  , etype     ( etype_ )
  , car       ( car_ )
  , cdr       ( cdr_ )
  , cong_data ( NULL )
  , atom_data ( NULL )
{ 
  if ( isTAtom( ) )
    atom_data = new AtomData;
}
//
// Constructor for new Definition
//
Enode::Enode( const int	id_
            , Enode *   def_ )
  : id        ( id_ )
  , etype     ( ENODE_TYPE_DEF )
  , car       ( def_ )
  , cong_data ( NULL )
  , atom_data ( NULL )
{ }

Enode::~Enode ( ) 
{ 
  if ( isSymb( ) || isNumb( ) ) 
    delete symb_data;
  else if ( cong_data ) 
    delete cong_data;
  if ( atom_data ) 
    delete atom_data;
}

void Enode::addParent ( Enode * p )
{
  if ( isEnil( ) )
    return;

  assert( p );
  assert( cong_data );
  assert( isTerm( ) || isList( ) );

  setParentSize( getParentSize( ) + 1 );
  // If it has no parents, adds p 
  if ( getParent( ) == NULL )
  {
    setParent( p );

    if ( etype == ENODE_TYPE_LIST )
      p->setSameCdr( p );
    else
      p->setSameCar( p );

    return;
  }
  // Otherwise adds p in the samecar/cdr of the parent of this node
  if ( etype == ENODE_TYPE_LIST )
  {
    // Build or update samecdr circular list
    assert( getParent( )->getSameCdr( ) != NULL );
    p->setSameCdr( getParent( )->getSameCdr( ) );
    getParent( )->setSameCdr( p );
  }
  else
  {
    ( etype == ENODE_TYPE_TERM );
    // Build or update samecar circular list
    assert( getParent( )->getSameCar( ) != NULL );
    p->setSameCar( getParent( )->getSameCar( ) );
    getParent( )->setSameCar( p );
  }
} 

void Enode::removeParent ( Enode * p )
{
  if ( isEnil( ) ) return;

  assert( getParent( ) != NULL );
  assert( getParentSize( ) > 0 );
  setParentSize( getParentSize( ) - 1 );
  // If only one parent, remove it and restore NULL
  if ( getParentSize( ) == 0 )
  {
    assert( getParent( ) == p );
    setParent( NULL );
    return;
  }
  // Otherwise adds remove p from the samecar/cdr list
  if ( etype == ENODE_TYPE_LIST )
  {
    // Build or update samecdr circular list
    assert( getParent( )->getSameCdr( ) == p );
    getParent( )->setSameCdr( p->getSameCdr( ) );
  }
  else
  {
    // Build or update samecar circular list
    assert( getParent( )->getSameCar( ) == p );
    getParent( )->setSameCar( p->getSameCar( ) );
  }
} 

void Enode::print( ostream & os )
{
  Enode * p = NULL;

  switch( etype )
  {
    case ENODE_TYPE_SYMBOL:
      os << getName( );
      break;

    case ENODE_TYPE_NUMBER:
      os << getName( );
      break;

    case ENODE_TYPE_TERM:
      if ( !cdr->isEnil( ) ) 
	os << "(";

      car->print( os );

      p = cdr;
      while ( !p->isEnil( ) )
      {
	os << " ";
	p->car->print( os );
	p = p->cdr;
      }

      if ( !cdr->isEnil( ) ) 
	os << ")";

      break;

    case ENODE_TYPE_LIST:

      if ( isEnil( ) )
      {
	os << "enil";
	break;
      }

      os << "[";
      car->print( os );

      p = cdr;
      while ( !p->isEnil( ) )
      {
	os << ", ";
	p->car->print( os );
	p = p->cdr;
      }

      os << "]";
      break;

    case ENODE_TYPE_DEF:
      os << ":= " << car;
      break;

    case ENODE_TYPE_UNDEF:
      assert( isEnil( ) );
      os << "-";
      break;

    default:
      cerr << "ERROR: Unknown case value" << endl;
      exit( 1 );
  }
}

void Enode::printSig( ostream & os )
{
  Pair( int ) sig = getSig( );
  os << "(" << sig.first << ", " << sig.second << ")";
}
