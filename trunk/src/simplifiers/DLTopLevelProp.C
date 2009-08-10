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

#include "DLTopLevelProp.h"

#define MAX_VARS 200
#define ub( I, J ) (upperBounds[ I + nof_vars * J ])
#define REINSERT_ORIGINAL 1

Enode *
DLTopLevelProp::doit( Enode * formula )
{
  assert( formula );

  if ( !topLevelInfo( formula ) )
    return formula;
  
  nof_vars = countVariables( formula );
  //
  // Do nothing if too many variables
  //
  if ( nof_vars >= MAX_VARS )
    return formula;
  //
  // Allocate matrix
  //
  upperBounds = (Real **)(new char[4 * nof_vars * nof_vars]);
  //
  // Initialize with NULL and diagonal 0
  //
  for ( unsigned i = 0 ; i < nof_vars ; i ++ )
    for ( unsigned j = 0 ; j < nof_vars ; j ++ )
      if ( i != j ) 
	ub( i, j ) = NULL;
      else
      {
	Real * n = new Real( 0 );
	ub( i, j ) = n;
      }
  //
  // Repeat until fix point
  //
  for ( ;; )
  {
    //
    // Step 1: retrieve top-level bounds
    //
    const bool changed = retrieveUpperBounds( formula );
    if ( !changed )
      break;
    //
    // Step 2: compute upper bounds
    //
    const bool sat = floydWarshall( );
    if ( !sat )
    {
      formula = egraph.mkFalse( );
      break;
    }
    //
    // Step 3: produce a new formula with substitutions done
    //
    formula = simplify( formula );
  }

  if ( formula != egraph.mkFalse( ) )
  {
    //
    // Add facts to formula
    //
    if ( !top_level_facts.empty( ) )
    {
      Enode * top_list = formula->isAnd( ) ? formula->getCdr( ) : egraph.cons( formula );
      while ( !top_level_facts.empty( ) )
      {
	Enode * atom = top_level_facts.back( );
	assert( atom );
	top_level_facts.pop_back( );
	top_list = egraph.cons( atom, top_list );
      }

      formula = egraph.mkAnd( top_list );
    }
  }

  //
  // Deallocate matrix
  //
  for ( unsigned i = 0 ; i < nof_vars ; i ++ )
    for ( unsigned j = 0 ; j < nof_vars ; j ++ )
      if ( ub( i, j ) != NULL )
	delete ub( i, j );

  delete [] upperBounds;

  //
  // Deallocate dlvars
  //
  while ( !dlvars.empty( ) )
  {
    assert( dlvars.back( ) );
    delete dlvars.back( );
    dlvars.pop_back( );
  }

  return formula;
}

bool
DLTopLevelProp::topLevelInfo( Enode * formula )
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
  
    if ( enode->isAnd( ) )
    {
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
	if ( !egraph.isDup1( arg ) )
	{
	  unprocessed_enodes.push_back( arg );
	  unprocessed_children = true;
	}
      }
    }
    //
    // SKip if unprocessed_children
    //
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      

    if ( enode->isTAtom( ) 
      || ( enode->isNot( ) && enode->get1st( )->isTAtom( ) ) )
    {
      egraph.doneDup1( );
      return true;
    }

    assert( !egraph.isDup1( enode ) );
    egraph.storeDup1( enode );
  }

  egraph.doneDup1( );
  return false;
}

unsigned
DLTopLevelProp::countVariables( Enode * formula )
{
  assert( formula );

  vector< Enode * > vars;

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
    for ( arg_list = enode->getCdr( ) ; 
	arg_list != egraph.enil ; 
	arg_list = arg_list->getCdr( ) )
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

    if ( !enode->isDTypeBool( ) && enode->isVar( ) )
      vars.push_back( enode );

    assert( !egraph.isDup1( enode ) );
    egraph.storeDup1( enode );
  }

  egraph.doneDup1( );

  if ( vars.size( ) <= MAX_VARS )
  {
    for ( size_t i = 0 ; i < vars.size( ) ; i ++ )
    {
      Enode * x = vars[ i ];
      assert( x );
      DLVar * vx = NULL;

      if ( (enodeid_t)id_to_dlvar.size( ) <= x->getId( ) )
	id_to_dlvar.resize( x->getId( ) + 1, NULL );
      //
      // Allocate a new DLVar if necessarvy
      //
      if ( id_to_dlvar[ x->getId( ) ] == NULL )
      {
	vx = new DLVar( dlvars.size( ), x );
	dlvars.push_back( vx );
	id_to_dlvar[ x->getId( ) ] = vx;
      }
      else
	vx = id_to_dlvar[ x->getId( ) ];

      assert( vx );
    } 
  }

  return vars.size( );
}

bool
DLTopLevelProp::retrieveUpperBounds( Enode * formula )
{
  assert( formula );

  bool changed = false;

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

    if ( enode->isAnd( ) )
    {
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
	if ( !egraph.isDup1( arg ) )
	{
	  unprocessed_enodes.push_back( arg );
	  unprocessed_children = true;
	}
      }
    }
    //
    // SKip if unprocessed_children
    //
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      

    //
    // We assume
    // that each atom has been rewritten (by DLCanonizer)
    // into one of the following forms:
    //
    // x <= y
    // x - y <= c
    //
    if ( enode->isTAtom( ) || (enode->isNot( ) && enode->get1st( )->isTAtom( ) ))
    {
      //
      // This fact will be simplified in the next step
      // so we save it here for future reinsert
      //
      top_level_facts.push_back( enode );
      const bool negate = enode->isNot( );
      Enode * e = negate ? enode->get1st( ) : enode;

      Enode * lhs = e->get1st( );
      Enode * rhs = e->get2nd( );

      DLVar * vx = NULL;
      DLVar * vy = NULL;
      Enode * x = NULL;
      Enode * y = NULL;
      Real c;

      if ( lhs->isVar( ) && rhs->isVar( ) )
      {
	x = lhs; 
	y = rhs;
	c = 0;
      }
      else
      {
	x = e->get1st( )->isMinus( ) ? e->get1st( )->get1st( ) : e->get2nd( )->get1st( );
	y = e->get1st( )->isMinus( ) ? e->get1st( )->get2nd( ) : e->get2nd( )->get2nd( );
	Enode * ec = e->get1st( )->isMinus( ) ? e->get2nd( ) : e->get1st( );

	assert( ec->isConstant( ) || ( ec->isUminus( ) && ec->get1st( )->isConstant( ) ) );
	c = ec->isConstant( ) ? ec->getCar( )->getValue( ) : -1 * ec->get1st( )->getCar( )->getValue( );
	//
	// Swap
	//
	if ( !e->get1st( )->isMinus( ) )
	{
	  Enode * tmp = x;
	  x = y;
	  y = tmp;
	  c = -1 * c;
	}
      }

      assert( x );
      assert( y );
      assert( x->getId( ) < (enodeid_t)id_to_dlvar.size( ) );
      assert( y->getId( ) < (enodeid_t)id_to_dlvar.size( ) );
      vx = id_to_dlvar[ x->getId( ) ];
      vy = id_to_dlvar[ y->getId( ) ];
      assert( vx );
      assert( vy );
      //
      // Recompute distance and swap variables
      //
      if ( negate )
      {
	DLVar * tmp = vx;
	vx = vy;
	vy = tmp;

	c = -1 * c - 1;
      }

      if ( ub( vx->id, vy->id ) == NULL )
      {
	ub( vx->id, vy->id ) = new Real( c );
	changed = true;
      }
      else if ( *ub( vx->id, vy->id ) > c )
      {
	*ub( vx->id, vy->id ) = c;
	changed = true;
      }
    }

    assert( !egraph.isDup1( enode ) );
    egraph.storeDup1( enode );
  }

  egraph.doneDup1( );
  return changed;
}

bool DLTopLevelProp::floydWarshall( )
{
  for ( unsigned k = 0 ; k < nof_vars ; k ++ )
  {
    for ( unsigned i = 0 ; i < nof_vars ; i ++ )
    {
      for ( unsigned j = 0 ; j < nof_vars ; j ++ )
      {
	if ( ub( i, j ) == NULL )
	{
	  if ( ub( i, k ) == NULL 
	    || ub( k, j ) == NULL )
	    continue;

	  ub( i, j ) = new Real( *ub( i, k ) + *ub( k, j ) );
	}
	else
	{
	  if ( ub( i, k ) == NULL 
	    || ub( k, j ) == NULL )
	    continue;
	  
	  if ( *ub( i, j ) > *ub( i, k ) + *ub( k, j ) )
	    *ub( i, j ) = *ub( i, k ) + *ub( k, j );
	}
	//
	// Negative cycle detection
	//
	if ( i == j && *ub( i, j ) < 0 )
	  return false;
      }
    }
  }

  return true;
}

Enode *
DLTopLevelProp::simplify( Enode * formula )
{
  assert( formula );
  vector< Enode * > unprocessed_enodes;
  egraph.initDupMap( );

  unprocessed_enodes   .push_back( formula );
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
      unprocessed_enodes  .pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    if ( enode->isBooleanOperator( ) && !enode->isTLit( ) )
    {
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
    }
    //
    // SKip if unprocessed_children
    //
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );
    Enode * result = NULL;

    if ( enode->isTLit( ) )
    {
      const bool negate = enode->isNot( );
      Enode * e = negate ? enode->get1st( ) : enode;

      Enode * lhs = e->get1st( );
      Enode * rhs = e->get2nd( );

      DLVar * vx = NULL;
      DLVar * vy = NULL;
      Real c;

      if ( lhs->isVar( ) && rhs->isVar( ) )
      {
	assert( id_to_dlvar[ lhs->getId( ) ] );
	assert( id_to_dlvar[ rhs->getId( ) ] );
	vx = id_to_dlvar[ lhs->getId( ) ];
	vy = id_to_dlvar[ rhs->getId( ) ];
	c = 0;
      }
      else
      {
        Enode * x = e->get1st( )->isMinus( ) ? e->get1st( )->get1st( ) : e->get2nd( )->get1st( );
	Enode * y = e->get1st( )->isMinus( ) ? e->get1st( )->get2nd( ) : e->get2nd( )->get2nd( );
	assert( id_to_dlvar[ x->getId( ) ] );
	assert( id_to_dlvar[ y->getId( ) ] );
	vx = id_to_dlvar[ x->getId( ) ];
	vy = id_to_dlvar[ y->getId( ) ];
	Enode * ec = e->get1st( )->isMinus( ) ? e->get2nd( ) : e->get1st( );

	assert( ec->isConstant( ) || ( ec->isUminus( ) && ec->get1st( )->isConstant( ) ) );
	c = ec->isConstant( ) ? ec->getCar( )->getValue( ) : -1 * ec->get1st( )->getCar( )->getValue( );
	//
	// Swap when necessary
	//
	if ( !e->get1st( )->isMinus( ) )
	{
	  DLVar * tmp = vx;
	  vx = vy;
	  vy = tmp;
	  c = -1 * c;
	}
      }
      //
      // Swap for negated inequalities
      //
      if ( negate )
      {
	DLVar * tmp = vx;
	vx = vy;
	vy = tmp;
	c = -1 * c - 1;
      }

      assert( vx );
      assert( vy );

      if ( ub( vy->id, vx->id ) != NULL && c + *ub( vy->id, vx->id ) < 0 )
	result = egraph.mkFalse( );
      else if ( ub( vx->id, vy->id ) != NULL && c >= *ub( vx->id, vy->id ) )
	result = egraph.mkTrue( );

      if ( result == NULL ) result = enode;
    }
    else
    {
      result = egraph.copyEnodeEtypeTermWithCache( enode );
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
