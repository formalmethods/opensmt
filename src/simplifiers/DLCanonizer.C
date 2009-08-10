/*********************************************************************
 Author: Roberto Bruttomesso <roberto.bruttomesso@gmail.com>
       , Aliaksei Tsitovich  <aliaksei.tsitovich@usi.ch>
       , Edgar Pek           <edgar.pek@usi.ch>

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

#include "DLCanonizer.h"

Enode * DLCanonizer::canonize(Enode * formula)
{
  assert( formula );

  egraph.initDupMap( );

  vector< Enode *> unprocessed_enodes;     // stack for unprocessed enodes
  unprocessed_enodes.push_back( formula ); // formula needs to be processed
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while ( !unprocessed_enodes.empty() )
  {
    Enode * enode = unprocessed_enodes.back();
    //
    // Skip if the node has already been processed before
    //
    if ( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back();
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
      // Push only children that haven't been processed yet,
      // but never children of a theory atom
      //
      if ( !enode->isTAtom( ) && egraph.valDupMap( arg ) == NULL )
      {
	unprocessed_enodes.push_back( arg );
	unprocessed_children = true;
      }
    }
    //
    // SKip if unprocessed_children
    //
    if (unprocessed_children)
      continue;

    unprocessed_enodes.pop_back( );
    Enode * result = NULL;

    if ( enode->isTAtom( ) )
    {
      result = canonize_atom(enode);
    }
    else
    {
      assert( enode->isTerm( ) );
      assert( enode->isDTypeBool( ) );
      result = egraph.copyEnodeEtypeTermWithCache( enode );
    }

    assert( egraph.valDupMap( enode ) == NULL );
    assert( result );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );
  assert( new_formula );
  egraph.doneDupMap( );

  return new_formula;
}

Enode * DLCanonizer::rescale( Enode * formula )
{
#if USE_GMP
  assert( formula );

  int nof_variables = 0;
  mpz_t lcm;
  mpz_init_set_ui( lcm, 1 );

  egraph.initDup1( );

  vector< Enode *> unprocessed_enodes;     // stack for unprocessed enodes
  unprocessed_enodes.push_back( formula ); // formula needs to be processed
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while ( !unprocessed_enodes.empty() )
  {
    Enode * enode = unprocessed_enodes.back();
    //
    // Skip if the node has already been processed before
    //
    if ( egraph.isDup1( enode ) )
    {
      unprocessed_enodes.pop_back();
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
      // Push only children that haven't been processed yet,
      // but never children of a theory atom
      //
      if ( !egraph.isDup1( arg ) )
      {
	unprocessed_enodes.push_back( arg );
	unprocessed_children = true;
      }
    }
    //
    // SKip if unprocessed_children
    //
    if (unprocessed_children)
      continue;

    unprocessed_enodes.pop_back( );

    if ( !enode->isDTypeBool( ) )
    {
      if ( enode->isVar( ) )
	nof_variables ++;
      else if ( enode->isConstant( ) )
      {
	const Real & c = enode->getCar( )->getValue( );
	mpz_lcm( lcm, lcm, c.get_den( ).get_mpz_t( ) );
      }
    }

    assert( !egraph.isDup1( enode ) );
    egraph.storeDup1( enode );
  }

  egraph.doneDup1( );

  Real rescale_factor = Real( mpz_class( lcm )) * Real( nof_variables * nof_variables + 1 );
  assert( rescale_factor > 1 );

  curr_constant_sum = 0;
  const bool gmpnotset = !egraph.getUseGmp( );
  egraph.initDupMap( );
  unprocessed_enodes.push_back( formula );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while ( !unprocessed_enodes.empty() )
  {
    Enode * enode = unprocessed_enodes.back();
    //
    // Skip if the node has already been processed before
    //
    if ( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back();
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
      // Push only children that haven't been processed yet,
      // but never children of a theory atom
      //
      if ( egraph.valDupMap( arg ) == NULL )
      {
	unprocessed_enodes.push_back( arg );
	unprocessed_children = true;
      }
    }
    //
    // SKip if unprocessed_children
    //
    if (unprocessed_children)
      continue;

    unprocessed_enodes.pop_back( );
    Enode * result = NULL;

    if ( !enode->isDTypeBool( ) && enode->isConstant( ) )
    {
      Real c = enode->getCar( )->getValue( );
      Real r = c * rescale_factor;

      if ( gmpnotset )
      {
	if ( r < 0 )
	  curr_constant_sum += -r + 1;
	else
	  curr_constant_sum +=  r + 1;

	if ( curr_constant_sum > Real( LONG_MAX / 2 ) && !egraph.getUseGmp( ) )
	  egraph.setUseGmp( );
      }

#if RESCALE_IN_DL
      result = enode;
#else
      result = egraph.mkNum( r );
#endif
    }
    else
    {
#if RESCALE_IN_DL
      result = enode;
#else
      result = egraph.copyEnodeEtypeTermWithCache( enode );
#endif
    }

    assert( egraph.valDupMap( enode ) == NULL );
    assert( result );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );
  assert( new_formula );
  egraph.doneDupMap( );

#if RESCALE_IN_DL
  egraph.setRescale( rescale_factor );
#endif

  return new_formula;
#else
  error( "cannot rescale without GMP", "" );
  return NULL;
#endif
}

//
// Performs reccursive canonization of an atom, results are returned in vars;
//
Enode * DLCanonizer::canonize_atom( Enode * e )
{
  assert( e->isTAtom( ) );
  assert( e->isEq( ) || e->isLeq( ) );
  Enode * result = NULL;
  Enode * lhs = e->get1st( );
  Enode * rhs = e->get2nd( );

  map_int2Real lhs_vars, rhs_vars;
  canonize_atom_rec( lhs, lhs_vars );
  canonize_atom_rec( rhs, rhs_vars );

  multiply_vars( rhs_vars, -1 );
  merge_vars( lhs_vars, rhs_vars );
  //
  // Gets the free member of an equation
  //
  Real free_member = 0;
  if ( lhs_vars.find( -1 ) != lhs_vars.end( ) )
  {
#if FAST_RATIONALS
    free_member = -lhs_vars[-1];
#else
    free_member = -1 * lhs_vars[-1];
#endif
    lhs_vars.erase( -1 );
  }

  if ( free_member < 0 )
    curr_constant_sum += -free_member + 1;
  else
    curr_constant_sum +=  free_member + 1;

  if ( curr_constant_sum > Real( LONG_MAX / 2 ) && !egraph.getUseGmp( ) )
    egraph.setUseGmp( );
  //
  // Number of variables must be 1 or 2
  //
  vector< int > garbage;
  for ( map_int2Real::iterator it = lhs_vars.begin( ) ; it != lhs_vars.end( ) ; it ++ )
    if ( it->second == 0 )
      garbage.push_back( it->first );
  while ( !garbage.empty( ) )
  {
    lhs_vars.erase( garbage.back( ) );
    garbage.pop_back( );
  }

  if ( lhs_vars.size( ) == 0 )
  {
    if ( e->isEq( ) )
    {
      if ( free_member == 0 )
	return egraph.mkTrue( );
      else
	return egraph.mkFalse( );
    }
    else
    {
      if ( free_member >= 0 )
	return egraph.mkTrue( );
      else
	return egraph.mkFalse( );
    }
  }
  //
  // Error in case of more than 2 variables
  //
  if ( lhs_vars.size( ) > 2 )
    error( "not a DL atom: ", e );
  //
  // If only one variable return x ~ c
  //
  map_int2Real::iterator it = lhs_vars.begin( );
  if ( lhs_vars.size( ) == 1 )
  {
    if( e->isLeq( ) )
    {
      result = e;
    }
    //
    // Otherwise it is an equality x = c
    // Compute x<=c && x>=c   , i.e.,
    //	       x<=c && !(x<=c-1), i.e.,
    //       !( !(x<=c) || (x<=c-1) )
    //
    else
    {
      Enode * x = vars_hash[ it->first ];
      Enode * num_1 = egraph.mkNum( free_member );
      Enode * leq_1 = egraph.mkLeq( egraph.cons( x, egraph.cons( num_1 ) ) );

      free_member = free_member - 1;
      Enode * num_2 = egraph.mkNum( free_member );
      Enode * leq_2 = egraph.mkLeq( egraph.cons( x, egraph.cons( num_2 ) ) );

      Enode * or1 = egraph.mkNot( egraph.cons( leq_1 ) );
      Enode * or2 = leq_2;
      result = egraph.mkNot( egraph.cons( egraph.mkOr( egraph.cons( or1, egraph.cons( or2 ) ) ) ) );
    }
  }
  //
  // If two variables return x - y ~ c
  //
  if ( lhs_vars.size( ) == 2 )
  {
    Enode * x = vars_hash[ it->first ];
    const Real & x_coeff = it->second;
    it ++;
    Enode * y = vars_hash[ it->first ];
    const Real & y_coeff = it->second;
    assert( x_coeff == 1 || x_coeff == -1 );
    assert( y_coeff == 1 || y_coeff == -1 );
    assert( x_coeff + y_coeff == 0 );
    //
    // x is the variable with positive constant,
    // swap otherwise
    //
    if ( x_coeff == -1 )
    {
      Enode * tmp = x;
      x = y;
      y = tmp;
    }
    // UNSOUND FOR RDL !!
#if 0
    bool reversed = false;
    //
    // x is the variable with lower id, swap
    // otherwise, and also invert constant
    //
    if ( x->getId( ) > y->getId( ) )
    {
      Enode * tmp = x;
      x = y;
      y = tmp;
#if FAST_RATIONALS
    free_member = -free_member;
#else
    free_member = -1 * free_member;
#endif
      // If this is a <=, now it became a >=
      reversed = true;
    }
    //
    // Transform x-y >= c into !(x-y <= c-1)
    //
    if ( e->isLeq( ) && reversed )
    {
      free_member = free_member - 1;
    }
#endif
    //
    // Compute result for leq
    //
    Enode * minus = egraph.mkMinus( egraph.cons( x, egraph.cons( y ) ) );
    if ( e->isLeq( ) )
    {
      Enode * num = egraph.mkNum( free_member );
      Enode * leq = egraph.mkLeq( egraph.cons( minus, egraph.cons( num ) ) );
#if 0
      result = reversed ? egraph.mkNot( egraph.cons( leq ) ) : leq;
#else
      result = leq;
#endif
    }
    //
    // Otherwise it is an equality x - y = c
    // Compute x-y<=c && (x-y>=c)   , i.e.,
    //	       x-y<=c && !(x-y<=c-1), i.e.,
    //         !( !(x-y<=c) || (x-y<=c-1) )
    //
    else if ( config.logic == QF_IDL )
    {
      Enode * num_1 = egraph.mkNum( free_member );
      Enode * leq_1 = egraph.mkLeq( egraph.cons( minus, egraph.cons( num_1 ) ) );

      free_member = free_member - 1;
      Enode * num_2 = egraph.mkNum( free_member );
      Enode * leq_2 = egraph.mkLeq( egraph.cons( minus, egraph.cons( num_2 ) ) );

      Enode * or1 = egraph.mkNot( egraph.cons( leq_1 ) );
      Enode * or2 = leq_2;
      result = egraph.mkNot( egraph.cons( egraph.mkOr( egraph.cons( or1, egraph.cons( or2 ) ) ) ) );
    }
    //
    // Simply rewrite as
    //
    // x-y<=c && y-x<=-c
    //
    else if ( config.logic == QF_RDL )
    {
      Enode * num_1 = egraph.mkNum( free_member );
      free_member = -free_member;
      Enode * num_2 = egraph.mkNum( free_member );
      Enode * leq_1 = egraph.mkLeq( egraph.cons( minus, egraph.cons( num_1 ) ) );
      Enode * x = minus->get1st( );
      Enode * y = minus->get2nd( );
      Enode * minus_2 = egraph.mkMinus( egraph.cons( y, egraph.cons( x ) ) );
      Enode * leq_2 = egraph.mkLeq( egraph.cons( minus_2, egraph.cons( num_2 ) ) );
      result = egraph.mkAnd( egraph.cons( leq_1, egraph.cons( leq_2 ) ) );
    }
  }

  assert( result );
  return result;
}

//
// Performs reccursive canonization of an atom, results are returned in vars;
//
void DLCanonizer::canonize_atom_rec(Enode * atom, map_int2Real & vars)
{
  // Check all accepting types of atoms
  if (atom->isPlus())
  {
    // canonize all operands and merge the resulting vars maps
    for (Enode * arg_list = atom->getCdr() ; arg_list != egraph.enil; arg_list
        = arg_list->getCdr() )
    {
      map_int2Real tmp;
      canonize_atom_rec(arg_list->getCar(), tmp);
      merge_vars(vars, tmp);
    }
  }
  else if (atom->isMinus())
  {
    assert( atom->getArity( ) == 2 );

    Enode * arg_list= atom->getCdr( );
    // canonize first operand
    assert( !arg_list->isEnil( ) );
    canonize_atom_rec(arg_list->getCar(), vars);

    // canonize all negated operands
    for (arg_list = arg_list->getCdr( ) ; arg_list != egraph.enil; arg_list = arg_list->getCdr() )
    {
      map_int2Real tmp;
      canonize_atom_rec(arg_list->getCar(), tmp);
      multiply_vars(tmp, -1);
      merge_vars(vars, tmp);
    }
  }
  else if (atom->isTimes())
  {
    assert(atom->getCdr() );
    assert(atom->getCdr( )->getCdr() );
    assert(atom->getCdr( )->getCdr( )->getCdr( )->isEnil() );
    Enode * arg1 = atom->get1st();
    Enode * arg2 = atom->get2nd();

    // canonize both operands
    map_int2Real tmp1, tmp2;
    canonize_atom_rec(arg1, tmp1);
    canonize_atom_rec(arg2, tmp2);

    // if tmp1 is a constant
    if (tmp1.size()==1 && tmp1.find(-1)!=tmp1.end())
    {
      multiply_vars(tmp2, tmp1[-1]);
      merge_vars(vars, tmp2);
    }
    // if tmp2 is a constant
    else if (tmp2.size()==1 && tmp2.find(-1)!=tmp2.end())
    {
      multiply_vars(tmp1, tmp2[-1]);
      merge_vars(vars, tmp1);
    }
    // None of operands is constant -> non-linear
    else
    {
      error( "atom is non-linear ", atom );
    }
  }
  else if (atom->isConstant())
  {
    // Read numerical value
    vars[-1]= atom->getCar( )->getValue();
  }
  else if (atom->isUminus())
  {
    map_int2Real tmp;
    canonize_atom_rec( atom->get1st( ), tmp );
    multiply_vars( tmp, -1 );
    merge_vars( vars, tmp );
  }
  else if (atom->isVar())
  {
    // Read the variable
    vars[atom->getId( )]=1;
    vars_hash[atom->getId( )] = atom;
  }
  else if (atom->isDiv())
  {
      // Read the (/ a b) clause
    assert(atom->get1st( )->isConstant() || atom->get1st( )->isUminus());
    assert(atom->get2nd( )->isConstant());
    if (atom->get1st( )->isUminus())
    {
#if FAST_RATIONALS
      vars[-1] = -atom->get1st( )->get1st()->getCar( )->getValue();
#else
      vars[-1] = -1 * atom->get1st( )->get1st()->getCar( )->getValue();
#endif
    }
    else
      vars[-1] = atom->get1st( )->getCar( )->getValue();

    vars[-1] /= atom->get2nd( )->getCar( )->getValue();
  }
  else
  {
    error( "something wrong with ", atom );
  }
}

//
// Multiply all values in map_int2Real by real
//
void DLCanonizer::multiply_vars(map_int2Real & vars, const Real value)
{
  for (map_int2Real::iterator it = vars.begin(); it!=vars.end(); it++)
    it->second = it->second * value;
}

//
// Merge map_int2Real dst and src into dst
//
void DLCanonizer::merge_vars(map_int2Real & dst, map_int2Real & src)
{
  // Values of intersecting entries are summarized
  for (map_int2Real::iterator it = src.begin(); it!=src.end(); it++)
    if (dst.find(it->first)!=dst.end())
      dst[it->first]+=it->second;
    else
      dst[it->first]=it->second;
}
