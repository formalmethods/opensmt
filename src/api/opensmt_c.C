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

#include "opensmt_c.h"
#include "Egraph.h"
#include "Tseitin.h"
#include "SimpSMTSolver.h"

#ifndef SMTCOMP

class OpenSMTContext
{
public:

  OpenSMTContext( )
    : config_p  ( new SMTConfig( NULL ) )
    , config    ( *config_p )
    , egraph_p  ( new Egraph( config ) )
    , egraph    ( *egraph_p )
    , solver_p  ( new SimpSMTSolver( egraph, config ) )
    , solver    ( *solver_p )
    , cnfizer_p ( new Tseitin( egraph, solver, config ) )
    , cnfizer   ( *cnfizer_p )
    , counter   ( 0 )
  { }

  ~OpenSMTContext( )
  {
    assert( config_p );
    assert( egraph_p );
    assert( solver_p );
    assert( cnfizer_p );
    delete cnfizer_p;
    delete solver_p;
    delete egraph_p;
    delete config_p;
  }

  void setLogic( logic_t l ) { config.logic = l; }

  SMTConfig *        config_p;
  SMTConfig &        config;
  Egraph *           egraph_p;
  Egraph &           egraph;
  SimpSMTSolver *    solver_p;
  SimpSMTSolver &    solver;
  Tseitin *          cnfizer_p;
  Tseitin &          cnfizer;

  int                counter;
};

//
// Communication APIs
//
void opensmt_set_verbosity( opensmt_context c, int n )
{ 
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  context.config.gconfig.print_stats = n;
}

char * opensmt_version( )
{
  return const_cast< char * >( PACKAGE_STRING );
}

opensmt_context opensmt_mk_context( opensmt_logic l )
{
  OpenSMTContext * c = new OpenSMTContext( );
  OpenSMTContext & context = *c;
  // Set the right logic
  switch( l )
  {
    case qf_uf:    context.setLogic( QF_UF );    break;
    case qf_bv:    context.setLogic( QF_BV );    break;
    case qf_rdl:   context.setLogic( QF_RDL );   break;
    case qf_idl:   context.setLogic( QF_IDL );   break;
    case qf_lra:   context.setLogic( QF_LRA );   break;
    case qf_ufidl: context.setLogic( QF_UFIDL ); break;
    case qf_uflra: context.setLogic( QF_UFLRA ); break;
  }
  // Tell that we need incremental solver
  context.config.incremental = true;
  // Initialize theory solvers
  context.egraph.initializeTheorySolvers( );
  // Return context
  return static_cast< void * >( c );
}

void opensmt_del_context( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  delete c_;
}

void opensmt_reset( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  context.solver.reset( );
}

void opensmt_print_expr( opensmt_expr e )
{
  Enode * enode = static_cast< Enode * >( e );
  cerr << enode;
}

void opensmt_push( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  context.solver.pushBacktrackPoint( );
}

void opensmt_pop( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  context.solver.popBacktrackPoint( );
}

void opensmt_assert( opensmt_context c, opensmt_expr e )
{
  assert( c );
  assert( e );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * expr = static_cast< Enode * >( e );
  context.egraph.addAssumption( expr );
}

opensmt_result opensmt_check( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * formula = context.egraph.getFormula( );
  lbool result = context.cnfizer.cnfizeAndGiveToSolver( formula );
  if ( result == l_Undef )
    result = context.solver.solve( );
  if ( result == l_Undef ) return l_undef;
  if ( result == l_False ) return l_false;
  return l_true;
}
//
// Model APIs
//
void opensmt_display_model( opensmt_context c )
{ 
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  context.solver.printModel( cerr );
  context.egraph.printModel( cerr );
}

void opensmt_display_model_to_file( opensmt_context c, const char * filename )
{ 
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  ofstream os( filename );
  context.solver.printModel( os );
  context.egraph.printModel( os );
}
//
// Formula construction APIs
//
opensmt_expr opensmt_mk_true( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * res = context.egraph.mkTrue( );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_false( opensmt_context c )
{
  assert( c );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * res = context.egraph.mkFalse( );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bool_var( opensmt_context c, char * s )
{
  assert( c );
  assert( s );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  vector< unsigned > tmp;
  tmp.push_back( DTYPE_BOOL );
  context.egraph.newSymbol( s, tmp );
  Enode * res = context.egraph.mkVar( s, true );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_int_var( opensmt_context c, char * s )
{
  assert( c );
  assert( s );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  vector< unsigned > tmp;
  tmp.push_back( DTYPE_INT );
  context.egraph.newSymbol( s, tmp );
  Enode * res = context.egraph.mkVar( s, true );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_real_var( opensmt_context c, char * s )
{
  assert( c );
  assert( s );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  vector< unsigned > tmp;
  tmp.push_back( DTYPE_REAL );
  context.egraph.newSymbol( s, tmp );
  Enode * res = context.egraph.mkVar( s, true );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_or( opensmt_context c, opensmt_expr * expr_list, unsigned n )
{
  assert( c );
  assert( expr_list );
  list< Enode * > args;
  for ( unsigned i = 0 ; i < n ; i ++ )
  {
    Enode * arg = static_cast< Enode * >( expr_list[ i ] );
    args.push_back( arg );
  }
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( args );
  Enode * res = context.egraph.mkOr( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_and( opensmt_context c, opensmt_expr * expr_list, unsigned n )
{
  assert( c );
  assert( expr_list );
  list< Enode * > args;
  for ( unsigned i = 0 ; i < n ; i ++ )
  {
    Enode * arg = static_cast< Enode * >( expr_list[ i ] );
    args.push_back( arg );
  }
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( args );
  Enode * res = context.egraph.mkAnd( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_eq ( opensmt_context c, opensmt_expr x, opensmt_expr y )
{
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkEq( args_list );
  return static_cast< void * >( res );
 }

opensmt_expr opensmt_mk_ite( opensmt_context c, opensmt_expr i, opensmt_expr t, opensmt_expr e ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  char def_name[ 32 ];
  sprintf( def_name, ITE_STR, context.counter++ );
  Enode * i_ = static_cast< Enode * >( i );
  Enode * t_ = static_cast< Enode * >( t );
  Enode * e_ = static_cast< Enode * >( e );
  const unsigned type = t_->getDType( );
  const int width = t_->getWidth( );
  assert( type == e_->getDType( ) );
  assert( width == e_->getWidth( ) );
  context.egraph.newSymbol( def_name, type | width );  
  Enode * v = context.egraph.mkVar( def_name );
  Enode * res;
  if ( i_->isTrue( )  ) res = t_;
  else if ( i_->isFalse( ) ) res = e_;
  else if ( t_ == e_ ) res = t_;
  else
  {
    Egraph & eg = context.egraph;
    Enode * eq1 = eg.mkEq( eg.cons( v, eg.cons( t_ ) ) );
    Enode * eq2 = eg.mkEq( eg.cons( v, eg.cons( e_ ) ) );
    Enode * imp1 = eg.mkImplies( eg.cons( i_, eg.cons( eq1 ) ) );
    Enode * imp2 = eg.mkImplies( eg.cons( eg.mkNot( eg.cons( i_ ) ), eg.cons( eq2 ) ) );
    res = eg.mkAnd( eg.cons( imp1, eg.cons( imp2 ) ) );
  }
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_not( opensmt_context c, opensmt_expr x)
{
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x ));
  Enode * res = context.egraph.mkNot( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_num_from_string( opensmt_context c, const char * s )
{
  assert( c );
  assert( s );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * res = context.egraph.mkNum( s );
  return res;
}

opensmt_expr opensmt_mk_num_from_mpz( opensmt_context c, const mpz_t n ) 
{ 
  assert( c );
  assert( n );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  mpz_class n_( n );
  Real num( n_ );
  Enode * res = context.egraph.mkNum( num );
  return res;
}

opensmt_expr opensmt_mk_num_from_mpq( opensmt_context c, const mpq_t n ) 
{ 
  assert( c );
  assert( n );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  mpq_class n_( n );
  Real num( n_ );
  Enode * res = context.egraph.mkNum( num );
  return res;
}

opensmt_expr opensmt_mk_plus( opensmt_context c, opensmt_expr * expr_list, unsigned n )
{
  list< Enode * > args;
  for ( unsigned i = 0 ; i < n ; i ++ )
  {
    Enode * arg = static_cast< Enode * >( expr_list[ i ] );
    args.push_back( arg );
  }
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( args );
  Enode * res = context.egraph.mkPlus( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_minus( opensmt_context c, opensmt_expr x, opensmt_expr y )
{
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkMinus( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_times( opensmt_context c, opensmt_expr * expr_list, unsigned n )
{
  list< Enode * > args;
  for ( unsigned i = 0 ; i < n ; i ++ )
  {
    Enode * arg = static_cast< Enode * >( expr_list[ i ] );
    args.push_back( arg );
  }
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( args );
  Enode * res = context.egraph.mkTimes( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_leq( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs )
{
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkLeq( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_lt( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkLt( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_gt( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkGt( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_geq( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkGeq( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_constant( opensmt_context c, unsigned w, unsigned long n ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  char buf[ 1024 ];
  sprintf( buf, "bv%ld[%d]", n, w );
  Enode * res = context.egraph.mkBvnum( buf );
  return res;
}

opensmt_expr opensmt_mk_bv_constant_from_string( opensmt_context c, unsigned w, const char * s ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  char buf[ 1024 ];
  sprintf( buf, "bv%s[%d]", s, w );
  Enode * res = context.egraph.mkBvnum( buf );
  return res;
}

opensmt_expr opensmt_mk_bv_add( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvadd( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_sub( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvsub( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_mul( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvmul( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_neg( opensmt_context c, opensmt_expr x ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x ) );
  Enode * res = context.egraph.mkBvneg( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_concat( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkConcat( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_and( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvand( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_or( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvor( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_xor( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvxor( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_not( opensmt_context c, opensmt_expr x ) 
{ 
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x ) );
  Enode * res = context.egraph.mkBvnot( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_extract( opensmt_context c, unsigned msb, unsigned lsb, opensmt_expr x ) 
{ 
  assert( c );
  assert( x );
  assert( lsb <= msb );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * arg = static_cast< Enode * >( x );
  Enode * res = context.egraph.mkExtract( msb, lsb, arg );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_sign_extend( opensmt_context c, opensmt_expr x, unsigned n ) 
{ 
  assert( c );
  assert( x );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * arg = static_cast< Enode * >( x );
  Enode * res = context.egraph.mkSignExtend( n, arg );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_zero_extend( opensmt_context c, opensmt_expr x, unsigned n ) 
{ 
  assert( c );
  assert( x );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * arg = static_cast< Enode * >( x );
  Enode * res = context.egraph.mkZeroExtend( n, arg );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_shift_left( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  assert( c );
  assert( x );
  assert( y );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvshl( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_shift_right( opensmt_context c, opensmt_expr x, opensmt_expr y ) 
{ 
  assert( c );
  assert( x );
  assert( y );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( x )
                    , context.egraph.cons( static_cast< Enode * >( y ) ) );
  Enode * res = context.egraph.mkBvlshr( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_lt( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvult( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_le( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvule( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_gt( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvugt( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_ge( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvuge( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_slt( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs ) 
{ 
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvslt( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_sle( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs )
{
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvsle( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_sgt( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs )
{
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvsgt( args_list );
  return static_cast< void * >( res );
}

opensmt_expr opensmt_mk_bv_sge( opensmt_context c, opensmt_expr lhs, opensmt_expr rhs )
{
  assert( c );
  assert( lhs );
  assert( rhs );
  OpenSMTContext * c_ = static_cast< OpenSMTContext * >( c );
  OpenSMTContext & context = *c_;
  Enode * args_list = context.egraph.cons( static_cast< Enode * >( lhs )
                    , context.egraph.cons( static_cast< Enode * >( rhs ) ) );
  Enode * res = context.egraph.mkBvsge( args_list );
  return static_cast< void * >( res );
}

#endif
