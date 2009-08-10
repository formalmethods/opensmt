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

#include "Egraph.h"

void Egraph::initializeStore( )
{
  //
  // Reserve room for at least 65536 nodes
  //
  id_to_enode .reserve( 65536 );
  //
  // Allocates SMTLIB predefined symbols
  //
  newSymbol( "true" , DTYPE_BOOL ); assert( ENODE_ID_TRUE  == id_to_enode.size( ) - 1 );
  newSymbol( "false", DTYPE_BOOL ); assert( ENODE_ID_FALSE == id_to_enode.size( ) - 1 );
  //
  // Arithmetic predefined operators and predicates
  //
  newSymbol( "+" , DTYPE_REAL ); assert( ENODE_ID_PLUS   == id_to_enode.size( ) - 1 );
  newSymbol( "-" , DTYPE_REAL ); assert( ENODE_ID_MINUS  == id_to_enode.size( ) - 1 );
  newSymbol( "~" , DTYPE_REAL ); assert( ENODE_ID_UMINUS == id_to_enode.size( ) - 1 );
  newSymbol( "*" , DTYPE_REAL ); assert( ENODE_ID_TIMES  == id_to_enode.size( ) - 1 );
  newSymbol( "/" , DTYPE_REAL ); assert( ENODE_ID_DIV    == id_to_enode.size( ) - 1 );
  newSymbol( "=" , DTYPE_BOOL ); assert( ENODE_ID_EQ     == id_to_enode.size( ) - 1 );
  newSymbol( "!=", DTYPE_BOOL ); assert( ENODE_ID_NEQ    == id_to_enode.size( ) - 1 );
  newSymbol( "<=", DTYPE_BOOL ); assert( ENODE_ID_LEQ    == id_to_enode.size( ) - 1 );
  newSymbol( ">=", DTYPE_BOOL ); assert( ENODE_ID_GEQ    == id_to_enode.size( ) - 1 );
  newSymbol( "<" , DTYPE_BOOL ); assert( ENODE_ID_LT     == id_to_enode.size( ) - 1 );
  newSymbol( ">" , DTYPE_BOOL ); assert( ENODE_ID_GT     == id_to_enode.size( ) - 1 );
  //
  // Bit-vector predefined operators and predicates
  //
  newSymbol( "bvslt"      , DTYPE_BOOL   ); assert( ENODE_ID_BVSLT       == id_to_enode.size( ) - 1 );
  newSymbol( "bvsgt"      , DTYPE_BOOL   ); assert( ENODE_ID_BVSGT       == id_to_enode.size( ) - 1 );
  newSymbol( "bvsle"      , DTYPE_BOOL   ); assert( ENODE_ID_BVSLE       == id_to_enode.size( ) - 1 );
  newSymbol( "bvsge"      , DTYPE_BOOL   ); assert( ENODE_ID_BVSGE       == id_to_enode.size( ) - 1 );
  newSymbol( "bvult"      , DTYPE_BOOL   ); assert( ENODE_ID_BVULT       == id_to_enode.size( ) - 1 );
  newSymbol( "bvugt"      , DTYPE_BOOL   ); assert( ENODE_ID_BVUGT       == id_to_enode.size( ) - 1 );
  newSymbol( "bvule"      , DTYPE_BOOL   ); assert( ENODE_ID_BVULE       == id_to_enode.size( ) - 1 );
  newSymbol( "bvuge"      , DTYPE_BOOL   ); assert( ENODE_ID_BVUGE       == id_to_enode.size( ) - 1 );
  newSymbol( "concat"     , DTYPE_BITVEC ); assert( ENODE_ID_CONCAT      == id_to_enode.size( ) - 1 );
  newSymbol( "distinct"   , DTYPE_BOOL   ); assert( ENODE_ID_DISTINCT    == id_to_enode.size( ) - 1 );
  newSymbol( "bvand"      , DTYPE_BITVEC ); assert( ENODE_ID_BVAND       == id_to_enode.size( ) - 1 );
  newSymbol( "bvor"       , DTYPE_BITVEC ); assert( ENODE_ID_BVOR        == id_to_enode.size( ) - 1 );
  newSymbol( "bvxor"      , DTYPE_BITVEC ); assert( ENODE_ID_BVXOR       == id_to_enode.size( ) - 1 );
  newSymbol( "bvnot"      , DTYPE_BITVEC ); assert( ENODE_ID_BVNOT       == id_to_enode.size( ) - 1 );
  newSymbol( "bvadd"      , DTYPE_BITVEC ); assert( ENODE_ID_BVADD       == id_to_enode.size( ) - 1 );
  newSymbol( "bvsub"      , DTYPE_BITVEC ); assert( ENODE_ID_BVSUB       == id_to_enode.size( ) - 1 );
  newSymbol( "bvmul"      , DTYPE_BITVEC ); assert( ENODE_ID_BVMUL       == id_to_enode.size( ) - 1 );
  newSymbol( "bvneg"      , DTYPE_BITVEC ); assert( ENODE_ID_BVNEG       == id_to_enode.size( ) - 1 );
  newSymbol( "bvlshr"     , DTYPE_BITVEC ); assert( ENODE_ID_BVLSHR      == id_to_enode.size( ) - 1 );
  newSymbol( "bvashr"     , DTYPE_BITVEC ); assert( ENODE_ID_BVASHR      == id_to_enode.size( ) - 1 );
  newSymbol( "bvshl"      , DTYPE_BITVEC ); assert( ENODE_ID_BVSHL       == id_to_enode.size( ) - 1 );
  newSymbol( "bvsrem"     , DTYPE_BITVEC ); assert( ENODE_ID_BVSREM      == id_to_enode.size( ) - 1 );
  newSymbol( "bvurem"     , DTYPE_BITVEC ); assert( ENODE_ID_BVUREM      == id_to_enode.size( ) - 1 );
  newSymbol( "bvsdiv"     , DTYPE_BITVEC ); assert( ENODE_ID_BVSDIV      == id_to_enode.size( ) - 1 );
  newSymbol( "bvudiv"     , DTYPE_BITVEC ); assert( ENODE_ID_BVUDIV      == id_to_enode.size( ) - 1 );
  newSymbol( "zero_extend", DTYPE_BITVEC ); assert( ENODE_ID_ZERO_EXTEND == id_to_enode.size( ) - 1 );
  //
  // Logical predefined predicates
  //
  newSymbol( "implies"     , DTYPE_BOOL  ); assert( ENODE_ID_IMPLIES    == id_to_enode.size( ) - 1 );
  newSymbol( "and"         , DTYPE_BOOL  ); assert( ENODE_ID_AND        == id_to_enode.size( ) - 1 );
  newSymbol( "or"          , DTYPE_BOOL  ); assert( ENODE_ID_OR         == id_to_enode.size( ) - 1 );
  newSymbol( "not"         , DTYPE_BOOL  ); assert( ENODE_ID_NOT        == id_to_enode.size( ) - 1 );
  newSymbol( "iff"         , DTYPE_BOOL  ); assert( ENODE_ID_IFF        == id_to_enode.size( ) - 1 );
  newSymbol( "xor"         , DTYPE_BOOL  ); assert( ENODE_ID_XOR        == id_to_enode.size( ) - 1 );
  newSymbol( "ite"         , DTYPE_UNDEF ); assert( ENODE_ID_ITE        == id_to_enode.size( ) - 1 );
  newSymbol( "if_then_else", DTYPE_BOOL  ); assert( ENODE_ID_IFTHENELSE == id_to_enode.size( ) - 1 );
  //
  // For cbe computation
  //
  newSymbol( "cbe"         , DTYPE_BOOL  ); assert( ENODE_ID_CBE == id_to_enode.size( ) - 1 );
  //
  // BitVec typecasts
  //
  newSymbol( "word1Cast"   , DTYPE_BITVEC ); assert( ENODE_ID_WORD1CAST == id_to_enode.size( ) - 1 );
  newSymbol( "boolCast"    , DTYPE_BOOL   ); assert( ENODE_ID_BOOLCAST  == id_to_enode.size( ) - 1 );
  //
  // Set top node to empty
  //
  top = NULL;
  //
  // Allocate true and false
  //
  etrue  = allocTrue ( );
  efalse = allocFalse( );
  //
  // Does not have ites (yet)
  //
  has_ites = false;
  //
  // Inserts true and false in signature table
  //
  insertSigTab( etrue );
  insertSigTab( efalse );
}

//
// Adds a new sort
//
void Egraph::newSort( const char * n )
{
  MapNameUint::iterator it = name_to_extrasort.find( n );

  if ( it != name_to_extrasort.end( ) )
    error( "redeclaring sort ", n );

  unsigned sort_id = DTYPE_U + 1 + name_to_extrasort.size( );
  name_to_extrasort.insert( it, make_pair( string( n ), sort_id ) );
  assert( extrasort_to_name.find( sort_id ) == extrasort_to_name.end( ) );
  extrasort_to_name[ sort_id ] = n;
}

//
// Retrieves a sort
//
unsigned Egraph::getSort( const char * n )
{
  MapNameUint::iterator it = name_to_extrasort.find( n );

  if ( it == name_to_extrasort.end( ) )
    error( "undeclared sort symbol", n );

  return it->second;
}

//
// Allocates true
//
Enode * Egraph::allocTrue ( )
{
  Enode * res = cons( id_to_enode[ ENODE_ID_TRUE ] );
  assert( res );
  assert( res->isTerm( ) );
  res->allocCongData( );
  res->setConstant( res );
  return res;
}

//
// Allocates false
//
Enode * Egraph::allocFalse ( )
{
  Enode * res = cons( id_to_enode[ ENODE_ID_FALSE ] );
  assert( res );
  assert( res->isTerm( ) );
  res->allocCongData( );
  res->setConstant( res );
  return res;
}

//
// Inserts a number
//
Enode * Egraph::insertNumber( Enode * n )
{
  assert( n->isNumb( ) );
  pair< MapNameEnode::iterator, bool > res = name_to_number.insert( make_pair( n->getName( ), n ) );
  // Number has been inserted
  if ( res.second )
  {
    if ( enable_undo )
    {
      undo_stack_term.push_back( n );
      undo_stack_oper.push_back( NUMB );
    }
    id_to_enode .push_back( n );
    assert( n->getId( ) == (enodeid_t)id_to_enode.size( ) - 1 );
    return n;
  }
  // Number was already there, get rid of n
  delete n;
  // Return the old one
  return res.first->second;
}

//
// Inserts a symbol
//
void Egraph::insertSymbol( Enode * s )
{
  assert( s );
  assert( s->isSymb( ) );
  // Consistency for id
  assert( (enodeid_t)id_to_enode.size( ) == s->getId( ) );
  // Symbol is not there
  assert( name_to_symbol.find( s->getName( ) ) == name_to_symbol.end( ) );
  // Insert Symbol
  name_to_symbol[ s->getName( ) ] = s;
  id_to_enode .push_back( s );
  // Save for undo
  if ( enable_undo )
  {
    undo_stack_oper.push_back( SYMB );
    undo_stack_term.push_back( s );
  }
}

//
// Removes a symbol
//
void Egraph::removeSymbol( Enode * s )
{
  assert( s->isSymb( ) );
  assert( enable_undo );
  MapNameEnode::iterator it = name_to_symbol.find( s->getName( ) );
  assert( it != name_to_symbol.end( ) );
  assert( it->second == s );
  name_to_symbol.erase( it );
  // Special removal for extraction
  int lsb, msb;
  s->getExtract( &msb, &lsb );
  if ( lsb != -1 )
  {
    const Pair( int ) sig = make_pair( msb, lsb );
    assert( ext_store.find( sig ) != ext_store.end( ) );
    ext_store.erase( sig );
    assert( ext_store.find( sig ) == ext_store.end( ) );
  }
  int i;
  // Special removal for sign_extend
  if ( s->isSignExtend( &i ) )
  {
    assert( se_store[ i ] == s );
    se_store[ i ] = NULL;
  }
  // Only the last added symbol can be removed
  assert( s->getId( ) == (enodeid_t)id_to_enode.size( ) - 1 );
  id_to_enode.pop_back( );
  delete s;
}

//
// Inserts a number
//
void Egraph::removeNumber( Enode * n )
{
  assert( n->isNumb( ) );
  assert( enable_undo );
  MapNameEnode::iterator it = name_to_number.find( n->getName( ) );
  assert( it != name_to_number.end( ) );
  assert( it->second == n );
  name_to_number.erase( it );
  assert( n->getId( ) == (enodeid_t)id_to_enode.size( ) - 1 );
  id_to_enode.pop_back( );
  delete n;
}

//
// Retrieves a symbol from the name
//
Enode * Egraph::lookupSymbol( const char * name )
{
  assert( name );
  MapNameEnode::iterator it = name_to_symbol.find( name );
  if ( it == name_to_symbol.end( ) ) return NULL;
  return it->second;
}

//
// Retrieves a define
//
Enode * Egraph::lookupDefine( const char * name )
{
  assert( name );
  MapNameEnode::iterator it = name_to_define.find( name );
  if ( it == name_to_define.end( ) ) return NULL;
  return it->second;
}

//
// Store a define
//
void Egraph::insertDefine( const char * n, Enode * d )
{
  assert( !enable_undo );
  assert( d );
  assert( n );
  assert( d->isDef( ) );
  assert( (enodeid_t)id_to_enode.size( ) == d->getId( ) );
  assert( name_to_define.find( n ) == name_to_define.end( ) );
  name_to_define[ n ] = d;
  id_to_enode.push_back( d );
}

//
// Insert into signature table possibly creating a new node
//
Enode * Egraph::insertSigTab ( const enodeid_t id, Enode * car, Enode * cdr )
{
  assert( enable_undo );
  assert( car == car->getRoot( ) );
  assert( cdr == cdr->getRoot( ) );

#ifdef BUILD_64
  enodeid_pair_t sig = encode( car->getCid( ), cdr->getCid( ) );
  Enode * res = sig_tab.lookup( sig );
#else
  const Pair( enodeid_t ) sig = make_pair( car->getCid( ), cdr->getCid( ) );
  Enode * res = sig_tab.lookup( sig );
#endif

  if ( res == NULL )
  {
    Enode * e = new Enode( id, car, cdr );
    sig_tab.insert( e );
    return e;
  }
  return res;
}

//
// Insert into Store
//
Enode * Egraph::insertStore( const enodeid_t id, Enode * car, Enode * cdr )
{
  assert( !enable_undo );
  assert( car == car->getRoot( ) );
  assert( cdr == cdr->getRoot( ) );

  Enode * e = new Enode( id, car, cdr );
  Enode * x = store.insert( e );
  // Insertion done
  if ( x == e ) return e;
  // Node already there
  delete e;
  return x;
}

//
// Retrieve element from signature table
//
Enode * Egraph::lookupSigTab ( Enode * e )
{
  // MapPair & sig_tab = e->isList( ) ? sig_tab_list : sig_tab_term ;
#ifdef BUILD_64
  Enode * res = sig_tab.lookup( e->getSig( ) );
#else
  Enode * res = sig_tab.lookup( e->getSig( ) );
#endif
  assert( res );
  return res;
}

//
// Adds element to signature table
//
Enode * Egraph::insertSigTab ( Enode * e )
{
  sig_tab.insert( e );
  return e;
}

//
// Remove element from signature table
//
void Egraph::removeSigTab ( Enode * e )
{
  sig_tab.erase( e );
}

//
// Copy list into a new one whose elements are retrieved from the cache
//

Enode * Egraph::copyEnodeEtypeListWithCache( Enode * l, bool map2 )
{
  assert( active_dup_map );

  list< Enode * > new_args;
  for ( Enode * arg = l ; !arg->isEnil( ) ; arg = arg->getCdr( ) )
  {
    new_args.push_front( map2
		       ? valDupMap2( arg->getCar( ) )
		       : valDupMap ( arg->getCar( ) )
		       );
  }

  return cons( new_args );
}

//
// Create a new term of the same kind but using info in the cache and
// also performs some simplifications
//
Enode * Egraph::copyEnodeEtypeTermWithCache( Enode * term, bool map2 )
{
  assert(  map2 || active_dup_map );
  assert( !map2 || active_dup_map2 );
  Enode * ll = copyEnodeEtypeListWithCache( term->getCdr( ), map2 );
  assert( ll->isList( ) );
  //
  // Case
  //
  if ( term->isAnd        ( ) ) return mkAnd       ( ll );
  if ( term->isOr         ( ) ) return mkOr        ( ll );
  if ( term->isNot        ( ) ) return mkNot       ( ll );
  if ( term->isImplies    ( ) ) return mkImplies   ( ll );
  if ( term->isIff        ( ) ) return mkIff       ( ll );
  if ( term->isXor        ( ) ) return mkXor       ( ll );
  if ( term->isEq         ( ) ) return mkEq        ( ll );
  if ( term->isNeq        ( ) ) return mkNeq       ( ll );
  if ( term->isLeq        ( ) ) return mkLeq       ( ll );
  if ( term->isBvule      ( ) ) return mkBvule     ( ll );
  if ( term->isBvsle      ( ) ) return mkBvsle     ( ll );
  if ( term->isDistinct   ( ) ) return mkDistinct  ( ll );
  if ( term->isConcat     ( ) ) return mkConcat    ( ll );
  if ( term->isBvadd      ( ) ) return mkBvadd     ( ll );
  if ( term->isBvsub      ( ) ) return mkBvsub     ( ll );
  if ( term->isBvmul      ( ) ) return mkBvmul     ( ll );
  if ( term->isBvand      ( ) ) return mkBvand     ( ll );
  if ( term->isBvnot      ( ) ) return mkBvnot     ( ll );
  if ( term->isBoolcast   ( ) ) return mkBoolcast  ( ll->getCar( ) );
  if ( term->isWord1cast  ( ) ) return mkWord1cast ( ll->getCar( ) );
  if ( term->isSignExtend ( ) ) return mkSignExtend( term->getWidth( ) - ll->getCar( )->getWidth( ), ll->getCar( ) );

  int lsb, msb;
  if ( term->isExtract( &msb, &lsb ) )
    return mkExtract( msb, lsb, ll->getCar( ) );

  if ( ll->getArity( ) == 3 )
  {
    Enode * i = ll->getCar( );
    Enode * t = ll->getCdr( )->getCar( );
    Enode * e = ll->getCdr( )->getCdr( )->getCar( );

    if ( term->isIte        ( ) ) return mkIte        ( i, t, e );
    if ( term->isIfthenelse ( ) ) return mkIfthenelse ( i, t, e );
  }

  if ( term->isVar( ) || term->isConstant( ) )
    return term;

  //
  // Enable if you want to make sure that your case is handled
  //
  //error( "Please add a case for ", term->getCar( ) );

  Enode * new_term = cons( term->getCar( ), ll );
  return new_term;
}

//
// FIXME: This is a little bit counter intuitive
// The list given is now in reverse order w.r.t.
// the returned one, they should insted be the
// same, more logical. But that implies that we
// have to modify other parts of the code, so
// be careful
//
Enode * Egraph::cons( list< Enode * > & args )
{
  Enode * elist = const_cast< Enode * >( enil );

  for ( list< Enode * >::iterator it = args.begin( )
      ; it != args.end( )
      ; it ++ )
    elist = cons( *it, elist );

  return elist;
}

//
// Creates a new term or list
//
Enode * Egraph::cons( Enode * car, Enode * cdr )
{
  assert( car );
  assert( cdr );
  assert( car->isTerm( ) || car->isSymb( ) || car->isNumb( ) );
  assert( cdr->isList( ) );
  Enode * e = NULL;

  if ( enable_undo )
  {
    // Move car and cdr to root
    car = car->getRoot( );
    cdr = cdr->getRoot( );
    // Create and insert a new enode if necessary
    e = insertSigTab( id_to_enode.size( ), car, cdr );
    // The node was there already. Return it
    if ( (enodeid_t)id_to_enode.size( ) != e->getId( ) ) return e;
    // We keep the created enode
    id_to_enode .push_back( e );
    // Initialize its congruence data structures
    initializeCong( e );
    // Save Backtrack information
    undo_stack_term.push_back( e );
    undo_stack_oper.push_back( CONS );
    assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  }
  else
  {
    assert( car == car->getRoot( ) );
    assert( cdr == cdr->getRoot( ) );
    // Create and insert a new enode if necessary
    e = insertStore( id_to_enode.size( ), car, cdr );
    // The node was there already. Return it
    if ( (enodeid_t)id_to_enode.size( ) != e->getId( ) ) return e;
    // We keep the created enode
    id_to_enode .push_back( e );
  }

  assert( e );
  assert( e->getId( ) > 0 );
  return e;
}

void Egraph::undoCons( Enode * e )
{
  assert( enable_undo );
  assert( e );
  assert( e->isTerm( ) || e->isList( ) );
  Enode * car = e->getCar( );
  Enode * cdr = e->getCdr( );
  assert( car );
  assert( cdr );
  // Node must be there
  assert( lookupSigTab( e ) == e );
  // Remove from sig_tab
  removeSigTab( e );
  // Remove Parent info
  if ( e->isList( ) )
    car->removeParent( e );
  cdr->removeParent( e );
  // Remove initialization
  initialized.erase( e->getId( ) );
  // Get rid of the correspondence
  id_to_enode.pop_back( );
  // Erase the enode
  delete e;
}

//
// Create a variable
//
Enode * Egraph::mkVar( const char * name )
{
  Enode * e = lookupSymbol( name );
  if ( e == NULL ) error( "undeclared function symbol ", name );
  return cons( e );
}

Enode * Egraph::mkNum( const char * value )
{
  Enode * new_enode = new Enode( id_to_enode.size( )
			       , value
			       , ETYPE_NUMB
                               , DTYPE_REAL );

  assert( new_enode );
  Enode * res = insertNumber( new_enode );
  return cons( res );
}

Enode * Egraph::mkNum( const Real & real_value )
{
#if FAST_RATIONALS
  return mkNum( const_cast< char * >(real_value.get_str( ).c_str( )) );
#elif USE_GMP
  return mkNum( const_cast< char * >(real_value.get_str( ).c_str( )) );
#else
  char buf[ 128 ];
  sprintf( buf, "%lf", real_value );
  return mkNum( buf );
#endif
}

Enode * Egraph::mkNum( const char * num, const char * den )
{
  string s = (string)num + "/" + (string)den;


#if FAST_RATIONALS
  Real real_value( s.c_str() );
  return mkNum( const_cast< char * >(real_value.get_str( ).c_str( )) );
#else
  Real num_d = atof( num );
  Real den_d = atof( den );
  Real value = num_d / den_d;
  return mkNum( value );
#endif
}

Enode * Egraph::mkBvnum( char * str )
{
  Enode * new_enode = NULL;

  if ( str[ 0 ] == 'b'
    && str[ 1 ] == 'v'
    && str[ 2 ] != 'b' )
  {
    char * end_value = strchr( str, '[' );
    char * p = &(str[2]);

    int width = 0;
    sscanf( end_value, "[%d]", &width );
    assert( width > 0 );
    //
    // Copy relevant part of the string
    //
    char dec_value_str[ width ];
    char * q = dec_value_str;
    while ( p != end_value ) *q ++ = *p ++;
    *q = '\0';
    //
    // Allocate a mp number
    //
    mpz_class dec_value( dec_value_str );
    //
    // Compute value with leading zeros
    //

    string value;
    value.insert( 0, width - dec_value.get_str( 2 ).size( ), '0' );
    value = value + dec_value.get_str( 2 );

    assert( (int)strlen( value.c_str( ) ) == width );

    new_enode = new Enode( id_to_enode.size( )
			 , value.c_str( )
			 , ETYPE_NUMB
			 , DTYPE_BITVEC | width );
  }
  else if ( str[ 0 ] == 'b'
         && str[ 1 ] == 'v'
         && str[ 2 ] == 'b'
         && str[ 3 ] == 'i'
	 && str[ 4 ] == 'n' )
  {
    int width = strlen( str ) - 5;

    new_enode = new Enode( id_to_enode.size( )
			 , &(str[ 5 ])
	                 , ETYPE_NUMB
			 , DTYPE_BITVEC | width );
  }
  else
  {
    int width = strlen( str );

    new_enode = new Enode( id_to_enode.size( )
			 , str
			 , ETYPE_NUMB
			 , DTYPE_BITVEC | width );
  }

  assert( new_enode );
  Enode * res = insertNumber( new_enode );

  return cons( res );
}

Enode * Egraph::mkExtract( int msb, int lsb, Enode * arg )
{
  assert( arg );
  assert( 0 <= lsb );
  assert( lsb <= msb );
  assert( msb <= arg->getWidth( ) - 1 );

  Enode * res = NULL;

  const int i = msb, j = lsb;
  int arg_msb, arg_lsb;
  //
  // Apply rewrite rules. We assume x to have width n, y to have width m
  //
  // Rule 1:
  // x[n-1:0] --> x
  //
  if ( arg->getWidth( ) == i - j + 1 )
    res = arg;
  //
  // Rewrite rule for extraction
  //
  // x[msb:lsb][i:j] --> x[i+lsb:j+lsb]
  //
  else if ( arg->isExtract( &arg_msb, &arg_lsb ) )
  {
    Enode * arg_arg = arg->getCdr( )->getCar( );
    assert( !arg_arg->isExtract( ) );
    res = mkExtract( i + arg_lsb, j + arg_lsb, arg_arg );
  }
  //
  // Rewrite rules for concatenation
  //
  else if ( arg->isConcat( )
         || arg->isCbe   ( ) )
  {
    list< Enode * > new_args;
    int width_left = arg->getWidth( );

    for ( Enode * list = arg->getCdr( )
	; !list->isEnil( )
	; list = list->getCdr( ) )
    {
      Enode * conc = list->getCar( );
      const int conc_width = conc->getWidth( );
      const int rem_width = width_left - conc_width;
      width_left = rem_width;
      // Compute current extraction indexes
      int real_msb = i - rem_width;
      int real_lsb = j - rem_width;
      // Continue if this slice is out of msb:lsb
      if ( real_msb < 0 || real_lsb >= conc_width )
	continue;
      // Fix indexes if out of bounds
      if ( real_msb >= conc_width ) real_msb = conc_width - 1;
      if ( real_lsb <  0 )          real_lsb = 0;
      // Add slice to list
      new_args.push_front( mkExtract( real_msb, real_lsb, conc ) );
    }
    if ( arg->isConcat( ) )
      res = mkConcat( cons( new_args ) );
    else
      res = mkCbe   ( cons( new_args ) );
  }
  //
  // Rewrite a selected number as the equivalent number
  //
  else if ( arg->isConstant( ) )
  {
    const char * value = arg->getCar( )->getName( );
    int width = arg->getWidth( );

    char new_value[ i - j + 2 ];
    const int bit_i = width - 1 - i;
    const int bit_j = width - 1 - j;

    for ( int h = bit_i ; h <= bit_j ; h ++ )
      new_value[ h - bit_i ] = value[ h ];

    new_value[ i - j + 1 ] = '\0';

    assert( (int)strlen( new_value ) == i - j + 1 );

    res = mkBvnum( new_value );
  }
  else
  {
    const Pair( int ) sig = make_pair( msb, lsb );
    MapPairEnode::iterator it = ext_store.find( sig );
    Enode * e = NULL;
    if ( it == ext_store.end( ) )
    {
      char name[ 256 ];
      sprintf( name, "extract[%d:%d]", msb, lsb );
      assert( lookupSymbol( name ) == NULL );
      e = newSymbol( name, DTYPE_BITVEC | (msb - lsb + 1) );
      e->setExtract( lsb );
      ext_store[ sig ] = e;
    }
    else
    {
      e = it->second;
    }
    assert( e );
    res = cons( e, cons( arg ) );
  }

  assert( res );
  return res;
}

Enode * Egraph::mkConcat( Enode * args )
{
  assert( args );

  Enode * res = NULL;

  if ( args->getArity( ) == 1 )
  {
     res = args->getCar( );
  }
  else
  {
    assert( args->getArity( ) >= 2 );

    Enode * a = args->getCar( );
    Enode * b = args->getCdr( )->getCar( );

    if ( args->getArity( ) == 2
      && a->isConstant ( )
      && b->isConstant ( ) )
    {
      char str[ strlen( a->getCar( )->getName( ) ) + strlen( b->getCar( )->getName( ) ) + 1 ];
      strcpy( str, a->getCar( )->getName( ) );
      strcat( str, b->getCar( )->getName( ) );
      res = mkBvnum( str );
    }
    else
    {
      list< Enode * > new_args;
      for ( Enode * list = args ; !list->isEnil( ) ; list = list->getCdr( ) )
      {
	Enode * e = list->getCar( );
	assert( e->isDTypeBitVec( ) );

	// Add arguments instead
	if ( e->isConcat( ) )
	  for ( Enode * l = e->getCdr( ) ; !l->isEnil( ) ; l = l->getCdr( ) )
	    new_args.push_front( l->getCar( ) );
	else
	  new_args.push_front( e );
      }

      res = cons( id_to_enode[ ENODE_ID_CONCAT ], cons( new_args ) );
    }
  }

  assert( res );
  return res;
}

Enode * Egraph::mkCbe( Enode * args )
{
  assert( args );
  assert( args->getArity( ) >= 1 );

  if ( args->getArity( ) == 1 )
    return args->getCar( );

  return cons( id_to_enode[ ENODE_ID_CBE ], args );
}

Enode * Egraph::mkBvadd ( Enode * args )
{
  assert( args );

  if ( args->getArity( ) == 1 )
    return args->getCar( );

  assert( args->getArity( ) == 2 );
  Enode * a = args->getCar( );
  Enode * b = args->getCdr( )->getCar( );

  assert( a->isDTypeBitVec( ) );
  assert( b->isDTypeBitVec( ) );
  assert( a->getWidth( ) == b->getWidth( ) );

  const int width = a->getWidth( );
  string zero_str;
  zero_str.insert( 0, width, '0' );
  Enode * zero = mkBvnum( const_cast< char * >( zero_str.c_str( ) ) );
  //
  // a + 0 = a, b + 0 = b
  //
  if ( a == zero ) return b;
  if ( b == zero ) return a;

  Enode * res = NULL;
  //
  // Both numbers are constants, simplify
  //
  if ( a->isConstant( ) && b->isConstant( ) )
  {
    mpz_class aval( a->getCar( )->getName( ), 2 );
    mpz_class bval( b->getCar( )->getName( ), 2 );
    aval = aval + bval;
    Enode * res = makeNumberFromGmp( aval, width );
    return res;
  }
  else
    res = cons( id_to_enode[ ENODE_ID_BVADD ], args );

  assert( res );
  return res;
}

Enode * Egraph::mkBvmul ( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );
  Enode * a = args->getCar( );
  Enode * b = args->getCdr( )->getCar( );

  assert( a->getWidth( ) == b->getWidth( ) );

  const int width = a->getWidth( );

  if ( a->isConstant( ) && b->isConstant( ) )
  {
    mpz_class aval( a->getCar( )->getName( ), 2 );
    mpz_class bval( b->getCar( )->getName( ), 2 );
    aval = aval * bval;
    return makeNumberFromGmp( aval, width );
  }

  if ( a->isConstant( ) && !b->isConstant( ) )
  {
    Enode * tmp = a;
    a = b;
    b = tmp;
  }

  string zero_str;
  zero_str.insert( 0, width, '0' );
  Enode * zero = mkBvnum( const_cast< char * >( zero_str.c_str( ) ) );
  string one_str;
  one_str.insert( 0, width - 1, '0' );
  one_str.push_back( '1' );
  Enode * one = mkBvnum( const_cast< char * >( one_str.c_str( ) ) );

  Enode * res = NULL;
  if ( a == zero || b == zero ) res = zero;
  else if ( a == one ) res = b;
  else if ( b == one ) res = a;
  else res = cons( id_to_enode[ ENODE_ID_BVMUL ], args );

  assert( res );
  return res;
}

//
// Translate signed division into unsigned one
//
Enode * Egraph::mkBvsdiv ( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );
  Enode * s = args->getCar( );
  Enode * t = args->getCdr( )->getCar( );
  assert( s->getWidth( ) == t->getWidth( ) );
  const int width = s->getWidth( );

  Enode * msb_s = mkExtract( width - 1, width - 1, s );
  Enode * msb_t = mkExtract( width - 1, width - 1, t );
  Enode * bit0  = mkBvnum( "0" );
  Enode * bit1  = mkBvnum( "1" );

  Enode * cond1 = mkAnd( cons( mkEq( cons( msb_s, cons( bit0 ) ) )
                       , cons( mkEq( cons( msb_t, cons( bit0 ) ) )
		       ) ) );

  Enode * case1 = mkBvudiv( cons( s, cons( t ) ) );

  Enode * cond2 = mkAnd( cons( mkEq( cons( msb_s, cons( bit1 ) ) )
                       , cons( mkEq( cons( msb_t, cons( bit0 ) ) )
		       ) ) );

  Enode * case2 = mkBvneg( cons( mkBvudiv( cons( mkBvneg( cons( s ) ), cons( t ) ) ) ) );

  Enode * cond3 = mkAnd( cons( mkEq( cons( msb_s, cons( bit0 ) ) )
                       , cons( mkEq( cons( msb_t, cons( bit1 ) ) )
		       ) ) );

  Enode * case3 = mkBvneg( cons( mkBvudiv( cons( s, cons( mkBvneg( cons( t ) ) ) ) ) ) );

  Enode * case4 = mkBvudiv( cons( mkBvneg( cons( s ) ), cons( mkBvneg( cons( t ) ) ) ) );

  Enode * res = mkIte( cond1
                     , case1
	             , mkIte( cond2
			    , case2
			    , mkIte( cond3
			           , case3
			           , case4 ) ) );

  return res;
}

//
// Translate signed division into unsigned one
//
Enode * Egraph::mkBvsrem ( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );
  Enode * s = args->getCar( );
  Enode * t = args->getCdr( )->getCar( );
  assert( s->getWidth( ) == t->getWidth( ) );
  const int width = s->getWidth( );

  Enode * msb_s = mkExtract( width - 1, width - 1, s );
  Enode * msb_t = mkExtract( width - 1, width - 1, t );
  Enode * bit0  = mkBvnum( "0" );
  Enode * bit1  = mkBvnum( "1" );

  Enode * cond1 = mkAnd( cons( mkEq( cons( msb_s, cons( bit0 ) ) )
                       , cons( mkEq( cons( msb_t, cons( bit0 ) ) )
		       ) ) );

  Enode * case1 = mkBvurem( cons( s, cons( t ) ) );

  Enode * cond2 = mkAnd( cons( mkEq( cons( msb_s, cons( bit1 ) ) )
                       , cons( mkEq( cons( msb_t, cons( bit0 ) ) )
		       ) ) );

  Enode * case2 = mkBvneg( cons( mkBvurem( cons( mkBvneg( cons( s ) ), cons( t ) ) ) ) );

  Enode * cond3 = mkAnd( cons( mkEq( cons( msb_s, cons( bit0 ) ) )
                       , cons( mkEq( cons( msb_t, cons( bit1 ) ) )
		       ) ) );

  Enode * case3 = mkBvurem( cons( s, cons( mkBvneg( cons( t ) ) ) ) );

  Enode * case4 = mkBvneg( cons( mkBvurem( cons( mkBvneg( cons( s ) ), cons( mkBvneg( cons( t ) ) ) ) ) ) );

  Enode * res = mkIte( cond1
                     , case1
	             , mkIte( cond2
			    , case2
			    , mkIte( cond3
			           , case3
			           , case4 ) ) );

  return res;
}

// Logical shift right
Enode * Egraph::mkBvlshr ( Enode * args )
{
  assert( args );
  Enode * term = args->getCar( );
  Enode * num  = args->getCdr( )->getCar( );
  assert( num->isConstant( ) );
  //
  // Convert number into decimal
  //
  const int num_width = num->getWidth( );
  const char * str = num->getCar( )->getName( );

  assert( num_width == (int)strlen( str ) );
  //
  // Skip leading zeros
  //
  int i;
  for ( i = 0 ; i < num_width && str[ i ] == '0' ; i ++ )
    ;
  //
  // Return term if shift by zero
  //
  if ( i == num_width )
    return term;

  i ++;
  unsigned dec_value = 1;
  for ( ; i < num_width ; i ++ )
  {
    dec_value = dec_value << 1;
    if ( str[ i ] == '1' )
      dec_value ++;
  }

  const int term_width = term->getWidth( );

  if( (int)dec_value >= term->getWidth( ) )
  {
    string zero;
    zero.insert( 0, term->getWidth( ), '0' );
    Enode * res = mkBvnum ( const_cast< char * >( zero.c_str( ) ) );
    assert( res->getWidth( ) == term->getWidth( ) );
    return res;
  }

  assert( (int)dec_value < term->getWidth( ) );
  //
  // Translate shift into concatenation and extraction
  //
  Enode * ext = mkExtract( term_width - 1, dec_value, term );
  assert( ext->getWidth( ) == term_width - (int)dec_value );

  string leading_zeros;
  leading_zeros.insert( 0, dec_value, '0' );

  Enode * lea = mkBvnum ( const_cast< char * >( leading_zeros.c_str( ) ) );
  Enode * con = mkConcat( cons( lea, cons( ext ) ) );

  assert( con->getWidth( ) == term->getWidth( ) );
  return con;
}

//
// Arithmetic shift right
//
Enode * Egraph::mkBvashr ( Enode * args )
{
  assert( args );
  return cons( id_to_enode[ ENODE_ID_BVASHR ], args );
}

//
// Rotate left
// Rewrite as x[width-i-1:0]::x[width-1:width-i]
//              x[j_h:i_h]::x[j_l:i_l]
//
Enode * Egraph::mkRotateLeft( int i, Enode * x )
{
  assert( x );
  assert( x->isTerm( ) );
  const int width = x->getWidth( );

  i = i % width;
  if ( i == 0 )
    return x;

  const int j_h = width - i - 1;
  const int i_h = 0;
  const int j_l = width - 1;
  const int i_l = width - i;
  assert( j_h - i_h + 1 + j_l - i_l + 1 == width );
  Enode * h = mkExtract( j_h, i_h, x );
  Enode * l = mkExtract( j_l, i_l, x );
  return mkConcat( cons( h, cons( l ) ) );
}

//
// Rotate right
// Rewrite as x[i-1:0]::x[width-1:i]
//          x[j_h:i_h]::x[j_l:i_l]
//
Enode * Egraph::mkRotateRight( int i, Enode * x )
{
  assert( x );
  assert( x->isTerm( ) );
  const int width = x->getWidth( );

  i = i % width;
  if ( i == 0 )
    return x;

  const int j_h = i - 1;
  const int i_h = 0;
  const int j_l = width - 1;
  const int i_l = i;
  assert( j_h - i_h + 1 + j_l - i_l + 1 == width );
  Enode * h = mkExtract( j_h, i_h, x );
  Enode * l = mkExtract( j_l, i_l, x );
  return mkConcat( cons( h, cons( l ) ) );
}

//
// Shift left
//
Enode * Egraph::mkBvshl ( Enode * args )
{
  assert( args );
  Enode * term = args->getCar( );
  Enode * num  = args->getCdr( )->getCar( );

  if ( term->isConstant( )
    && !num->isConstant( ) )
  {
    //
    // Special case for spear benchmarks
    //
    string one;
    one.insert( 0, term->getWidth( ) - 1, '0' );
    one += '1';
    string zero;
    zero.insert( 0, term->getWidth( ), '0' );
    Enode * bv_one  = mkBvnum( const_cast< char * >( one .c_str( ) ) );
    Enode * bv_zero = mkBvnum( const_cast< char * >( zero.c_str( ) ) );
    if ( term == bv_one )
      return mkIte( mkEq( cons( num, cons( bv_zero ) ) ), term, bv_zero );
  }

  if( !num->isConstant( ) )
    return cons( id_to_enode[ ENODE_ID_BVSHL ], args );
  //
  // Convert number into decimal
  //
  const int num_width = num->getWidth( );
  const char * str = num->getCar( )->getName( );

  assert( num_width == (int)strlen( str ) );
  //
  // Skip leading zeros
  //
  int i;
  for ( i = 0 ; i < num_width && str[ i ] == '0' ; i ++ )
    ;
  //
  // Return term if shift by zero
  //
  assert( i <= num_width );
  assert( i != num_width || str[ i - 1 ] == '0' );
  if ( i == num_width )
    return term;

  i ++;
  unsigned dec_value = 1;
  for ( ; i < num_width ; i ++ )
  {
    dec_value = dec_value << 1;
    if ( str[ i ] == '1' )
      dec_value ++;
  }

  const int term_width = term->getWidth( );
  assert( (int)dec_value < term->getWidth( ) );
  //
  // Translate shift into concatenation and extraction
  //
  Enode * ext = mkExtract( term_width - dec_value - 1, 0, term );

  string trailing_zeros;
  trailing_zeros.insert( 0, dec_value, '0' );

  Enode * tra = mkBvnum ( const_cast< char * >( trailing_zeros.c_str( ) ) );
  Enode * con = mkConcat( cons( ext, cons( tra ) ) );

  assert( con->getWidth( ) == term->getWidth( ) );

  return con;
}

//
// Translate it into (bvadd (bvnot x) 1)
//
Enode * Egraph::mkBvneg( Enode * args )
{
  assert( args->getArity( ) == 1 );
  Enode * bvnot = mkBvnot( args );
  Enode * e = args->getCar( );
  string num_str;
  num_str.insert( 0, e->getWidth( ) - 1, '0' );
  num_str.push_back( '1' );
  Enode * bvnum = mkBvnum( const_cast< char * >( num_str.c_str( ) ) );
  return mkBvadd( cons( bvnot, cons( bvnum ) ) );
}

Enode * Egraph::mkBvsub( Enode * args )
{
  Enode * a = args->getCar( );
  Enode * b = args->getCdr( )->getCar( );
  char buf[ 32 ];
  sprintf( buf, "bv1[%d]", a->getWidth( ) );
  Enode * one   = mkBvnum( buf );
  Enode * neg_b = mkBvneg( cons( b ) );
  Enode * res   = mkBvadd( cons( a, cons( neg_b ) ) );
  return res;
}

Enode * Egraph::mkBvand( Enode * args )
{
  assert( args );
  assert( args->isList( ) );
  Enode * res = NULL;

  Enode * bv0 = mkBvnum( const_cast< char * >( "0" ) );
  Enode * bv1 = mkBvnum( const_cast< char * >( "1" ) );

  initDup1( );

  list< Enode * > new_args;
  for ( Enode * list = args ; !list->isEnil( ) ; list = list->getCdr( ) )
  {
    Enode * e = list->getCar( );
    assert( e->isDTypeBitVec( ) );

    if ( isDup1( e ) ) continue;

    if ( e == bv1 )
      continue;

    if ( e == bv0 )
    {
      doneDup1( );
      return bv0;
    }

    new_args.push_front( e );
    storeDup1( e );

    assert( (*new_args.begin( ))->getWidth( ) == new_args.back( )->getWidth( ) );
  }

  doneDup1( );

  if ( new_args.size( ) == 0 )
    res = bv1;
  else if ( new_args.size( ) == 1 )
    res = new_args.back( );
  else
    res = cons( id_to_enode[ ENODE_ID_BVAND ], cons( new_args ) );

  assert( res );
  return res;
}

Enode * Egraph::mkBvor( Enode * args )
{
  assert( args );

  Enode * res = NULL;
  Enode * bv0 = mkBvnum( const_cast< char * >( "0" ) );
  Enode * bv1 = mkBvnum( const_cast< char * >( "1" ) );

  initDup1( );
  list< Enode * > new_args;
  for ( Enode * list = args ; !list->isEnil( ) ; list = list->getCdr( ) )
  {
    Enode * e = list->getCar( );
    assert( e->isDTypeBitVec( ) );

    // Redundant argument
    if ( e == bv0 )
      continue;
    // Return 1 if 1 is an argument
    if ( e == bv1 ) { doneDup1( ); return bv1; }

    if ( isDup1( e ) )
      continue;

    new_args.push_front( e );
    storeDup1( e );

    assert( (*new_args.begin( ))->getWidth( ) == new_args.back( )->getWidth( ) );
  }
  doneDup1( );

  if ( new_args.size( ) == 0 )
    res = bv0;
  else if ( new_args.size( ) == 1 )
    res = new_args.back( );
  else
    res = cons( id_to_enode[ ENODE_ID_BVOR ], cons( new_args ) );

  assert( res );
  return res;
}

Enode * Egraph::mkBvnot( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 1 );
  Enode * arg = args->getCar( );
  assert( arg->isDTypeBitVec( ) );

  Enode * res = NULL;

  if ( arg->isConstant( ) )
  {
    const char * bin_value = arg->getCar( )->getName( );
    char new_bin_value[ strlen( bin_value ) + 1 ];
    unsigned i;
    for ( i = 0 ; i < strlen( bin_value ) ; i ++ )
      new_bin_value[ i ] = ( bin_value[ i ] == '0' ? '1' : '0' );
    new_bin_value[ i ] = '\0';

    assert( strlen( new_bin_value ) == strlen( bin_value ) );
    res = mkBvnum( new_bin_value );
  }
  //
  // (bvnot (bvnot x)) --> x
  //
  else if ( arg->isBvnot( ) )
    res = arg->get1st( );
  else
    res = cons( id_to_enode[ ENODE_ID_BVNOT ], args );

  assert( res );
  return res;
}

Enode * Egraph::mkBvxor( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );
  Enode * a = args->getCar( );
  Enode * b = args->getCdr( )->getCar( );
  Enode * res = NULL;

  if( a->isConstant( ) && b->isConstant( ) )
  {
    const char * a_value = a->getCar( )->getName( );
    const char * b_value = b->getCar( )->getName( );
    char new_value[ strlen( a_value ) + 1 ];

    assert( strlen( b_value ) == strlen( a_value ) );

    unsigned i;
    for ( i = 0 ; i < strlen( a_value ) ; i ++ )
      new_value[ i ] = ( a_value[ i ] == b_value[ i ] ? '0' : '1' );
    new_value[ i ] = '\0';

    assert( strlen( new_value ) == strlen( a_value ) );
    res = mkBvnum( new_value );
  }
  else
    res = cons( id_to_enode[ ENODE_ID_BVXOR ], args );

  return res;
}

//
// Sign extend a variable x_n
//
Enode * Egraph::mkSignExtend( int i, Enode * x )
{
  assert( x );
  assert( x->isTerm( ) );

  Enode * res = NULL;

  if ( x->isConstant( ) )
  {
    // Retrieve msb
    const char msb_chr = *(x->getCar( )->getName( ));
    // Generate trailing ones or zeros
    string leading;
    leading.insert( 0, i, msb_chr );
    Enode * extension = mkBvnum( const_cast< char * >( leading.c_str( ) ) );
    res = mkConcat( cons( extension, cons( x ) ) );
  }
  else
  {
    // If already there
    Enode * e = NULL;
    if ( i < se_store.size( ) && se_store[ i ] != NULL )
      e = se_store[ i ];
    else
    {
      if ( i >= se_store.size( ) )
	se_store.resize( i + 1, NULL );
      assert( i < se_store.size( ) );
      assert( se_store[ i ] == NULL );
      char name[ 32 ];
      sprintf( name, "sign_extend[%d]", i );
      assert( lookupSymbol( name ) == NULL );
      e = newSymbol( name, DTYPE_BITVEC | (i + x->getWidth( )) );
      se_store[ i ] = e;
    }

    assert( e );
    res = cons( e, cons( x ) );
    res->setWidth( i + x->getWidth( ) );
  }

  assert( res );
  assert( res->getWidth( ) == i + x->getWidth( ) );
  return res;
}

//
// Zero extend a variable x_n
// Rewritten as (0..0 :: x)
//
Enode * Egraph::mkZeroExtend( int i, Enode * x )
{
  assert( x );
  assert( x->isTerm( ) );
  // Create padding 0s
  string num_str;
  num_str.insert( 0, i, '0' );
  Enode * extend_zero = mkBvnum( const_cast< char * >( num_str.c_str( ) ) );
  // Create extension 0
  Enode * res = mkConcat( cons( extend_zero, cons( x ) ) );
  return res;
}

Enode * Egraph::mkUf( const char * name, Enode * args )
{
  Enode * e = lookupSymbol( name );
  if ( e == NULL ) error( "undeclared function symbol ", name );
  return cons( e, args );
}

Enode * Egraph::mkUp( const char * name, Enode * args )
{
  Enode * e = lookupSymbol( name );
  if ( e == NULL ) error( "undeclared predicate symbol ", name );
  return cons( e, args );
}

//
// Shortcut
//
Enode * Egraph::newSymbol( const char * name, const unsigned t )
{
  vector< unsigned > tmp;
  tmp.push_back( t );
  return newSymbol( name, tmp );
}
//
// Creates a new symbol. Name must be new
//
Enode * Egraph::newSymbol( const char * name, vector< unsigned > & sorts )
{
  if ( lookupSymbol( name ) != NULL )
    error( "symbol already declared ", name );

  assert( sorts.size( ) >= 1 );

  const unsigned dtype = sorts.back( );
  sorts.pop_back( );

  Enode * new_enode = new Enode( id_to_enode.size( )
			       , name
                               , ETYPE_SYMB
			       , dtype
			       , sorts );

  insertSymbol( new_enode );	                        // Insert symbol enode
  assert( lookupSymbol( name ) == new_enode );          // Symbol must be there now
  return new_enode;
}

Enode * Egraph::getDefine( const char * name )
{
  Enode * e = lookupDefine( name );
  assert( e );
  assert( e->getCar( ) != NULL );
  return e->getCar( );
}

void Egraph::mkDefine( const char * name, Enode * def )
{
  Enode * e = lookupDefine( name );
  if( e == NULL )
  {
    Enode * new_enode = new Enode( id_to_enode.size( ), def );
    insertDefine( name, new_enode );
  }
  else
  {
    // This symbol has been declared before. We just
    // replace its definition with this new one
    e->setDef( def );
  }
}

Enode * Egraph::mkBvule( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );

  Enode * x = args->getCar( );
  Enode * y = args->getCdr( )->getCar( );

  if ( x->isConstant( ) )
  {
    mpz_class xval( x->getCar( )->getName( ), 2 );
    if ( xval == 0 )
      return mkTrue( );
  }

  if ( y->isConstant( ) )
  {
    mpz_class yval( y->getCar( )->getName( ), 2 );
    if ( yval == 0 )
      return mkEq( cons( x, cons( y ) ) );
  }

  if ( x->isConstant( ) && y->isConstant( ) )
  {
    mpz_class xval( x->getCar( )->getName( ), 2 );
    mpz_class yval( y->getCar( )->getName( ), 2 );
    if ( xval > yval )
      return mkFalse( );
    else
      return mkTrue( );
  }

  return cons( id_to_enode[ ENODE_ID_BVULE ], args );
}

Enode * Egraph::mkBvsle( Enode * args )
{
  // TODO: add more simplifications
  assert( args );
  assert( args->getArity( ) == 2 );

  Enode * x = args->getCar( );
  Enode * y = args->getCdr( )->getCar( );

  if ( x->isConstant( ) && y->isConstant( ) )
  {
    mpz_class xval( x->getCar( )->getName( ), 2 );
    mpz_class yval( y->getCar( )->getName( ), 2 );
    if ( xval == yval )
      return mkTrue( );
  }

  if ( x->getWidth( ) == 1 )
  {
    Enode * bit0  = mkBvnum( "0" );
    Enode * bit1  = mkBvnum( "1" );
    return mkNot( cons( mkAnd( cons( mkEq( cons( x, cons( bit0 ) ) ),
	                       cons( mkEq( cons( y, cons( bit1 ) ) ) ) ) ) ) );
  }

  return cons( id_to_enode[ ENODE_ID_BVSLE ], args );
}

Enode * Egraph::mkEq( Enode * args )
{
  assert( args );
  assert( args->isList( ) );
  assert( args->getCar( ) );
  assert( args->getCdr( )->getCar( ) );

  Enode * x = args->getCar( );
  Enode * y = args->getCdr( )->getCar( );
  assert( !x->isDTypeBitVec( ) || x->getWidth( ) == y->getWidth( ) );

  //
  // Hack for yices parser
  //
  if ( x->isDTypeBool( ) )
  {
    assert( y->isDTypeBool( ) );
    return mkIff( args );
  }

  assert( !x->isDTypeBool( ) );
  assert( !y->isDTypeBool( ) );

  // Two equal terms
  // x = x => true
  if ( x == y )
    return mkTrue( );

  // Two different constants
  // 1 = 0 => false
  if ( x->isConstant( ) && y->isConstant( ) )
    return mkFalse( );

  if ( x->getId( ) > y->getId( ) )
    args = cons( y, cons( x ) );

  return cons( id_to_enode[ ENODE_ID_EQ ], args );
}

Enode * Egraph::mkNeq( Enode * args )
{
  assert( args );
  assert( args->isList( ) );
  assert( args->getCar( ) );
  assert( args->getCdr( )->getCar( ) );

  Enode * x = args->getCar( );
  Enode * y = args->getCdr( )->getCar( );

  assert( !x->isDTypeBool( ) );
  assert( !y->isDTypeBool( ) );

  // Two equal terms
  // x = x => false
  if ( x == y )
    return mkFalse( );

  // Two different constants
  // 1 = 0 => true
  if ( x->isConstant( ) && y->isConstant( ) )
    return mkTrue( );

  if ( x->getId( ) > y->getId( ) )
    args = cons( y, cons( x ) );

  //return cons( id_to_enode[ ENODE_ID_NEQ ], args );
  return mkNot( cons( cons( id_to_enode[ ENODE_ID_EQ ], args ) ) );
}

Enode * Egraph::mkLeq( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );

  Enode * x = args->getCar( );
  Enode * y = args->getCdr( )->getCar( );

  // Two equal terms
  // x = x => true
  if ( x == y )
    return mkTrue( );

  // Two constants: evaluate
  if ( x->isConstant( ) && y->isConstant( ) )
    return x->getCar( )->getValue( ) <= y->getCar( )->getValue( )
         ? mkTrue ( )
	 : mkFalse( );

  return cons( id_to_enode[ ENODE_ID_LEQ ], args );
}

Enode * Egraph::mkNot( Enode * args )
{
  assert( args );
  assert( args->isList( ) );
  assert( args->getCar( ) );
  Enode * arg = args->getCar( );
  assert( arg->isDTypeBool( ) );
  assert( arg->isTerm( ) );
  // not not p --> p
  if ( arg->isNot( ) )
    return arg->get1st( );

  // not false --> true
  if ( arg->isFalse( ) )
    return mkTrue( );

  // not true --> false
  if ( arg->isTrue( ) )
    return mkFalse( );

  return cons( id_to_enode[ ENODE_ID_NOT ], args );
}

Enode * Egraph::mkAnd( Enode * args )
{
  assert( args );
  assert( args->isList( ) );

  initDup1( );

  list< Enode * > new_args;
  for ( Enode * alist = args ; !alist->isEnil( ) ; alist = alist->getCdr( ) )
  {
    Enode * e = alist->getCar( );
    assert( e->isDTypeBool( ) );

    if ( isDup1( e ) ) continue;
    if ( e->isTrue( ) ) continue;
    if ( e->isFalse( ) ) { doneDup1( ); return mkFalse( ); }

    new_args.push_front( e );
    storeDup1( e );
  }

  doneDup1( );

  Enode * res = NULL;

  if ( new_args.size( ) == 0 )
    res = mkTrue( );
  else if ( new_args.size( ) == 1 )
    res = new_args.back( );
  else
    res = cons( id_to_enode[ ENODE_ID_AND ], cons( new_args ) );

  assert( res );
  return res;
}

Enode * Egraph::mkOr( Enode * args )
{
  assert( args );
  assert( args->isList( ) );

  initDup1( );

  list< Enode * > new_args;
  for ( Enode * list = args ; !list->isEnil( ) ; list = list->getCdr( ) )
  {
    Enode * e = list->getCar( );

    assert( e->isDTypeBool( ) );

    if ( isDup1( e ) ) continue;
    if ( e->isFalse( ) ) continue;
    if ( e->isTrue( ) ) { doneDup1( ); return mkTrue( ); }

    new_args.push_front( e );
    storeDup1( e );
  }

  doneDup1( );

  if ( new_args.size( ) == 0 )
    return mkFalse( );

  if ( new_args.size( ) == 1 )
    return new_args.back( );

  return cons( id_to_enode[ ENODE_ID_OR ], cons( new_args ) );
}

Enode * Egraph::mkIff( Enode * args )
{
  assert( args );
  assert( args->getArity( ) == 2 );
  Enode * first  = args->getCar( );
  Enode * second = args->getCdr( )->getCar( );

  if ( first ->isTrue ( ) )               return second;
  if ( first ->isFalse( ) )               return mkNot( cons( second ) );
  if ( second->isTrue ( ) )               return first;
  if ( second->isFalse( ) )               return mkNot( cons( first ) );
  if ( first == second )                  return mkTrue ( );
  if ( first == mkNot( cons( second ) ) ) return mkFalse( );

  return cons( id_to_enode[ ENODE_ID_IFF ], args );
}

Enode * Egraph::mkIfthenelse( Enode * i, Enode * t, Enode * e )
{
  assert( i );
  assert( t );
  assert( e );

  if ( i->isTrue ( ) ) return t;
  if ( i->isFalse( ) ) return e;
  if ( t->isFalse( ) ) return mkAnd( cons( mkNot( cons( i ) ), cons( e ) ) );
  if ( e->isFalse( ) ) return mkAnd( cons( i                 , cons( t ) ) );
  if ( t->isTrue ( ) ) return mkOr ( cons( i                 , cons( e ) ) );
  if ( e->isTrue ( ) ) return mkOr ( cons( mkNot( cons( i ) ), cons( t ) ) );
  if ( t == e )        return t;

  return cons( id_to_enode[ ENODE_ID_IFTHENELSE ], cons( i, cons( t, cons( e ) ) ) );
}

Enode * Egraph::mkIte( Enode * i, Enode * t, Enode * e )
{
  assert( i );
  assert( t );
  assert( e );
  assert( i->isDTypeBool( ) );
  assert( t->getDType( ) == e->getDType( ) );

  if ( i->isTrue( )  ) return t;
  if ( i->isFalse( ) ) return e;
  if ( t == e )        return t;

  has_ites = true;

  return cons( id_to_enode[ ENODE_ID_ITE ], cons( i, cons( t, cons( e ) ) ) );
}

// TODO: add simplifications
Enode * Egraph::mkXor( Enode * args )
{
  assert( args );

  assert( args->getArity( ) == 2 );
  Enode * first  = args->getCar( );
  Enode * second = args->getCdr( )->getCar( );

  if ( first ->isFalse( ) )               return second;
  if ( first ->isTrue ( ) )               return mkNot( cons( second ) );
  if ( second->isFalse( ) )               return first;
  if ( second->isTrue ( ) )               return mkNot( cons( first ) );
  if ( first == second )                  return mkFalse( );
  if ( first == mkNot( cons( second ) ) ) return mkTrue ( );

  return cons( id_to_enode[ ENODE_ID_XOR ], args );
}

Enode * Egraph::mkImplies( Enode * args )
{
  assert( args );

  Enode * first  = args->getCar( );
  Enode * second = args->getCdr( )->getCar( );
  Enode * not_first = mkNot( cons( first ) );

  if ( first ->isFalse( ) ) return mkTrue( );
  if ( second->isTrue ( ) ) return mkTrue( );
  if ( first ->isTrue ( ) ) return second;
  if ( second->isFalse( ) ) return not_first;

  return mkOr( cons( not_first
	     , cons( second ) ) );
}

Enode * Egraph::mkDistinct( Enode * args )
{
  assert( args );
  Enode * res = NULL;
  //
  // Replace distinction with negated equality when it has only 2 args
  //
  if ( args->getArity( ) == 2 )
  {
    Enode * x = args->getCar( );
    Enode * y = args->getCdr( )->getCar( );
    res = mkNot( cons( mkEq( cons( x, cons( y ) ) ) ) );
  }
  else
  {
    res = cons( id_to_enode[ ENODE_ID_DISTINCT ], args );

    // The thing is that this distinction might have been
    // already processed. So if the index is -1 we are sure
    // it is new
    if ( res->getDistIndex( ) == -1 )
    {
      size_t index = index_to_dist.size( );

      if ( index > sizeof( dist_t ) * 8 )
	error( "max number of distinctions supported is " ,sizeof( dist_t ) * 8 );

      res->setDistIndex( index );
      // Store the correspondence
      index_to_dist.push_back( res );
      // Check invariant
      assert( index_to_dist[ index ] == res );
    }
  }

  assert( res );
  return res;
}

//
// Packs assumptions and formula and return it into a single enode
//
Enode * Egraph::getFormula( )
{
  if ( assumptions.empty( ) )
  {
    if ( top == NULL )
      error( "no formula statement defined", "" );

    return top;
  }

  // Pack the formula and the assumptions
  // into an AND statement, and return it
  if ( top != NULL )
    assumptions.push_back( top );

  Enode * cdr = const_cast<Enode *>( enil );

  for ( unsigned i = 0 ; i < assumptions.size( ) ; i ++ )
    cdr = cons( assumptions[ i ], cdr );

  return mkAnd( cdr );
}

Enode * Egraph::mkWord1cast( Enode * arg )
{
  if ( arg->isDTypeBitVec( ) ) return arg;

  assert( arg->isDTypeBool( ) );

  if ( arg->isWord1cast( ) ) return arg;
  if ( arg->isBoolcast ( ) ) return arg->get1st( );
  if ( arg->isTrue     ( ) ) return mkBvnum( const_cast< char * >( "1" ) );
  if ( arg->isFalse    ( ) ) return mkBvnum( const_cast< char * >( "0" ) );

  return cons( id_to_enode[ ENODE_ID_WORD1CAST ], cons( arg ) );
}

Enode * Egraph::mkBoolcast( Enode * arg )
{
  if ( arg->isDTypeBool( ) ) return arg;

  assert( arg->isDTypeBitVec( ) );
  assert( arg->getWidth( ) == 1 );

  if ( arg->isBoolcast ( ) ) return arg;
  if ( arg->isWord1cast( ) ) return arg->get1st( );
  if ( arg == mkBvnum( const_cast< char * >( "1" ) ) ) return mkTrue ( );
  if ( arg == mkBvnum( const_cast< char * >( "0" ) ) ) return mkFalse( );

  return cons( id_to_enode[ ENODE_ID_BOOLCAST ], cons( arg ) );
}

//
// Computes the polarities for theory atoms and
// set the decision polarity
// Nodes with both polarities gets a random polarity
//
void Egraph::computePolarities( Enode * formula )
{
  assert( config.logic != UNDEF );

  vector< Enode * > unprocessed_enodes;
  initDup1( );

  unprocessed_enodes  .push_back( formula );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    unprocessed_enodes  .pop_back( );
    //
    // Skip if the node has already been processed before
    //
    if ( isDup1( enode ) )
      continue;

    //
    // Process children
    //
    if ( enode->isBooleanOperator( ) )
    {
      Enode * arg_list;
      for ( arg_list = enode->getCdr( )
	  ; !arg_list->isEnil( )
	  ; arg_list = arg_list->getCdr( ) )
      {
	Enode * arg = arg_list->getCar( );
	assert( arg->isTerm( ) );
	unprocessed_enodes  .push_back( arg );
      }

      storeDup1( enode );
      continue;
    }

    assert( enode->isAtom( ) );
    //
    // Skip Boolean Variables
    //
    if ( !enode->isTAtom( ) )
      continue;
    //
    // Here set polarity
    //
    if ( config.logic == QF_UF )
    {
      if ( enode->isEq( ) )
      {
	Enode * lhs = enode->get1st( );
	Enode * rhs = enode->get2nd( );
	//
	// Equality between the same f
	//
	if ( lhs->getCar( ) == rhs->getCar( ) )
	  enode->setDecPolarity( l_False );
	else
	  enode->setDecPolarity( l_True );
      }
      else if ( enode->isUp( ) )
	enode->setDecPolarity( l_False );
    }
    else if ( config.logic == QF_IDL
	   || config.logic == QF_RDL )
    {
      assert( enode->isLeq( ) );
      Enode * lhs = enode->get1st( );
      Enode * rhs = enode->get2nd( );
      const bool lhs_v_c = lhs->isVar( ) || lhs->isConstant( ) || ( lhs->isUminus( ) && lhs->get1st( )->isConstant( ) );
      const bool rhs_v_c = rhs->isVar( ) || rhs->isConstant( ) || ( rhs->isUminus( ) && rhs->get1st( )->isConstant( ) );
      Real weight;

      if ( lhs_v_c && rhs_v_c )
      {
	if ( lhs->isVar( ) && rhs->isVar( ) )
	  weight = 0;
	else if ( lhs->isVar( ) )
	{
	  assert( rhs->isConstant( ) || ( rhs->isUminus( ) && rhs->get1st( )->isConstant( ) ) );
#if FAST_RATIONALS
	  weight = rhs->isConstant( ) ? rhs->getCar( )->getValue( ) : Real(-rhs->get1st( )->getCar( )->getValue( ));
#else
	  weight = rhs->isConstant( ) ? rhs->getCar( )->getValue( ) : -1 * rhs->get1st( )->getCar( )->getValue( );
#endif
	}
	else
	{
	  assert( lhs->isConstant( ) || ( lhs->isUminus( ) && lhs->get1st( )->isConstant( ) ) );
#if FAST_RATIONALS
	  weight = lhs->isConstant( ) ? lhs->getCar( )->getValue( ) : -lhs->get1st( )->getCar( )->getValue( );
#else
	  weight = lhs->isConstant( ) ? lhs->getCar( )->getValue( ) : -1 * lhs->get1st( )->getCar( )->getValue( );
#endif

	}
      }
      else
      {
	Enode * d = enode->get1st( )->isMinus( ) ? enode->get1st( ) : enode->get2nd( );
	Enode * c = enode->get1st( )->isMinus( ) ? enode->get2nd( ) : enode->get1st( );

	assert( c->isConstant( ) || ( c->isUminus( ) && c->get1st( )->isConstant( ) ) );
#if FAST_RATIONALS
	weight = c->isConstant( ) ? c->getCar( )->getValue( ) : -c->get1st( )->getCar( )->getValue( );
	weight = enode->get1st( )->isMinus( ) ? weight : -weight;
#else
	weight = c->isConstant( ) ? c->getCar( )->getValue( ) : -1 * c->get1st( )->getCar( )->getValue( );
	weight = enode->get1st( )->isMinus( ) ? weight : -1 * weight;
#endif

      }
      //
      // Decide polarity
      //
      if ( weight < 0 )
	enode->setDecPolarity( l_True );	// Decide to assign positively first
      else
	enode->setDecPolarity( l_False );	// Decide to assign negatively first

#if FAST_RATIONALS
      Real abs_weight = weight < 0 ? -weight : weight;
      int w = (int)abs_weight.get_d( );
#elif USE_GMP
      Real abs_weight = weight < 0 ? -1 * weight : weight;
      int w = (int)abs_weight.get_d( );
#else
      Real abs_weight = weight < 0 ? -1 * weight : weight;
      int w = (int)abs_weight;
#endif

      enode->setWeightInc( w + 1 );
    }
    else if ( config.logic == QF_LRA )
    {
      assert( enode->isLeq( ) );
      Enode * lhs = enode->get1st( );
      Enode * rhs = enode->get2nd( );
      const bool lhs_c = lhs->isConstant( ) || ( lhs->isUminus( ) && lhs->get1st( )->isConstant( ) );
      const bool rhs_c = rhs->isConstant( ) || ( rhs->isUminus( ) && rhs->get1st( )->isConstant( ) );
      assert( lhs_c || rhs_c );
      Real weight;

#if FAST_RATIONALS
      if ( lhs_c )
	weight = lhs->isUminus( ) ? lhs->get1st( )->getCar( )->getValue( ) : Real(-lhs->getCar( )->getValue( ));
      else
	weight = rhs->isUminus( ) ? Real(-rhs->get1st( )->getCar( )->getValue( )) : rhs->getCar( )->getValue( );
#else
      if ( lhs_c )
	weight = lhs->isUminus( ) ? lhs->get1st( )->getCar( )->getValue( ) : -1 * lhs->getCar( )->getValue( );
      else
	weight = rhs->isUminus( ) ? -1 * rhs->get1st( )->getCar( )->getValue( ) : rhs->getCar( )->getValue( );
#endif
      //
      // Decide polarity
      //
      if ( weight < 0 )
	enode->setDecPolarity( l_True );	// Decide to assign positively first
      else
	enode->setDecPolarity( l_False );	// Decide to assign negatively first
    }

    storeDup1( enode );
  }

  // Done with cache
  doneDup1( );
}

Enode *
Egraph::makeNumberFromGmp( mpz_class & n, const int width )
{
  assert( n >= 0 );
  string s = n.get_str( 2 );
  string new_bin_value;
  //
  // Handle overflow
  //
  if ( (int)s.size( ) > width )
  {
    s = s.substr( s.size( ) - width, width );
    assert( (int)s.size( ) == width );
  }
  assert( width >= (int)s.size( ) );
  if( width - (int)s.size( ) > 0 )
    new_bin_value.insert( 0, width - s.size( ), '0' );
  new_bin_value += s;
  return mkBvnum( const_cast< char * >(new_bin_value.c_str( )) );
}

//=================================================================================================
// Debugging Routines

#ifdef STATISTICS
void Egraph::printMemStats( ostream & os )
{
  unsigned total = 0;

  for ( unsigned i = 0 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];
    assert( e != NULL );
    total += e->sizeInMem( );
  }

  os << "# " << endl;
  os << "# General Statistics" << endl;
  os << "#" << endl;
  os << "# Total enodes..........: " << id_to_enode.size( ) << endl;
  os << "# Enode size in memory..: " << ( total / 1048576.0 ) << " MB" << endl;
  os << "# Avg size per enode....: " << ( total / id_to_enode.size( ) ) << " B" << endl;
  os << "#" << endl;
  os << "# Splay Tree Statistics" << endl;
  store.printStatistics( os );
  os << "#" << endl;
  os << "# Signature Table Statistics" << endl;
  enodeid_t maximal;
  sig_tab.printStatistics( os, &maximal );
  os << "# Maximal node..........: " << id_to_enode[ maximal ] << endl;
  os << "#" << endl;
  os << "# Supporting data structures" << endl;
  os << "#" << endl;
  os << "# id_to_enode........: " << id_to_enode.size( ) * sizeof( Enode * ) / 1048576.0 << " MB" << endl;
  os << "# duplicates1........: " << duplicates1.size( ) * sizeof( int ) / 1048576.0 << " MB" << endl;
  os << "# duplicates2........: " << duplicates2.size( ) * sizeof( int ) / 1048576.0 << " MB" << endl;
  os << "# dup_map............: " << dup_map.size( ) * sizeof( Enode * ) / 1048576.0 << " MB" << endl;
  os << "# dup_set............: " << dup_set.size( ) * sizeof( int ) / 1048576.0 << " MB" << endl;
  os << "# id_to_belong_mask..: " << id_to_belong_mask.size( ) * sizeof( int ) / 1048576.0 << " MB" << endl;
  os << "# id_to_fan_in.......: " << id_to_fan_in.size( ) * sizeof( int ) / 1048576.0 << " MB" << endl;
  os << "# index_to_dist......: " << index_to_dist.size( ) * sizeof( Enode * ) / 1048576.0 << " MB" << endl;
  os << "# cache..............: " << cache.size( ) * sizeof( Enode * ) / 1048576.0 << " MB" << endl;
  os << "# ext_store..........: " << ext_store.size( ) * sizeof( pair< pair< int, int >, Enode * > ) / 1048576.0 << " MB" << endl;
  os << "# se_store...........: " << se_store.size( ) * sizeof( pair< pair< int, int >, Enode * > ) / 1048576.0 << " MB" << endl;
  os << "# id_to_inc_edges....: " << id_to_inc_edges.size( ) * sizeof( int ) / 1048576.0 << " MB" << endl;
  os << "#" << endl;

}

void Egraph::printEnodeList( ostream & os )
{
  os << "# =================================================" << endl;
  os << "# Enode Bank Information" << endl;
  os << "# " << endl;
  os << "# -----+-------------------------------------------" << endl;
  os << "# Enode Symbol List" << endl;
  os << "# -----+-------------------------------------------" << endl;
  for ( unsigned i = 0 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];

    if( e->getId( ) <= ENODE_ID_LAST )
      continue;

    if( e->isSymb( ) || e->isNumb( ) || e->isDef( ) )
    {
      // Print index formatted
      stringstream tmp; tmp << i;
      os << "# ";
      for ( int j = 3 - tmp.str( ).size( ) ; j >= 0 ; j -- ) os << " ";
      os << tmp.str( ) << " | ";

      e->print( os );
      os << endl;
    }
  }
  os << "# -----+-------------------------------------------" << endl;
  os << "# Enode Term List" << endl;
  os << "# -----+-------------------------------------------" << endl;
  for ( unsigned i = 0 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];
    if( e->isTerm( ) )
    {
      // Print index formatted
      stringstream tmp; tmp << i;
      os << "# ";
      for ( int j = 3 - tmp.str( ).size( ) ; j >= 0 ; j -- ) os << " ";
      os << tmp.str( ) << " | ";

      e->print( os );
      os << endl;
    }
  }
  os << "# -----+-------------------------------------------" << endl;
  os << "# Enode List List" << endl;
  os << "# -----+-------------------------------------------" << endl;
  for ( unsigned i = 0 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];
    if( e->isList( ) )
    {
      // Print index formatted
      stringstream tmp; tmp << i;
      os << "# ";
      for ( int j = 3 - tmp.str( ).size( ) ; j >= 0 ; j -- ) os << " ";
      os << tmp.str( ) << " | ";

      e->print( os );
      os << endl;
    }
  }
}
#endif

#ifndef SMTCOMP
void Egraph::dumpToFile( const char * filename, Enode * formula )
{
  ofstream dump_out ( filename );
  dumpHeaderToFile  ( dump_out );
  dumpFormulaToFile ( dump_out, formula );
  dump_out.close( );
}

void Egraph::dumpHeaderToFile( ofstream & dump_out )
{
  dump_out << "(benchmark dumped_with_OpenSMT" << endl;
  // Print logic
  dump_out << ":logic ";
  dump_out << logicStr( config.logic ) << endl;
  // Print status
  dump_out << ":status " << (config.status == l_True ? "sat" : (config.status == l_False ? "unsat" : "unknown")) << endl;
  // Print extrasorts
  for ( MapNameUint::iterator it = name_to_extrasort.begin( )
      ; it != name_to_extrasort.end( )
      ; it ++ )
    dump_out << ":extrasorts( " << it->first << " )" << endl;
  // Print extrapreds/funcs
  for ( MapNameEnode::iterator it = name_to_symbol.begin( )
      ; it != name_to_symbol.end( )
      ; it ++ )
  {
    Enode * e = (it->second);
    enodeid_t id = (it->second)->getId( );

    // Skip if predefined symbol
    if ( id <= ENODE_ID_LAST )
      continue;
    int l, m;
    char * s;
    // Skip extraction
    if ( sscanf( (it->first).c_str( ), "extract[%d:%d]", &m, &l ) == 2 )
      continue;
    // Skip sign extend
    if ( sscanf( (it->first).c_str( ), "sign_extend[%d]", &l ) == 1 )
      continue;
    // Skip bitblated variables
    if ( (it->first).c_str( )[0] == '_' )
      continue;

    // Uninterpreted predicate or boolean variable
    if ( e->isDTypeBool( ) )
    {
      dump_out << ":extrapreds(( " << it->first;
      vector< unsigned > & args_sorts = e->getSort( );
      for ( unsigned i = 0 ; i < args_sorts.size( ) ; i ++ )
      {
	if ( args_sorts[ i ] == DTYPE_U )
	  dump_out << " U";
	else if ( args_sorts[ i ] == DTYPE_REAL )
	  dump_out << " Real";
	else if ( args_sorts[ i ] == DTYPE_INT )
	  dump_out << " Int";
	else if ( args_sorts[ i ] == DTYPE_BITVEC )
	  assert( false );
	else
	{
	  assert( extrasort_to_name.find( args_sorts[ i ] ) != extrasort_to_name.end( ) );
	  dump_out << " " << extrasort_to_name[ args_sorts[ i ] ];
	}
      }
      dump_out << " ))" << endl;
    }
    // Uninterpreted functions
    else
    {
      dump_out << ":extrafuns(( " << it->first;
      vector< unsigned > & args_sorts = e->getSort( );
      for ( unsigned i = 0 ; i < args_sorts.size( ) ; i ++ )
      {
	if ( args_sorts[ i ] == DTYPE_U )
	  dump_out << " U";
	else if ( args_sorts[ i ] == DTYPE_REAL )
	  dump_out << " Real";
	else if ( args_sorts[ i ] == DTYPE_INT )
	  dump_out << " Int";
	else if ( args_sorts[ i ] == DTYPE_BITVEC )
	  assert( false );
	else
	{
	  assert( extrasort_to_name.find( args_sorts[ i ] ) != extrasort_to_name.end( ) );
	  dump_out << " " << extrasort_to_name[ args_sorts[ i ] ];
	}
      }
      if ( e->isDTypeU( ) )
	dump_out << " U";
      else if ( e->isDTypeReal( ) )
	dump_out << " Real";
      else if ( e->isDTypeInt( ) )
	dump_out << " Int";
      else if ( e->isDTypeBitVec( ) )
	dump_out << " BitVec[" << e->getWidth( ) << "]";
      else
      {
	dump_out << " SORT" << e->getDType( );
      }
      dump_out << " ))" << endl;
    }
  }
}

#define PRINT_MAX_SHARED 0

void Egraph::dumpFormulaToFile( ofstream & dump_out, Enode * formula )
{
  int par_to_close = 0;

  dump_out << ":formula" << endl;

  vector< Enode * > unprocessed_enodes;
  Map( enodeid_t, string ) enode_to_def;

  unprocessed_enodes.push_back( formula );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    //
    // Skip if the node has already been processed before
    //
    if ( enode_to_def.find( enode->getId( ) ) != enode_to_def.end( ) )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = enode->getCdr( )
	; !arg_list->isEnil( )
	; arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed
      //
      if ( enode_to_def.find( arg->getId( ) ) == enode_to_def.end( )
	&& arg->isBooleanOperator( ) )
      {
	unprocessed_enodes.push_back( arg );
	unprocessed_children = true;
      }
    }
    //
    // SKip if unprocessed_children
    //
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );

    char buf[ 32 ];

#if PRINT_MAX_SHARED
    if ( enode->isDTypeBool( ) )
    {
      sprintf( buf, "$def%d", enode->getId( ) );
      dump_out << "(flet (" << buf << " ";
    }
    else
    {
      sprintf( buf, "?def%d", enode->getId( ) );
      dump_out << "(let (" << buf << " ";
    }

    par_to_close ++;

    if ( enode->getArity( ) > 0 ) dump_out << "(";
    dump_out << enode->getCar( );
    for ( Enode * list = enode->getCdr( ) ; !list->isEnil( ) ; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      dump_out << " " << enode_to_def[ arg->getId( ) ];
    }
    if ( enode->getArity( ) > 0 ) dump_out << ")";
    dump_out << ")" << endl;

    assert( enode_to_def.find( enode->getId( ) ) == enode_to_def.end( ) );
    enode_to_def[ enode->getId( ) ] = buf;
#else
    sprintf( buf, "$def%d", enode->getId( ) );
    dump_out << "(flet (" << buf << " ";

    par_to_close ++;

    if ( enode->getArity( ) > 0 ) dump_out << "(";
    dump_out << enode->getCar( );
    for ( Enode * list = enode->getCdr( )
	; !list->isEnil( )
	; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      if ( arg->isBooleanOperator( ) )
	dump_out << " " << enode_to_def[ arg->getId( ) ];
      else
	dump_out << " " << arg;
    }
    if ( enode->getArity( ) > 0 ) dump_out << ")";
    dump_out << ")" << endl;

    assert( enode_to_def.find( enode->getId( ) ) == enode_to_def.end( ) );
    enode_to_def[ enode->getId( ) ] = buf;
#endif
  }

  dump_out << enode_to_def[ formula->getId( ) ] << endl;
  // Close flets/lets
  for ( int i = 0 ; i < par_to_close ; i ++ )
  {
    if ( i % 40 == 0 ) dump_out << endl;
    dump_out << ")";
  }
  // Close benchmark
  dump_out << endl << ")" << endl;
}
#endif
