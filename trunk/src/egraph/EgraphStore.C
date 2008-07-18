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

#include "Egraph.h"

void Egraph::initializeStore( )
{
  //
  // Allocates SMTLIB predefined symbols
  //
  newSymbol( "true" , ENODE_ARITY_0, TYPE_BOOL ); assert( ENODE_ID_TRUE  == id_to_enode.size( ) - 1 );                 
  newSymbol( "false", ENODE_ARITY_0, TYPE_BOOL ); assert( ENODE_ID_FALSE == id_to_enode.size( ) - 1 );                
  //
  // Arithmetic predefined operators and predicates
  //
  newSymbol( "+" , ENODE_ARITY_N, TYPE_ARIT ); assert( ENODE_ID_PLUS   == id_to_enode.size( ) - 1 );
  newSymbol( "-" , ENODE_ARITY_2, TYPE_ARIT ); assert( ENODE_ID_MINUS  == id_to_enode.size( ) - 1 );
  newSymbol( "~" , ENODE_ARITY_1, TYPE_ARIT ); assert( ENODE_ID_UMINUS == id_to_enode.size( ) - 1 );                   
  newSymbol( "*" , ENODE_ARITY_N, TYPE_ARIT ); assert( ENODE_ID_TIMES  == id_to_enode.size( ) - 1 );                                       
  newSymbol( "/" , ENODE_ARITY_2, TYPE_REAL ); assert( ENODE_ID_DIV    == id_to_enode.size( ) - 1 );                                       
  newSymbol( "=" , ENODE_ARITY_N, TYPE_BOOL ); assert( ENODE_ID_EQ     == id_to_enode.size( ) - 1 );                                      
  newSymbol( "<=", ENODE_ARITY_2, TYPE_BOOL ); assert( ENODE_ID_LEQ    == id_to_enode.size( ) - 1 );                                      
  newSymbol( ">=", ENODE_ARITY_2, TYPE_BOOL ); assert( ENODE_ID_GEQ    == id_to_enode.size( ) - 1 );                                      
  newSymbol( "<" , ENODE_ARITY_2, TYPE_BOOL ); assert( ENODE_ID_LT     == id_to_enode.size( ) - 1 );                                       
  newSymbol( ">" , ENODE_ARITY_2, TYPE_BOOL ); assert( ENODE_ID_GT     == id_to_enode.size( ) - 1 );                                       
  //
  // Bit-vector predefined operators and predicates
  //
  newSymbol( "bvslt"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVSLT       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvsgt"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVSGT       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvsle"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVSLEQ      == id_to_enode.size( ) - 1 );                
  newSymbol( "bvsge"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVSGEQ      == id_to_enode.size( ) - 1 );                
  newSymbol( "bvult"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVULT       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvugt"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVUGT       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvule"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVULEQ      == id_to_enode.size( ) - 1 );                
  newSymbol( "bvuge"      , ENODE_ARITY_2, TYPE_BOOL   ); assert( ENODE_ID_BVUGEQ      == id_to_enode.size( ) - 1 );                
  newSymbol( "concat"     , ENODE_ARITY_N, TYPE_BITVEC ); assert( ENODE_ID_CONCAT      == id_to_enode.size( ) - 1 );               
  newSymbol( "distinct"   , ENODE_ARITY_N, TYPE_BOOL   ); assert( ENODE_ID_DISTINCT    == id_to_enode.size( ) - 1 );
  newSymbol( "bvand"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVAND       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvor"       , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVOR        == id_to_enode.size( ) - 1 );                 
  newSymbol( "bvxor"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVXOR       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvnot"      , ENODE_ARITY_1, TYPE_BITVEC ); assert( ENODE_ID_BVNOT       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvadd"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVADD       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvsub"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVSUB       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvmul"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVMUL       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvneg"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVNEG       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvlshr"     , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVLSHR      == id_to_enode.size( ) - 1 );               
  newSymbol( "bvshl"      , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVSHL       == id_to_enode.size( ) - 1 );                
  newSymbol( "bvsrem"     , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVSREM      == id_to_enode.size( ) - 1 );                               
  newSymbol( "bvurem"     , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVUREM      == id_to_enode.size( ) - 1 );                               
  newSymbol( "bvsdiv"     , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVSDIV      == id_to_enode.size( ) - 1 );                               
  newSymbol( "bvudiv"     , ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_BVUDIV      == id_to_enode.size( ) - 1 );                               
  newSymbol( "sign_extend", ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_SIGN_EXTEND == id_to_enode.size( ) - 1 );                                          
  newSymbol( "zero_extend", ENODE_ARITY_2, TYPE_BITVEC ); assert( ENODE_ID_ZERO_EXTEND == id_to_enode.size( ) - 1 );                                          
  //                                                                               
  // Logical predefined predicates
  //
  newSymbol( "implies"     , ENODE_ARITY_2, TYPE_BOOL ); assert( ENODE_ID_IMPLIES    == id_to_enode.size( ) - 1 );
  newSymbol( "and"         , ENODE_ARITY_N, TYPE_BOOL ); assert( ENODE_ID_AND        == id_to_enode.size( ) - 1 );
  newSymbol( "or"          , ENODE_ARITY_N, TYPE_BOOL ); assert( ENODE_ID_OR         == id_to_enode.size( ) - 1 );              
  newSymbol( "not"         , ENODE_ARITY_1, TYPE_BOOL ); assert( ENODE_ID_NOT        == id_to_enode.size( ) - 1 );               
  newSymbol( "iff"         , ENODE_ARITY_N, TYPE_BOOL ); assert( ENODE_ID_IFF        == id_to_enode.size( ) - 1 );                  
  newSymbol( "xor"         , ENODE_ARITY_N, TYPE_BOOL ); assert( ENODE_ID_XOR        == id_to_enode.size( ) - 1 );                  
  newSymbol( "ite"         , ENODE_ARITY_3, TYPE_BOOL ); assert( ENODE_ID_ITE        == id_to_enode.size( ) - 1 );       
  newSymbol( "if_then_else", ENODE_ARITY_3, TYPE_BOOL ); assert( ENODE_ID_IFTHENELSE == id_to_enode.size( ) - 1 );
  //
  // Set top node to empty
  //
  top = NULL;
  //
  // Allocate true and false
  //
  etrue  = allocTrue ( );
  efalse = allocFalse( );
}

//
// Adds a new sort
//
void Egraph::newSort( const char * n )
{
  MapNameInt::iterator it = name_to_extrasort.find( n );

  if ( it != name_to_extrasort.end( ) )
  {
    cerr << "Error: declaring sort " << n << " twice" << endl;
    exit( 1 );
  }
  
  int sort_id = TYPE_U + 1 + (int)name_to_extrasort.size( );
  name_to_extrasort.insert( it, make_pair( string( n ), sort_id ) );
}

//
// Retrieves a sort
//
int Egraph::getSort( const char * n )
{
  MapNameInt::iterator it = name_to_extrasort.find( n );

  if ( it == name_to_extrasort.end( ) )
  {
    cerr << "Error: undeclared sort " << n << endl;
    exit( 1 );
  }

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
  insertSigTab( res );
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
  insertSigTab( res );
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
    id_to_enode.push_back( n );
    assert( n->getId( ) == (int)id_to_enode.size( ) - 1 );
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
  assert( (int)id_to_enode.size( ) == s->getId( ) );
  // Symbol is not there
  assert( name_to_symbol.find( s->getName( ) ) == name_to_symbol.end( ) );
  // Insert Symbol
  name_to_symbol[ s->getName( ) ] = s;
  id_to_enode.push_back( s );
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
  assert( enable_undo );
  MapNameEnode::iterator it = name_to_symbol.find( s->getName( ) );
  assert( it != name_to_symbol.end( ) );
  assert( it->second == s );
  name_to_symbol.erase( it );
  // Only the last added symbol can be removed
  assert( s->getId( ) == (int)id_to_enode.size( ) - 1 );
  id_to_enode.pop_back( );
  delete s;
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
  assert( (int)id_to_enode.size( ) == d->getId( ) );
  assert( name_to_define.find( n ) == name_to_define.end( ) );
  name_to_define[ n ] = d;
  id_to_enode.push_back( d );
}

//
// Insert into signature table possibly creating a new node
//
Enode * Egraph::insertSigTab ( const int id, Enode * car, Enode * cdr )
{ 
  assert( enable_undo );
  assert( car == car->getRoot( ) );
  assert( cdr == cdr->getRoot( ) );
  const Pair( int ) sig = make_pair( car->getCid( ), cdr->getCid( ) );

  MapPairEnode & sig_tab = car->isTerm( ) ? sig_tab_list : sig_tab_term ;
  MapPairEnode::iterator it = sig_tab.find( sig );

  if ( it == sig_tab.end( ) )
  {
    Enode * e = new Enode( id, car->isTerm( ) ? ENODE_TYPE_LIST : ENODE_TYPE_TERM, car, cdr );   
    sig_tab[ sig ] = e;
    return e;
  }
  return it->second;
}

//
// Insert into Store
//
Enode * Egraph::insertStore  ( const int id, Enode * car, Enode * cdr )
{ 
  assert( !enable_undo );
  assert( car == car->getRoot( ) );
  assert( cdr == cdr->getRoot( ) );
  const Pair( int ) sig = make_pair( car->getId( ), cdr->getId( ) );
  MapPairEnode::iterator it = store.find( sig );
  if ( it == store.end( ) )
  {
    Enode * e = new Enode( id, car->isTerm( ) ? ENODE_TYPE_LIST : ENODE_TYPE_TERM, car, cdr );   
    store[ sig ] = e;
    return e;
  }
  return it->second;
}

//
// Retrieve element from signature table
//
Enode * Egraph::lookupSigTab ( Enode * e ) 
{ 
  MapPairEnode & sig_tab = e->isList( ) ? sig_tab_list : sig_tab_term ;
  MapPairEnode::iterator it = sig_tab.find( e->getSig( ) );
  assert( it != sig_tab.end( ) );
  return it->second;
}

//
// Adds element to signature table
//
Enode * Egraph::insertSigTab ( Enode * e ) 
{ 
  MapPairEnode & sig_tab = e->isList( ) ? sig_tab_list : sig_tab_term ;
  pair< MapPairEnode::iterator, bool > res = sig_tab.insert( make_pair( e->getSig( ), e ) );
  if ( res.second )
    return e;
  return res.first->second;
}

//
// Remove element from signature table
//
void Egraph::removeSigTab ( Enode * e ) 
{ 
  MapPairEnode & sig_tab = e->isList( ) ? sig_tab_list : sig_tab_term ;
  assert( sig_tab.find( e->getSig( ) ) != sig_tab.end( ) );
  sig_tab.erase( e->getSig( ) ); 
}

//
// Copy list into a new one whose elements are retrieved from the cache
//
Enode * Egraph::copyEnodeEtypeListWithCache( Enode * list, Map( int, Enode * ) & cache )
{
  if ( list != enil )
  {
    Enode * cdr = copyEnodeEtypeListWithCache( list->getCdr( ), cache );
    assert( cache.find( list->getCar( )->getId( ) ) != cache.end( ) );
    return cons( cache[ list->getCar( )->getId( ) ], cdr ); 
  }

  return const_cast<Enode *>( enil );
}

//
// Creates a new term or list
//
Enode * Egraph::cons( Enode * car, Enode * cdr )
{
  assert( !enable_undo );
  assert( car );
  assert( cdr );
  assert( car->isTerm( ) || car->isSymb( ) || car->isNumb( ) );
  assert( cdr->isList( ) );
  assert( car == car->getRoot( ) );                
  assert( cdr == cdr->getRoot( ) );                
  // Create and insert a new enode if necessary
  Enode * e = insertStore( id_to_enode.size( ), car, cdr );
  // The node was there already. Return it
  if ( (int)id_to_enode.size( ) != e->getId( ) ) return e;
  // We keep the created enode
  id_to_enode.push_back( e );      
  return e;
}

//
// Undoable Cons. For future use
//
Enode * Egraph::uCons( Enode * car, Enode * cdr )
{
  assert( false );
  assert( enable_undo );
  assert( car );
  assert( cdr );
  assert( car->isTerm( ) );
  assert( cdr->isList( ) );
  // Move car and cdr to root
  car = car->getRoot( );                
  cdr = cdr->getRoot( );                
  // Create and insert a new enode if necessary
  Enode * e = insertSigTab( id_to_enode.size( ), car, cdr );
  // The node was there already. Return it
  if ( (int)id_to_enode.size( ) != e->getId( ) ) return e;
  // We keep the created enode
  id_to_enode.push_back( e );      
  // Add Parents
  car->addParent( e );
  cdr->addParent( e );
  // Save Backtrack information
  undo_stack_term.push_back( e );
  undo_stack_oper.push_back( CONS );
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );

  return e;
}

void Egraph::undoCons( Enode * e )
{
  assert( e );
  // assert( checkUFInvariants( sig_tab ) );
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
  car->removeParent( e );
  cdr->removeParent( e );
  // Get rid of the correspondence
  id_to_enode.pop_back( );      
  // Erase the enode
  delete e;
  // assert( checkUFInvariants( sig_tab ) );
}

//
// Create a term 
//
Enode * Egraph::mkVar( const char * name )
{
  Enode * e = lookupSymbol( name );

  if ( e == NULL )
  {
    cerr << "Undeclared symbol: " << name << endl;
    exit( 1 );
  }

  return cons( e );
}

Enode * Egraph::mkNum( const char * value )
{
  Enode * new_enode = new Enode( id_to_enode.size( )
                               , ENODE_TYPE_NUMBER
			       , ENODE_ARITY_0
			       , TYPE_ARIT
			       , value );  
  Enode * res = insertNumber( new_enode );
  return cons( res );
}

Enode * Egraph::mkNum( const Real & real_value )
{
  char value[ 32 ];
  sprintf( value, "%lf", real_value );
  return mkNum( value );
}

Enode * Egraph::mkNum( const char * num, const char * den )
{
  Real real_value( atof( num ) / atof( den ) );	  
  char value[ 32 ];
  sprintf( value, "%lf", real_value );
  return mkNum( value );
}

Enode * Egraph::mkBvnum( char * str )
{
  Enode * new_enode = NULL;

  if ( str[ 0 ] == 'b' && str[ 1 ] == 'v' )
  {
    char * end_value = strchr( str, '[' );
    char * p = &(str[2]);

    int width = 0;
    sscanf( end_value, "[%d]", &width );
    assert( width > 0 );

    char value[ width + 1 ];

    int i = 0;
    while ( p != end_value )
      value[ i++ ] = *p++;
    value[ i ] = '\0';

    long int_value = atol( value );

    for ( int i = 1 ; i <= width ; i ++ )
    {
      if ( int_value & SETBIT( i-1 ) )
	value[ width - i ] = '1';
      else
	value[ width - i ] = '0';
    }
    value[ width ] = '\0';

    new_enode = new Enode( id_to_enode.size( )
	                 , ENODE_TYPE_NUMBER
			 , ENODE_ARITY_0
			 , TYPE_BITVEC | width
			 , value );  
  }
  else
  {
    int width = strlen( str );
    new_enode = new Enode( id_to_enode.size( )
	                 , ENODE_TYPE_NUMBER
			 , ENODE_ARITY_0
			 , TYPE_BITVEC | width
			 , str );
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
  assert( msb <= arg->width( ) - 1 );
  
  const int i = msb, j = lsb;
  //
  // Apply rewrite rules. We assume x to have width n, y to have width m
  //
  // Rule 1:
  // x[n-1:0] --> x
  //
  if ( arg->width( ) == i - j + 1 )
    return arg;
  //
  // Rewrite rules for concatenation
  //
  if ( arg->isConcat( ) )
  {
    assert( arg->getArity( ) == 2 );
    Enode * x = arg->get1st( );
    Enode * y = arg->get2nd( );
    int m = y->width( );
    //
    // Rule 2:
    // (x :: y)[i:j] --> x
    // 
    // where j > m-1
    //
    if ( j > m - 1 )
      return x;
    //
    // Rule 3:
    // (x :: y)[i:j] --> x
    // 
    // where i < m
    //
    if ( i < m )
      return y;
    //
    // Rule 4:
    // (x :: y)[i:j] --> x[i-m:0] :: y[m-1:j]
    //
    // otherwise
    // 
    return mkConcat( cons( mkExtract( m-1, j, y ), cons( mkExtract( i-m, 0, x ) ) ) );
  }
  //
  // Rewrite a selected number as the equivalent number
  //
  if ( arg->isConstant( ) )
  {
    const char * value = arg->getCar( )->getName( );
    int width = arg->width( );

    char new_value[ i - j + 3 ];
    for ( int h = j, k = 0 ; h <= i ; h ++, k ++ )
      new_value[ i - j - k ] = value[ width - 1 - h ];
    new_value[ i - j + 2 ] = '\0';

    return mkBvnum( new_value );
  }

  char name[ 256 ];
  sprintf( name, "extract[%d:%d]", msb, lsb );
  Enode * e = lookupSymbol( name );
  if ( e == NULL )
    e = newSymbol( name, ENODE_ARITY_1, TYPE_BITVEC | (msb - lsb + 1) );
  assert( e );
  return cons( e, cons( arg ) );
}

Enode * Egraph::mkConcat( Enode * args )   
{
  assert( args );
  assert( args->getArity( ) >= 1 );

  if ( args->getArity( ) == 1 )
    return args->getCar( );

  return cons( id_to_enode[ ENODE_ID_CONCAT ], args ); 
}

Enode * Egraph::mkSignExtend( int, Enode * ) { assert( false ); }
Enode * Egraph::mkZeroExtend( int, Enode * ) { assert( false ); }

Enode * Egraph::mkUf( const char * name, Enode * args )
{
  Enode * e = lookupSymbol( name );
  assert( e );
  return cons( e, args );
}

Enode * Egraph::mkUp( const char * name, Enode * args )
{
  Enode * e = lookupSymbol( name );
  assert( e );
  return cons( e, args );
}

//
// Creates a new symbol. Name must be new
//
Enode * Egraph::newSymbol( const char * name, const char arity, const int type )
{
  if ( lookupSymbol( name ) != NULL )
  {
    cerr << "Error: symbol " << name << " already declared" << endl;
    exit( 1 );
  }    

  Enode * new_enode = new Enode( id_to_enode.size( )
                               , ENODE_TYPE_SYMBOL
			       , arity
			       , type
			       , name );
  insertSymbol( new_enode );	                        // Insert symbol enode
  assert( lookupSymbol( name ) == new_enode );          // Symbol must be there now
  return new_enode;// lookupSymbol( name );
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

Enode * Egraph::mkEq( Enode * args )               
{ 
  assert( args );
  assert( args->isList( ) );
  assert( args->getCar( ) );
  assert( args->getCdr( )->getCar( ) );

  Enode * x = args->getCar( );
  Enode * y = args->getCdr( )->getCar( );

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

Enode * Egraph::mkNot( Enode * args )
{ 
  assert( args );
  assert( args->isList( ) );
  assert( args->getCar( ) );
  Enode * arg = args->getCar( );
  assert( arg->isTerm( ) );
  // not not p --> p
  if ( arg->isNot( ) )
    return arg->getCdr( )->getCar( );

  // not false --> true
  if ( arg->isFalse( ) )
    return mkTrue( );

  // not true --> false
  if ( arg->isTrue( ) )
    return mkFalse( );

  return cons( id_to_enode[ ENODE_ID_NOT ], args ); 
}

// TODO: add simplifications
Enode * Egraph::mkAnd( Enode * args )   
{ 
  assert( args );
  assert( args->isList( ) );
  return cons( id_to_enode[ ENODE_ID_AND ], args ); 
}

// TODO: add simplifications
Enode * Egraph::mkOr( Enode * args )   
{ 
  assert( args );
  assert( args->isList( ) );
  return cons( id_to_enode[ ENODE_ID_OR ], args ); 
}

// TODO: add simplifications
Enode * Egraph::mkIff( Enode * args ) 
{ 
  assert( args );
  return cons( id_to_enode[ ENODE_ID_IFF ], args ); 
}

// TODO: add simplifications
Enode * Egraph::mkIfthenelse( Enode * i, Enode * t, Enode * e )  
{ 
  assert( i );
  assert( t );
  assert( e );
  return cons( id_to_enode[ ENODE_ID_IFTHENELSE ], cons( i, cons( t, cons( e ) ) ) ); 
}

// TODO: add simplifications
Enode * Egraph::mkIte( Enode * i, Enode * t, Enode * e ) 
{ 
  assert( i );
  assert( t );
  assert( e );
  return cons( id_to_enode[ ENODE_ID_ITE ], cons( i, cons( t, cons( e ) ) ) ); 
}

// TODO: add simplifications
Enode * Egraph::mkXor( Enode * args ) 
{ 
  assert( args );
  return cons( id_to_enode[ ENODE_ID_XOR ], args ); 
}

// TODO: add simplifications
Enode * Egraph::mkImplies( Enode * args ) 
{ 
  assert( args );
  return cons( id_to_enode[ ENODE_ID_IMPLIES ], args ); 
}

Enode * Egraph::mkDistinct( Enode * args )
{
  assert( args );
  Enode * res = cons( id_to_enode[ ENODE_ID_DISTINCT ], args );
  // The thing is that this distinction might have been
  // already processed. So if the index is -1 we are sure
  // it is new
  if ( res->getDistIndex( ) == -1 )
  {
    size_t index = index_to_dist.size( );
    if ( index > sizeof( dist_t ) * 8 )
    {
      cerr << "Error: max number of distinctions supported is " << sizeof( dist_t ) * 8 << endl;
      exit( 1 );
    }
    res->setDistIndex( index );
    // Store the correspondence
    index_to_dist.push_back( res );
    // Check invariant
    assert( index_to_dist[ index ] == res );
  }
  
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
    {
      cerr << "Error: no formula statement defined" << endl;
      exit( 1 );
    }

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

//=================================================================================================
// Debugging Routines

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

void Egraph::printStatistics ( ostream & os )
{
  os << "#" << endl
     << "# Number of Enodes.........: " << id_to_enode.size( ) << endl;

  unsigned size = 0;
  for ( unsigned i = 0 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];
    size += e->sizeInMem( );
  }

  os << "# Number of Symbol Enodes..: " << name_to_symbol.size( ) << endl;
  os << "# Number of Number Enodes..: " << name_to_number.size( ) << endl;
  os << "# Number of Define Enodes..: " << name_to_define.size( ) << endl;

  os << "# Egraph size..............: " << size / 1024.0 / 1024.0 << " MB" << endl
     << "#" << endl;
}

#ifdef PEDANTIC_DEBUG
//
// Check invariants on sig_tab
//
bool Egraph::checkUFInvariants ( )
{
  // 
  // First check:
  // For each enode, if it is a congruence root, then it
  // must be present in sig_tab
  //
  for ( unsigned i = ENODE_ID_LAST + 1 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];
    
    // Symbol nodes should not stay in sig_tab
    if ( !e->isTerm( ) && !e->isList( ) )
      continue;
    if ( !e->hasCongData( ) )
      continue;
    // Check that node is there if it is a congruence root
    if ( e->getCgPtr( ) == e )
      lookupSigTab( e );
  }
  //
  // Second check:
  // For each element of sig_tab check that the pointed node has
  // the corresponding signature, and check that it is a congruence root
  //
  for ( Map( Pair( int ), Enode * )::iterator it = sig_tab_list.begin( ) ;
        it != sig_tab_list.end( ) ;
	it ++ )
  {
    pair< int, int > sig = it->first;
    Enode * p = it->second;

    if ( p->getSig( ) != sig )
    {
      cerr << "Error: wrong signature for " << p << endl;
      return false; 
    }

    if ( p->getCgPtr( ) != p )
    {
      cerr << "Error: signature contains a node " << p << " that is not a congruence root" << endl;
      return false;
    }
  }
  for ( Map( Pair( int ), Enode * )::iterator it = sig_tab_term.begin( ) ;
        it != sig_tab_term.end( ) ;
	it ++ )
  {
    pair< int, int > sig = it->first;
    Enode * p = it->second;

    if ( p->getSig( ) != sig )
    {
      cerr << "Error: wrong signature for " << p << endl;
      return false; 
    }

    if ( p->getCgPtr( ) != p )
    {
      cerr << "Error: signature contains a node that is not a congruence root" << endl;
      return false;
    }
  }
  //
  // Third check:
  // For each enode check that "size" is precisely the
  // size of the equivalence class of the node reachable
  // with pointer "next", and the same for "parent_size"
  // and "parent". For "next" also check that the roots
  // all point to the same node
  //
  for ( unsigned i = 0 ; i < id_to_enode.size( ) ; i ++ )
  {
    Enode * e = id_to_enode[ i ];

    if ( !e->isList( ) && !e->isTerm( ) )
      continue;
    if ( !e->hasCongData( ) )
      continue;

    // Check "size" consistency and "root" consistency
    Enode * v = e;
    const Enode * vstart = v;
    const Enode * root = v->getRoot( );
    int counter = 0;
    for (;;)
    {
      counter ++;
      v = v->getNext( );
      if ( v->getRoot( ) != root )
      {
	cerr << "Error: equivalence class with different roots for " << e << endl;
	return false;
      }
      if ( v == vstart )
	break;
    }

    if ( e == e->getRoot( ) && counter != e->getSize( ) )
    {
      cerr << "Error: wrong equivalence class size for " << e << endl;
      return false;
    }

    if ( e->getParent( ) == NULL )
      continue;

    // Check "parent_size" consistency
    Enode * p = e->getParent( );
    const Enode * pstart = p;
    const bool scdr = ( e->isList( ) );
    counter = 0;
    for (;;)
    {
      counter ++;
      p = scdr ? p->getSameCdr( ) : p->getSameCar( ) ;
      if ( p == pstart )
	break;
    }

    if ( e == e->getRoot( ) && counter != e->getParentSize( ) )
    {
      cerr << "Error: wrong parent size for " << e << endl;
      return false;
    }
  }

  return true;
}
#endif

