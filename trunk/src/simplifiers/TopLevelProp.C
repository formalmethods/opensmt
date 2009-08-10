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

#include "TopLevelProp.h"

#define INLINE_CONSTANTS         0
#define SIMPLIFY_TWIN_EQUALITIES 1

Enode *
TopLevelProp::doit( Enode * formula )
{
  assert( formula );
  //
  // Learn some transitivity atoms for UF
  //
  if ( config.logic == QF_UF )
    formula = learnTransitivity( formula );
  //
  // Repeat until fix point
  //
  bool stop = false;
  while ( !stop )
  {
    //
    // Step 1: gather top-level facts (including predicates)
    //
    Map( enodeid_t, Enode * ) substitutions;
    Map( enodeid_t, bool ) top_level_atoms;
    retrieveSubstitutions( formula, substitutions, top_level_atoms );
    //
    // Step 2: produce a new formula with substitutions done (if any to use)
    //
    bool sub_stop = true;

    if ( !substitutions.empty( ) )
      formula = substitute( formula, substitutions, top_level_atoms, sub_stop );
    //
    // Step 3: Only for BV remove unconstrained terms
    //
    bool unc_stop = true;
#if SIMPLIFY_TWIN_EQUALITIES
    bool twin_stop = true;
#endif
    if ( config.logic == QF_BV )
    {
      formula = propagateUnconstrainedVariables( formula, unc_stop );
#if SIMPLIFY_TWIN_EQUALITIES
      formula = simplifyTwinEqualities( formula, twin_stop );
#endif
    }

    stop = sub_stop && unc_stop && twin_stop;
  }

  return formula;
}

void
TopLevelProp::retrieveSubstitutions( Enode * formula
                                   , Map( enodeid_t, Enode * ) & substitutions
				   , Map( enodeid_t, bool ) & top_level_atoms )
{
  vector< Enode * > unprocessed_enodes;
  vector< bool >    unprocessed_polarity;
  egraph.initDup1( );

  unprocessed_enodes  .push_back( formula );
  unprocessed_polarity.push_back( true );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    assert( unprocessed_enodes.size( ) == unprocessed_polarity.size( ) );
    Enode * enode = unprocessed_enodes.back( );
    const bool polarity = unprocessed_polarity.back( );
    unprocessed_enodes  .pop_back( );
    unprocessed_polarity.pop_back( );
    // 
    // Skip if the node has already been processed before
    //
    if ( egraph.isDup1( enode ) )
      continue;

    egraph.storeDup1( enode );
    //
    // Process children
    //
    if ( enode->isBooleanOperator( ) )
    {
      bool recurse = true;
      bool new_polarity;

      if ( enode->isAnd( ) && polarity )
	new_polarity = true;	  
      else if ( enode->isNot( ) )
	new_polarity = !polarity;
      else if ( enode->isOr( ) && !polarity )
	new_polarity = false;
      else
	recurse = false;

      if ( recurse )
      {
	Enode * arg_list;
	for ( arg_list = enode->getCdr( ) 
	    ; arg_list != egraph.enil 
	    ; arg_list = arg_list->getCdr( ) )
	{
	  Enode * arg = arg_list->getCar( );
	  assert( arg->isTerm( ) );
	  unprocessed_enodes  .push_back( arg );
	  unprocessed_polarity.push_back( new_polarity );
	}
      }
    }
    //
    // Add a new substitution for iff if possible
    //
    if ( enode->isIff( ) && polarity )
    {
      Enode * lhs = enode->get1st( );
      Enode * rhs = enode->get2nd( );

      if ( !lhs->isVar( ) && !rhs->isVar( ) )
	continue;

      Enode * var  = lhs->isVar( ) ? lhs : rhs;
      Enode * term = lhs->isVar( ) ? rhs : lhs;

      if ( contains( term, var ) )
	continue;

      // Substitute variable with term that does not contain it
      substitutions[ var->getId( ) ] = term;
    }
    //
    // Add a new substitution for = if possible
    //
    else if ( enode->isAtom( ) )
    {
      // Substitute boolean variable with true/false
      if ( enode->isVar( ) ) 
      {
	substitutions[ enode->getId( ) ] = polarity ? egraph.mkTrue( ) : egraph.mkFalse( );
	continue;
      }

      // Substitute positive equalities
      if ( !enode->isEq( ) || !polarity )
	continue;

      assert( enode->isEq( ) );
      assert( polarity );

      Enode * lhs = enode->get1st( );
      Enode * rhs = enode->get2nd( );

      if ( ( config.logic == QF_IDL 
	  || config.logic == QF_RDL )
	&& enode->isEq( ) )
      {
	Enode * norm = normalizeDLAtom( enode );
	//
	// It might be the case that norm is simplified to true/false
	//
	if ( norm->isTrue( ) || norm->isFalse( ) )
	{
	  substitutions[ enode->getId( ) ] = norm;
	  continue;
	}

	lhs = norm->get1st( );
	rhs = norm->get2nd( );
	//
	// Case x-y = c
	//
	if ( enode->isEq( ) 
	  && !lhs->isVar( ) 
	  && !rhs->isVar( ) )
	{
	  assert( lhs->isMinus( ) || rhs->isMinus( ) );
	  if ( lhs->isMinus( ) )
	  {
	    assert( rhs->isConstant( ) );
	    rhs = egraph.mkPlus( egraph.cons( lhs->get2nd( ), egraph.cons( rhs ) ) );
	    lhs = lhs->get1st( );
	  }	  
	  else if ( rhs->isMinus( ) )
	  {
	    assert( lhs->isConstant( ) );
	    lhs = egraph.mkPlus( egraph.cons( rhs->get2nd( ), egraph.cons( lhs ) ) );
	    rhs = rhs->get1st( );
	  }	  
	  assert( lhs->isVar( ) || rhs->isVar( ) );
	}

	if ( !lhs->isVar( ) && !rhs->isVar( ) )
	  continue;

	Enode * var  = lhs->isVar( ) ? lhs : rhs;
	Enode * term = lhs->isVar( ) ? rhs : lhs;

	if ( contains( term, var ) )
	  continue;

	// Substitute variable with term that does not contain it
	substitutions[ var->getId( ) ] = term;
      }
      else if ( enode->isEq( ) )
      {
#if INLINE_CONSTANTS
	if ( lhs->isConstant( ) )
	{
	  assert( !rhs->isConstant( ) );
	  substitutions[ rhs->getId( ) ] = lhs;
	  continue;
	}
	else if ( rhs->isConstant( ) )
	{
	  assert( !lhs->isConstant( ) );
	  substitutions[ lhs->getId( ) ] = rhs;
	  continue;
	}
#endif

	if ( !lhs->isVar( ) && !rhs->isVar( ) )
	  continue;

	Enode * var  = lhs->isVar( ) ? lhs : rhs;
	Enode * term = lhs->isVar( ) ? rhs : lhs;

	if ( contains( term, var ) )
	  continue;

	// Substitute variable with term that does not contain it
	substitutions[ var->getId( ) ] = term;
      }
    }
  }

  // Done with cache
  egraph.doneDup1( );
}

bool
TopLevelProp::contains( Enode * term, Enode * var )
{
  vector< Enode * > unprocessed_enodes;
  egraph.initDup2( );

  unprocessed_enodes.push_back( term );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    if ( enode == var ) 
    {
      egraph.doneDup2( );
      return true;
    }
    // 
    // Skip if the node has already been processed before
    //
    if ( egraph.isDup2( enode ) )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = enode->getCdr( ) ; 
	arg_list != egraph.enil ; 
	arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed
      //
      if ( !egraph.isDup2( arg ) )
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
    egraph.storeDup2( enode );
  }

  egraph.doneDup2( );

  return false;
}

Enode *
TopLevelProp::substitute( Enode * formula
                        , Map( enodeid_t, Enode * ) & substitutions
                        , Map( enodeid_t, bool ) & top_level_atoms
			, bool & sub_stop )
{
  assert( formula );
  list< Enode * > reinsert_back;

  vector< Enode * > unprocessed_enodes;
  egraph.initDupMap( );

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
    if ( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = enode->getCdr( ) 
	; arg_list != egraph.enil 
	; arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );

      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed
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
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      
    Enode * result = NULL;

#if INLINE_CONSTANTS
    error( "buggy -- should reinsert atoms that equate constants" );
    Map( enodeid_t, Enode * )::iterator it = substitutions.find( enode->getId( ) );
    // Substitute
    if ( it != substitutions.end( ) )
    {
      result = it->second;
    }
#else
    if ( enode->isVar( ) )
    {
      Map( enodeid_t, Enode * )::iterator it = substitutions.find( enode->getId( ) );
      // Substitute
      if ( it != substitutions.end( ) )
      {
	result = it->second;
      }
    }
    else if ( enode->isTAtom( ) )
    {
      Map( enodeid_t, Enode * )::iterator it = substitutions.find( enode->getId( ) );
      // Substitute
      if ( it != substitutions.end( ) )
      {
	result = it->second;
      }
    }
#endif

    if ( result == NULL )
      result = egraph.copyEnodeEtypeTermWithCache( enode );
    else
      sub_stop = false;

    if ( result->isTAtom( ) 
      && ( config.logic == QF_IDL || config.logic == QF_RDL ) )
      result = normalizeDLAtom( result );

    assert( result );
    assert( egraph.valDupMap( enode ) == NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );

  egraph.doneDupMap( );
  assert( new_formula );
  return new_formula;
}

//
// Performs reccursive canonization of an atom, results are returned in vars;
//
Enode * TopLevelProp::normalizeDLAtom( Enode * e )
{
  assert( e->isTAtom( ) );
  assert( e->isEq( ) || e->isLeq( ) );

  Enode * result = NULL;

  Enode * lhs = e->get1st( );
  Enode * rhs = e->get2nd( );
  const bool lhs_v_c = lhs->isVar( ) || lhs->isConstant( ) || ( lhs->isUminus( ) && lhs->get1st( )->isConstant( ) );
  const bool rhs_v_c = rhs->isVar( ) || rhs->isConstant( ) || ( rhs->isUminus( ) && rhs->get1st( )->isConstant( ) );

  //
  // Check some common shapes which we leave intact
  //
  // Case 1: x ~ y
  //
  if ( lhs_v_c && rhs_v_c )
  {
    result = e->isLeq( ) 
           ? egraph.mkLeq( e->getCdr( ) )
	   : egraph.mkEq ( e->getCdr( ) );
  }
  //
  // Case 2: x - y ~ c
  //
  else if ( ( lhs->isMinus( ) && lhs->get1st( )->isVar( ) && lhs->get2nd( )->isVar( ) && rhs_v_c )
         || ( rhs->isMinus( ) && rhs->get1st( )->isVar( ) && rhs->get2nd( )->isVar( ) && lhs_v_c ) )
  {
    result = e->isLeq( ) 
           ? egraph.mkLeq( e->getCdr( ) )
	   : egraph.mkEq ( e->getCdr( ) );
  }
  //
  // Otherwise we rewrite the term 
  // in the form of case 2
  //
  else
  {
    map_int2Real lhs_vars, rhs_vars;
    normalizeDLAtomRec( lhs, lhs_vars );
    normalizeDLAtomRec( rhs, rhs_vars );

    multiplyVars( rhs_vars, -1 );
    mergeVars( lhs_vars, rhs_vars );

    //
    // Gets the free member of an equation
    //
    Real free_member = 0;
    if ( lhs_vars.find( -1 ) != lhs_vars.end( ) )
    {
      free_member = -lhs_vars[-1];
      lhs_vars.erase( -1 );
    }
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

    if ( lhs_vars.size( ) > 2 )
      error( "not a DL atom: ", e );
    //
    // If only one variable return x ~ c
    //
    map_int2Real::iterator it = lhs_vars.begin( );
    if ( lhs_vars.size( ) == 1 )
    {
      Enode * x = vars_hash[ it->first ];
      const Real c_ = free_member / it->second;
      Enode * c = egraph.mkNum( c_ ); 
      Enode * list = egraph.cons( x, egraph.cons( c ) );
      result = e->isLeq( ) 
	     ? egraph.mkLeq( list )
	     : egraph.mkEq ( list );
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

      Enode * c = egraph.mkNum( free_member ); 
      Enode * minus = NULL;

      if ( x_coeff == 1 )
	minus = egraph.mkMinus( egraph.cons( x, egraph.cons( y ) ) ); 
      else
	minus = egraph.mkMinus( egraph.cons( y, egraph.cons( x ) ) ); 

      Enode * list = egraph.cons( minus, egraph.cons( c ) );

      result = e->isLeq( ) 
	     ? egraph.mkLeq( list )
	     : egraph.mkEq ( list );
    }
  }

  assert( result );
  return result;
}
//
// Performs reccursive canonization of an atom, results are returned in vars;
//
void TopLevelProp::normalizeDLAtomRec(Enode * atom, map_int2Real & vars)
{
  // Check all accepting types of atoms
  if (atom->isPlus())
  {
    // canonize all operands and merge the resulting vars maps
    for (Enode * arg_list = atom->getCdr() ; arg_list != egraph.enil; arg_list
        = arg_list->getCdr() )
    {
      map_int2Real tmp;
      normalizeDLAtomRec(arg_list->getCar(), tmp);
      mergeVars(vars, tmp);
    }
  }
  else if (atom->isMinus())
  {
    assert( atom->getArity( ) == 2 );

    Enode * arg_list= atom->getCdr( );
    // canonize first operand
    assert( !arg_list->isEnil( ) );
    normalizeDLAtomRec(arg_list->getCar(), vars);

    // canonize all negated operands
    for (arg_list = arg_list->getCdr( ) ; arg_list != egraph.enil; arg_list = arg_list->getCdr() )
    {
      map_int2Real tmp;
      normalizeDLAtomRec(arg_list->getCar(), tmp);
      multiplyVars(tmp, -1);
      mergeVars(vars, tmp);
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
    normalizeDLAtomRec(arg1, tmp1);
    normalizeDLAtomRec(arg2, tmp2);

    // if tmp1 is a constant
    if (tmp1.size()==1 && tmp1.find(-1)!=tmp1.end())
    {
      multiplyVars(tmp2, tmp1[-1]);
      mergeVars(vars, tmp2);
    }
    // if tmp2 is a constant
    else if (tmp2.size()==1 && tmp2.find(-1)!=tmp2.end())
    {
      multiplyVars(tmp1, tmp2[-1]);
      mergeVars(vars, tmp1);
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
    normalizeDLAtomRec( atom->get1st( ), tmp );
    multiplyVars( tmp, -1 );
    mergeVars( vars, tmp );
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
      vars[-1] = -atom->get1st( )->get1st()->getCar( )->getValue();
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
void TopLevelProp::multiplyVars(map_int2Real & vars, const Real value)
{
  for (map_int2Real::iterator it = vars.begin(); it!=vars.end(); it++)
    it->second = it->second * value;
}
//
// Merge map_int2Real dst and src into dst
//
void TopLevelProp::mergeVars(map_int2Real & dst, map_int2Real & src)
{
  // Values of intersecting entries are summarized
  for (map_int2Real::iterator it = src.begin(); it!=src.end(); it++)
    if (dst.find(it->first)!=dst.end())
      dst[it->first]+=it->second;
    else
      dst[it->first]=it->second;
}

Enode * 
TopLevelProp::learnTransitivity( Enode * formula )
{
  list< Enode * > implications;
  vector< Enode * > unprocessed_enodes;
  egraph.initDupMap( );

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
    if ( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = enode->getCdr( ) ; 
	arg_list != egraph.enil ; 
	arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed
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
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      

    Enode * result = NULL;
    //
    // Add (or (and (= x w) (= w z)) (and (= x y) (= y z))) -> (= x z)
    //
    const bool condition1 = enode->isOr( ) && enode->getArity( ) == 2
                         && enode->get1st( )->isAnd( ) && enode->get1st( )->get1st( )->isEq( ) && enode->get1st( )->get2nd( )->isEq( )
                         && enode->get2nd( )->isAnd( ) && enode->get2nd( )->get1st( )->isEq( ) && enode->get2nd( )->get2nd( )->isEq( );

    if ( condition1 )
    {
      //
      // (= v1 v2) (= v3 v4)
      //
      Enode * v1 = enode->get1st( )->get1st( )->get1st( );
      Enode * v2 = enode->get1st( )->get1st( )->get2nd( );
      Enode * v3 = enode->get1st( )->get2nd( )->get1st( );
      Enode * v4 = enode->get1st( )->get2nd( )->get2nd( );
      //
      // (= t1 t2) (= t3 t4)
      //
      Enode * t1 = enode->get2nd( )->get1st( )->get1st( );
      Enode * t2 = enode->get2nd( )->get1st( )->get2nd( );
      Enode * t3 = enode->get2nd( )->get2nd( )->get1st( );
      Enode * t4 = enode->get2nd( )->get2nd( )->get2nd( );
      //
      // Detect bridging variables
      //
      const bool condition2a = v1 == v3 || v1 == v4 || v2 == v3 || v2 == v4;
      const bool condition2b = t1 == t3 || t1 == t4 || t2 == t3 || t2 == t4;

      if ( condition2a && condition2b )
      {
	Enode * w  = (v1 == v3 || v1 == v4 ? v1 : v2);
	Enode * x1 = (v1 == w ? v2 : v1);
	Enode * z1 = (v3 == w ? v4 : v3);

	Enode * y  = (t1 == t3 || t1 == t4 ? t1 : t2);
	Enode * x2 = (t1 == y ? t2 : t1);
	Enode * z2 = (t3 == y ? t4 : t3);

	const bool condition2 = (x1 == x2 && z1 == z2) 
	                     || (x1 == z2 && x2 == z1);

	if ( condition2 )
	{
	  Enode * impl = egraph.mkEq( egraph.cons( x1, egraph.cons( z1 ) ) );
	  implications.push_back( egraph.mkImplies( egraph.cons( enode, egraph.cons( impl ) ) ) );
	}
      }
    }

    if ( result == NULL )
    {
      result = egraph.copyEnodeEtypeTermWithCache( enode );
    }

    assert( result != NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * result = egraph.valDupMap( formula );
  egraph.doneDupMap( );

  if ( !implications.empty( ) )
  {
    implications.push_back( result );
    result = egraph.mkAnd( egraph.cons( implications ) );
  }

  return result;
}

Enode * 
TopLevelProp::simplifyTwinEqualities( Enode * formula, bool & stop )
{
  assert( stop );

  vector< Enode * > unprocessed_enodes;
  egraph.initDupMap( );

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
    if ( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = enode->getCdr( ) ; 
	arg_list != egraph.enil ; 
	arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed
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
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      

    Enode * result = NULL;

    if ( enode->isEq( ) )
    {
      Enode * lhs = enode->get1st( );
      Enode * rhs = enode->get2nd( );
      //
      // (= (se[n] x) (se[n] y)) --> (= x y)
      //
      int slhs, srhs;
      if ( lhs->isSignExtend( &slhs ) 
	&& rhs->isSignExtend( &srhs )
	&& slhs == srhs )
      {
	Enode * lhs_arg = egraph.valDupMap( lhs->get1st( ) );
	Enode * rhs_arg = egraph.valDupMap( rhs->get1st( ) );
	result = egraph.mkEq( egraph.cons( lhs_arg, egraph.cons( rhs_arg ) ) );
      }
      //
      // (= (bvmul x y) (bvmul x z)) --> (= y z)
      //
      else if ( (lhs->isBvmul( ) && rhs->isBvmul( ))
	     || (lhs->isBvand( ) && rhs->isBvand( )) )
      {
	assert( enode->getArity( ) == 2 );

	Enode * lhs_arg_1 = egraph.valDupMap( lhs->get1st( ) );
	Enode * lhs_arg_2 = egraph.valDupMap( lhs->get2nd( ) );
	Enode * rhs_arg_1 = egraph.valDupMap( rhs->get1st( ) );
	Enode * rhs_arg_2 = egraph.valDupMap( rhs->get2nd( ) );

	if ( lhs_arg_1 == rhs_arg_1 )
	  result = egraph.mkEq( egraph.cons( lhs_arg_2, egraph.cons( rhs_arg_2 ) ) );
	else if ( lhs_arg_1 == rhs_arg_2 )
	  result = egraph.mkEq( egraph.cons( lhs_arg_2, egraph.cons( rhs_arg_1 ) ) );
	else if ( lhs_arg_2 == rhs_arg_1 )
	  result = egraph.mkEq( egraph.cons( lhs_arg_1, egraph.cons( rhs_arg_2 ) ) );
	else if ( lhs_arg_2 == rhs_arg_2 )
	  result = egraph.mkEq( egraph.cons( lhs_arg_1, egraph.cons( rhs_arg_1 ) ) );
      }
    }

    if ( result == NULL )
      result = egraph.copyEnodeEtypeTermWithCache( enode );
    else
      stop = false;

    assert( result != NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * result = egraph.valDupMap( formula );
  egraph.doneDupMap( );

  return result;
}

//
// Repeat until fix point:
// . Compute variables that occur only once (unconstrained variables)
// . Replace unconstrained terms (those that contain an unconstrained variable) with a free variable
//
Enode *
TopLevelProp::propagateUnconstrainedVariables( Enode * formula, bool & unc_stop )
{
  bool fix_point_not_reached = true;
  int i = 0;
  while ( fix_point_not_reached )
  {
    vector< int > id_to_inc_edges;
    computeIncomingEdges( formula, id_to_inc_edges ); 
    fix_point_not_reached = false;
    formula = replaceUnconstrainedTerms( formula, id_to_inc_edges, fix_point_not_reached );
    // unc_stop is true if at least another iteration is done, which 
    // means that something has been simplified
    if ( unc_stop && fix_point_not_reached ) unc_stop = false;
  } 

  return formula;
}

Enode *
TopLevelProp::replaceUnconstrainedTerms( Enode * formula
                                      , vector< int > & id_to_inc_edges
                                      , bool & fix_point_not_reached )
{
  vector< Enode * > unprocessed_enodes;
  egraph.initDupMap( );

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
    if ( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = enode->getCdr( ) 
	; arg_list != egraph.enil 
	; arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed
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
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      
    Enode * result = NULL;
    //
    // Operators for which ONE unconstrained variable is enough
    //
    if ( enode->isBvadd  ( )
      || enode->isBvsub  ( )
      || enode->isBvxor  ( )
      || enode->isBvnot  ( ) 
      || enode->isExtract( ) )
    {
      for ( Enode * ll = enode->getCdr( ) 
	  ; !ll->isEnil( )
	  ; ll = ll->getCdr( ) )
      {
	Enode * arg = egraph.valDupMap( ll->getCar( ) );
	if ( arg->isVar( ) 
	  && id_to_inc_edges[ arg->getId( ) ] == 1 )
	{
	  // Allocate a new unconstrained variable for the term
	  char def_name[ 32 ];
	  sprintf( def_name, "UNC_%d", enode->getId( ) );
	  egraph.newSymbol( def_name, DTYPE_BITVEC | enode->getWidth( ) );
	  result = egraph.mkVar( def_name );
	  if ( id_to_inc_edges.size( ) < result->getId( ) )
	    id_to_inc_edges.resize( result->getId( ) + 1, 0 );
	  fix_point_not_reached = true;
	  break;
	}
      }
    }
    //
    // Operators for which ALL unconstrained variable are required (I think)
    //
    else if ( enode->isBvmul( )
           || enode->isBvand( )
           || enode->isBvor ( )
           || enode->isBvudiv( ) 
           || enode->isConcat( ) )
    {
      bool unconstrained = true;
      for ( Enode * ll = enode->getCdr( ) 
	  ; !ll->isEnil( ) && unconstrained
	  ; ll = ll->getCdr( ) )
      {
	Enode * arg = egraph.valDupMap( ll->getCar( ) );
	if ( !arg->isVar( ) 
	  || id_to_inc_edges[ arg->getId( ) ] != 1 )
	{
	  unconstrained = false;
	}
      }
      if ( unconstrained )
      {
	// Allocate a new unconstrained variable for the term
	char def_name[ 32 ];
	sprintf( def_name, "UNC_%d", enode->getId( ) );
	egraph.newSymbol( def_name, DTYPE_BITVEC | enode->getWidth( ) );
	result = egraph.mkVar( def_name );
	if ( (int)id_to_inc_edges.size( ) < result->getId( ) )
	  id_to_inc_edges.resize( result->getId( ) + 1, 0 );
	fix_point_not_reached = true;
      }
    }
    else if ( enode->isEq ( ) 
	   || enode->isNot( ) 
	   || enode->isXor( )
	   || enode->isIff( ) )
    {
      for ( Enode * ll = enode->getCdr( ) 
	  ; !ll->isEnil( )
	  ; ll = ll->getCdr( ) )
      {
	Enode * arg = egraph.valDupMap( ll->getCar( ) );
	if ( arg->isVar( ) 
	  && id_to_inc_edges[ arg->getId( ) ] == 1 )
	{
	  // Allocate a new unconstrained variable for the term
	  char def_name[ 32 ];
	  sprintf( def_name, "UNC_%d", enode->getId( ) );
	  egraph.newSymbol( def_name, DTYPE_BOOL );
	  result = egraph.mkVar( def_name );
	  if ( id_to_inc_edges.size( ) < result->getId( ) )
	    id_to_inc_edges.resize( result->getId( ) + 1, 0 );
	  fix_point_not_reached = true;
	  break;
	}
      }
    }
    else if ( enode->isAnd( )
           || enode->isOr ( ) )
    {
      bool unconstrained = true;
      for ( Enode * ll = enode->getCdr( ) 
	  ; !ll->isEnil( ) && unconstrained
	  ; ll = ll->getCdr( ) )
      {
	Enode * arg = egraph.valDupMap( ll->getCar( ) );
	if ( !arg->isVar( ) 
	  || id_to_inc_edges[ arg->getId( ) ] != 1 )
	  unconstrained = false;
      }
      if ( unconstrained )
      {
	// Allocate a new unconstrained variable for the term
	char def_name[ 32 ];
	sprintf( def_name, "UNC_%d", enode->getId( ) );
	egraph.newSymbol( def_name, DTYPE_BOOL );
	result = egraph.mkVar( def_name );
	if ( id_to_inc_edges.size( ) < result->getId( ) )
	  id_to_inc_edges.resize( result->getId( ) + 1, 0 );
	fix_point_not_reached = true;
      }
    }
    else if ( enode->isIte( ) )
    {
      Enode * i = enode->get1st( );
      Enode * t = enode->get2nd( );
      Enode * e = enode->get3rd( );

      if ( i->isVar( ) && id_to_inc_edges[ i->getId( ) ] == 1 )
      {
	if ( (t->isVar( ) && id_to_inc_edges[ t->getId( ) ] == 1) 
	  || (e->isVar( ) && id_to_inc_edges[ e->getId( ) ] == 1) ) 
	{
	  char def_name[ 32 ];
	  sprintf( def_name, "UNC_%d", enode->getId( ) );
	  egraph.newSymbol( def_name, DTYPE_BITVEC | enode->getWidth( ) );
	  result = egraph.mkVar( def_name );
	  if ( id_to_inc_edges.size( ) < result->getId( ) )
	    id_to_inc_edges.resize( result->getId( ) + 1, 0 );
	  fix_point_not_reached = true;
	}
      }
    }
    //
    // If nothing has been done copy and simplify
    //
    if ( result == NULL )
      result = egraph.copyEnodeEtypeTermWithCache( enode );

    assert( egraph.valDupMap( enode ) == NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );
  assert( new_formula );
  egraph.doneDupMap( );

  return new_formula;
}

void TopLevelProp::computeIncomingEdges( Enode * formula
                                      , vector< int > & id_to_inc_edges )
{
  assert( formula );

  vector< Enode * > unprocessed_enodes;
  egraph.initDup1( );

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
    if ( egraph.isDup1( enode ) )
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
      if ( !egraph.isDup1( arg ) )
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

    for ( Enode * ll = enode->getCdr( )
	; !ll->isEnil( )
	; ll = ll->getCdr( ) )
    {
      Enode * arg = ll->getCar( );
      if ( arg->getId( ) >= (enodeid_t)id_to_inc_edges.size( ) )
	id_to_inc_edges.resize( arg->getId( ) + 1, 0 );
      id_to_inc_edges[ arg->getId( ) ] ++;
    }

    egraph.storeDup1( enode );
  }

  egraph.doneDup1( );
}

/*
void
TopLevelProp::retrieveDistinctions( Enode * formula )
{
  vector< Enode * > unprocessed_enodes;
  egraph.initDup1( );

  unprocessed_enodes  .push_back( formula );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    unprocessed_enodes.pop_back( );
    // 
    // Skip if the node has already been processed before
    //
    if ( egraph.isDup1( enode ) )
      continue;

    egraph.storeDup1( enode );
    //
    // Process children
    //
    if ( enode->isAnd( ) )
    {
      Enode * arg_list;
      for ( arg_list = enode->getCdr( ) 
	  ; arg_list != egraph.enil 
	  ; arg_list = arg_list->getCdr( ) )
      {
	Enode * arg = arg_list->getCar( );
	if ( arg->isDistinct( ) )
	  unprocessed_enodes.push_back( arg );
      }
    }
    else if ( enode->isDistinct( ) )
    {
      //
      // Set distinct flag for the list of arguments
      //
      Enode * arg_list;
      for ( arg_list = enode->getCdr( ) 
	  ; arg_list != egraph.enil 
	  ; arg_list = arg_list->getCdr( ) )
      {
	Enode * arg = arg_list->getCar( );
	if ( enode_to_dist_index.find( arg->getId( ) ) == enode_to_dist_index.end( ) )
	  enode_to_dist_index[ arg->getId( ) ] = 0;
	enode_to_dist_index[ arg->getId( ) ] |= SETBIT(distinctions.size( ));
      }

      distinctions.push_back( enode ); 
    }
  }

  // Done with cache
  egraph.doneDup1( );
}

void
TopLevelProp::retrieveNe( Enode * formula )
{
  vector< Enode * > unprocessed_enodes;
  egraph.initDup1( );

  unprocessed_enodes  .push_back( formula );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    unprocessed_enodes.pop_back( );
    // 
    // Skip if the node has already been processed before
    //
    if ( egraph.isDup1( enode ) )
      continue;

    egraph.storeDup1( enode );
    //
    // Process children
    //
    if ( enode->isAnd( ) )
    {
      Enode * arg_list;
      for ( arg_list = enode->getCdr( ) 
	  ; arg_list != egraph.enil 
	  ; arg_list = arg_list->getCdr( ) )
      {
	Enode * arg = arg_list->getCar( );
	unprocessed_enodes.push_back( arg );
      }
    }
    else if ( enode->isNot( ) && enode->get1st( )->isEq( ) )
    {
      ne.push_back( enode->get1st( ) ); 
    }
  }

  // Done with cache
  egraph.doneDup1( );
}
*/

