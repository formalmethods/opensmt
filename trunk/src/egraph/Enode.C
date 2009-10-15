/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@gmail.com>

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

#include "Enode.h"

//
// Constructor for ENIL
//
Enode::Enode( )
  : id        ( ENODE_ID_ENIL )
  , properties( 0 )
  , car       ( NULL )
  , cdr       ( NULL )
  , cong_data ( NULL )
  , atom_data ( NULL )
{ 
  setEtype( ETYPE_LIST );
}
//
// Constructor for new Symbols
//
Enode::Enode( const enodeid_t      id_
	    , const char *         name_ 
	    , const etype_t        etype_
	    , const unsigned       dtype_
	    , vector< unsigned > & arg_sorts_
	    )
  : id         ( id_ )
  , properties ( 0 )
  , car        ( NULL )
  , cdr        ( NULL )
  , atom_data  ( NULL )
{
  setEtype( etype_ );
  setDtype( dtype_ );
  setArity( arg_sorts_.size( ) );
  symb_data = new SymbData( name_, etype_, dtype_, arg_sorts_ );
}
//
// Constructor for new Numbers
//
Enode::Enode( const enodeid_t id_
	    , const char *    name_ 
	    , const etype_t   etype_ 
	    , const unsigned  dtype_ )
  : id         ( id_ )
  , properties ( 0 )
  , car        ( NULL )
  , cdr        ( NULL )
  , atom_data  ( NULL )
{
  setEtype( etype_ );
  setDtype( dtype_ );
  setArity( 0 );

  vector< unsigned > tmp;
  symb_data = new SymbData( name_, etype_, dtype_, tmp );
}
//
// Constructor for new Terms/Lists
//
Enode::Enode( const enodeid_t id_
            , Enode *         car_
            , Enode *         cdr_ )
  : id         ( id_ )
  , properties ( 0 )
  , car        ( car_ )
  , cdr        ( cdr_ )
  , cong_data  ( NULL )
  , atom_data  ( NULL )
{
  assert( car );
  assert( cdr );
  assert( car->isTerm( ) || car->isSymb( ) || car->isNumb( ) );
  assert( cdr->isList( ) );
  //
  // If car is term, then this node is a list
  //
  if ( car->isTerm( ) )
  {
    setEtype( ETYPE_LIST );
    if ( cdr->getArity( ) == MAX_ARITY - 1 )
      setArity( cdr->getArity( ) );
    else
      setArity( cdr->getArity( ) + 1 );
  }
  //
  // Otherwise this node is a term
  //
  else
  {
    //
    // Set Etype
    //
    setEtype( ETYPE_TERM );
    //
    // Set Arity
    //
    setArity( cdr->getArity( ) );
    //
    // Set Dtype
    //
    //
    // For arithmetic and ites we don't know whether 
    // they contain Real, Int, BitVec, ... . We 
    // determine the type now
    //
    if ( isPlus  ( ) 
      || isMinus ( ) 
      || isTimes ( ) 
      || isUminus( )
      || isDiv   ( ) )
    {
      setDtype( get1st( )->getDType( ) );
    }
    else if ( isIte( ) )
    {
      setDtype( get2nd( )->getDType( ) );
    }
    else
    {
      setDtype( car->getDType( ) );
    }
    //
    // Set width for bitvectors
    //
    if ( isDTypeBitVec( ) )
    {
      // Compute width
      if ( isConcat( ) )
      {
	uint32_t width = 0;
	Enode * arg_list;
	for ( arg_list = cdr
	    ; !arg_list->isEnil( ) 
	    ; arg_list = arg_list->getCdr( ) )
	{
	  Enode * arg = arg_list->getCar( );
	  width += arg->getWidth( );
	}
	setWidth( width );
      }
      else if ( isIte( ) )
	setWidth( get2nd( )->getWidth( ) );
      else if ( isExtract( ) )
	setWidth( car->getWidth( ) );
      else if ( isSignExtend( ) )
	; // Do nothing, we set width in mkSignExtend
      else if ( isWord1cast( ) )
	setWidth( 1 );
      else if ( getArity( ) > 0 )
	setWidth( get1st( )->getWidth( ) );
      else
	setWidth( car->getWidth( ) );

      assert( isSignExtend( ) || getWidth( ) > 0 );
    }
  }

  if ( isTAtom( ) )
    atom_data = new AtomData;

  assert( isTerm( ) || isList( ) );
}
//
// Constructor for new Definition
//
Enode::Enode( const enodeid_t	id_
            , Enode *		def_ )
  : id         ( id_ )
  , properties ( ETYPE_DEF )
  , car        ( def_ )
  , cong_data  ( NULL )
  , atom_data  ( NULL )
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

    if ( isList( ) )
      p->setSameCdr( p );
    else
      p->setSameCar( p );

    return;
  }
  // Otherwise adds p in the samecar/cdr of the parent of this node
  if ( isList( ) )
  {
    // Build or update samecdr circular list
    assert( getParent( )->getSameCdr( ) != NULL );
    p->setSameCdr( getParent( )->getSameCdr( ) );
    getParent( )->setSameCdr( p );
  }
  else
  {
    // Build or update samecar circular list
    assert( getParent( )->getSameCar( ) != NULL );
    p->setSameCar( getParent( )->getSameCar( ) );
    getParent( )->setSameCar( p );
  }
}

void Enode::removeParent ( Enode * p )
{
  if ( isEnil( ) ) return;

  assert( isList( ) || isTerm( ) );
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
  if ( isList( ) )
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

  if( isSymb( ) )
    os << getName( );
  else if ( isNumb( ) )
  {
    if ( isDTypeBitVec( ) )
    {
    // Choose binary or decimal value; binary won't work with most solvers
#if 0
      os << "bvbin" << getName();
#else
      os << "bv" << getValue( ) << "[" << strlen(getName()) << "]";
#endif
    }
    else
    {
      Real r = getValue();
#if USE_GMP
      if (r < 0)
      {
	if (r.get_den() != 1)
	  os << "(/ " << "(~ " << abs(r.get_num()) <<")" << " " << r.get_den() << ")";
	else
	  os << "(~ " << abs(r) <<")";
      }
      else
      {
	if (r.get_den() != 1)
	  os << "(/ " << r.get_num() << " " << r.get_den() << ")";
	else
	  os << r;
      }
#else
      os << r;
#endif
    }
  }
  else if ( isTerm( ) )
  {
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
  }
  else if ( isList( ) )
  {
    if ( isEnil( ) )
      os << "-";
    else
    {
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
    }
  }
  else if ( isDef( ) )
  {
    os << ":= " << car;
  }
  else if ( isEnil( ) )
  {
    os << "-";
  }
  else
    error( "unknown case value", "" );
}

void Enode::printSig( ostream & os )
{
#ifdef BUILD_64
  enodeid_pair_t sig = getSig( );
  os << "(" << (sig>>sizeof(enodeid_t)*8) << ", " << (sig|0x00000000FFFFFFFF) << ")";
#else
  Pair( enodeid_t ) sig = getSig( );
  os << "(" << sig.first << ", " << sig.second << ")";
#endif
}
