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

#include "LATopLevelProp.h"

Enode *
LATopLevelProp::doit( Enode * formula )
{
  assert( formula );
  //
  // Repeat until fix point
  //
  for ( ;; )
  {
    //
    // Exit if no top-level facts
    //
    if ( !retrieveTopLevelFacts( formula ) )
      break;

    formula = simplify( formula );
  }

  return formula;
}

bool
LATopLevelProp::retrieveTopLevelFacts( Enode * formula )
{
  top_level_facts.clear( );

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
    // Add a new substitution for = if possible
    //
    else if ( enode->isAtom( ) )
    {
      //
      // TODO: do some processing on enode so that
      // it can be used for simplification
      // Also remember to take polarity into account
      //
      Enode * equiv_bound = enode;
      top_level_facts.push_back( equiv_bound );
    }
    else
    {
      error( "case not handled: ", enode );
    }
  }

  // Done with cache
  egraph.doneDup1( );
  return !top_level_facts.empty( );
}

Enode *
LATopLevelProp::simplify( Enode * formula )
{
  assert( formula );
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
      unprocessed_enodes  .pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    if ( enode->isBooleanOperator( ) )
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

    Enode * result = NULL;

    if ( enode->isTAtom( ) )
    {
      // Simplify Tatom here according to top-level info
    }
    else
      result = egraph.copyEnodeEtypeTermWithCache( enode );

    assert( result );
    assert( egraph.valDupMap( enode ) == NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );
  egraph.doneDupMap( );
  assert( new_formula );
  return new_formula;
}
