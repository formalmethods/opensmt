/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

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

#include "TopLevelProp.h"
#include "LA.h"
#include "BVNormalize.h"

#define INLINE_CONSTANTS         0
#define SIMPLIFY_TWIN_EQUALITIES 1

Enode *
TopLevelProp::doit( Enode * formula )
{
  assert( formula );
  //
  // Learn some eq atoms useful for diamonds 
  // benchmarks in SMTLIB
  //
  if ( config.logic == QF_UF )
    formula = learnEqTransitivity( formula );
  //
  // Canonize Arithmetic
  //
  if ( config.logic == QF_O
    || config.logic == QF_IDL
    || config.logic == QF_RDL
    || config.logic == QF_LRA 
    || config.logic == QF_UFIDL 
    || config.logic == QF_UFLRA )
    formula = canonize( formula );
  //
  // Canonize BVs
  //
  if ( config.logic == QF_BV )
  {
    BVNormalize normalizer( egraph, config );
    formula = normalizer.doit( formula );
  }
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
    if ( !retrieveSubstitutions( formula, substitutions ) )
      return egraph.mkFalse( );
    //
    // Step 2: produce a new formula with substitutions done (if any to use)
    //
    bool sub_stop = true;

    if ( !substitutions.empty( ) )
      formula = substitute( formula, substitutions, sub_stop );
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

#if NEW_SPLIT
#else
  if ( config.logic == QF_O
    || config.logic == QF_IDL
    || config.logic == QF_RDL
    || config.logic == QF_LRA
    || config.logic == QF_UFIDL
    || config.logic == QF_UFLRA )
    formula = splitEqs( formula );
#endif

  return formula;
}

bool
TopLevelProp::retrieveSubstitutions( Enode * formula
                                   , Map( enodeid_t, Enode * ) & substitutions )
{
  vector< Enode * > unprocessed_enodes;
  vector< bool >    unprocessed_polarity;
  vector< Enode * > top_level_arith;

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
      if ( enode->isTrue( ) )
      {
	substitutions[ enode->getId( ) ] = egraph.mkTrue( );
	continue;
      }

      if ( enode->isFalse( ) )
      {
	substitutions[ enode->getId( ) ] = egraph.mkFalse( );
	continue;
      }

      // Substitute boolean variable with true/false
      if ( enode->isVar( ) ) 
      {
	substitutions[ enode->getId( ) ] = polarity 
	                                 ? egraph.mkTrue( ) 
					 : egraph.mkFalse( );
	continue;
      }

      assert( enode->isTAtom( ) );

      // Substitute positive equalities
      if ( !enode->isEq( ) || !polarity )
	continue;

      assert( enode->isEq( ) );
      assert( polarity );

      if ( config.logic == QF_UF )
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
      else if ( config.logic == QF_O
             || config.logic == QF_IDL
	     || config.logic == QF_RDL
	     || config.logic == QF_LRA 
             || config.logic == QF_UFIDL
             || config.logic == QF_UFLRA )
      {
	top_level_arith.push_back( enode );
      }
      else if ( config.logic == QF_BV )
      {
	// TODO: do something for BV
      }
      else
      {
	error( "logic not supported yet", "" );
      }
    }
  }

  // Done with cache
  egraph.doneDup1( );
  //
  // Gaussian elimination for top-level arith
  //
  if ( config.logic == QF_O
    || config.logic == QF_IDL 
    || config.logic == QF_RDL
    || config.logic == QF_LRA 
    || config.logic == QF_UFIDL 
    || config.logic == QF_UFLRA )
  {
    vector< LAExpression * > equalities;
    // Initialize
    while ( !top_level_arith.empty( ) )
    {
      equalities.push_back( new LAExpression( top_level_arith.back( ) ) );
      top_level_arith.pop_back( );
    }
    //
    // If just one equality, produce substitution right away
    //
    if ( equalities.size( ) == 0 )
      ; // Do nothing
    else if ( equalities.size( ) == 1 )
    {
      LAExpression & lae = *equalities[ 0 ];
      if ( lae.solve( ) == NULL )
	error( "there is something wrong here", "" );
      pair< Enode *, Enode * > sub = lae.getSubst( egraph );
      assert( sub.first );
      assert( sub.second );
      assert( substitutions.find( (sub.first)->getId( ) ) == substitutions.end( ) );
      substitutions[ (sub.first)->getId( ) ] = sub.second;
    }
    //
    // Otherwise obtain substitutions 
    // by means of Gaussian Elimination
    //
    else
    {
      // 
      // FORWARD substitution
      // We put the matrix equalities into upper triangular form
      //
      for ( unsigned i = 0 ; i + 1 < equalities.size( ) ; i ++ )
      {
	LAExpression & s = *equalities[ i ];
	// Solve w.r.t. first variable
	if ( s.solve( ) == NULL )
	{
	  if ( s.toEnode( egraph ) == egraph.mkTrue( ) )
	    continue;
	  assert( s.toEnode( egraph ) == egraph.mkFalse( ) );
	  return false;
	}
	// Use the first variable x in s to generate a
	// substitution and replace x in lac
	for ( unsigned j = i + 1 ; j < equalities.size( ) ; j ++ )
	{
	  LAExpression & lac = *equalities[ j ];
	  combine( s, lac );
	}
      }
      //
      // BACKWARD substitution
      // From the last equality to the first we put
      // the matrix equalities into canonical form
      //
      for ( int i = equalities.size( ) - 1 ; i >= 1 ; i -- )
      {
	LAExpression & s = *equalities[ i ];
	// Solve w.r.t. first variable
	if ( s.solve( ) == NULL )
	{
	  if ( s.toEnode( egraph ) == egraph.mkTrue( ) )
	    continue;
	  assert( s.toEnode( egraph ) == egraph.mkFalse( ) );
	  return false;
	}
	// Use the first variable x in s as a
	// substitution and replace x in lac
	for ( int j = i - 1 ; j >= 0 ; j -- )
	{
	  LAExpression & lac = *equalities[ j ];
	  combine( s, lac );
	}
      }
      //
      // Now, for each row we get a substitution
      //
      for ( unsigned i = 0 ; i < equalities.size( ) ; i ++ )
      {
	LAExpression & lae = *equalities[ i ];
	pair< Enode *, Enode * > sub = lae.getSubst( egraph );
	if ( sub.first == NULL ) continue;
	assert( sub.second );
	assert( substitutions.find( (sub.first)->getId( ) ) == substitutions.end( ) );
	substitutions[ (sub.first)->getId( ) ] = sub.second;
      }
    }
    // Clean constraints
    while ( !equalities.empty( ) )
    {
      delete equalities.back( );
      equalities.pop_back( );
    }
  }

  return true;
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
	result = it->second;
    }
    else if ( enode->isTAtom( ) )
    {
      Map( enodeid_t, Enode * )::iterator it = substitutions.find( enode->getId( ) );
      // Substitute
      if ( it != substitutions.end( ) )
	result = it->second;
    }
#endif

    if ( result == NULL )
      result = egraph.copyEnodeEtypeTermWithCache( enode );
    else
      sub_stop = false;
    //
    // Canonize again arithmetic/bitvectors theory atoms
    //
    if ( result->isTAtom( ) )
    {
      if ( config.logic == QF_O
	|| config.logic == QF_IDL
	|| config.logic == QF_RDL
	|| config.logic == QF_LRA 
	|| config.logic == QF_UFIDL 
	|| config.logic == QF_UFLRA )
      {
	LAExpression a( result );
	result = a.toEnode( egraph );
      }
    }

    assert( result );
    assert( egraph.valDupMap( enode ) == NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );

  egraph.doneDupMap( );
  assert( new_formula );
  return new_formula;
}

Enode * 
TopLevelProp::learnEqTransitivity( Enode * formula )
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
	  if ( static_cast< int >( id_to_inc_edges.size( ) ) < result->getId( ) )
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
	  if ( static_cast< int >( id_to_inc_edges.size( ) ) < result->getId( ) )
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
	if ( static_cast< int >( id_to_inc_edges.size( ) ) < result->getId( ) )
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
	  if ( static_cast< int >( id_to_inc_edges.size( ) ) < result->getId( ) )
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

Enode *
TopLevelProp::canonize( Enode * formula )
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
    // Replace arithmetic atoms with canonized version
    //
    if ( enode->isTAtom( ) )
    {
      LAExpression a( enode );
      result = a.toEnode( egraph );
    } 
    //
    // If nothing have been done copy and simplify
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

#if NEW_SPLIT
#else
Enode *
TopLevelProp::splitEqs( Enode * formula )
{
  assert( config.logic == QF_O
       || config.logic == QF_IDL
       || config.logic == QF_RDL
       || config.logic == QF_LRA 
       || config.logic == QF_UFIDL 
       || config.logic == QF_UFLRA );

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
    // Replace arithmetic atoms with canonized version
    // TODO: maybe we can push it inside LAExpression
    //
    if ( enode->isTAtom( ) && enode->isEq( ) )
    {
      LAExpression a( enode );
      Enode * e = a.toEnode( egraph );
      Enode * lhs = e->get1st( );
      Enode * rhs = e->get2nd( );
      Enode * leq = egraph.mkLeq( egraph.cons( lhs, egraph.cons( rhs ) ) );
      LAExpression b( leq );
      leq = b.toEnode( egraph );
      Enode * geq = egraph.mkGeq( egraph.cons( lhs, egraph.cons( rhs ) ) );
      LAExpression c( geq );
      geq = c.toEnode( egraph );
      result = egraph.mkAnd( egraph.cons( leq, egraph.cons( geq ) ) );
    } 
    //
    // If nothing have been done copy and simplify
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
#endif
