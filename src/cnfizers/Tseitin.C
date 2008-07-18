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

#include "Cnfizer.h"
#include "Tseitin.h"

//
// Performs the actual cnfization
//
bool Tseitin::cnfize( Enode * formula, Map( int, Enode * ) & cnf_cache )
{
  assert( !formula->isAnd( ) );

  vector< Enode * > unprocessed_enodes;       // Stack for unprocessed enodes
  unprocessed_enodes.push_back( formula );    // formula needs to be processed
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    // 
    // Skip if the node has already been processed before
    //
    if ( cnf_cache.find( enode->getId( ) ) != cnf_cache.end( ) )
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
      // Push only if it is an unprocessed boolean operator
      //
      if ( enode->isBooleanOperator( ) 
	&& cnf_cache.find( arg->getId( ) ) == cnf_cache.end( ) )
      {
	unprocessed_enodes.push_back( arg );
	unprocessed_children = true;
      }
      //
      // If it is an atom (either boolean or theory) just
      // store it in the cache
      //
      else if ( arg->isAtom( ) )
      {
	cnf_cache.insert( make_pair( arg->getId( ), arg ) );
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
    // At this point, every child has been processed
    //
    //
    // Do the actual cnfization, according to the node type
    //
    char def_name[ 32 ];

    if ( enode->isLit( ) )
    {
      result = enode;
    }
    else if ( enode->isNot( ) )
    {
      Enode * arg_def = cnf_cache[ enode->get1st( )->getId( ) ];       
      assert( arg_def );
      result = egraph.mkNot( egraph.cons( arg_def ) ); // Toggle the literal
    }
    else
    {
      Enode * new_arg_list = egraph.copyEnodeEtypeListWithCache( enode->getCdr( ), cnf_cache );

      sprintf( def_name, "CNF_DEF_%d", enode->getId( ) );
      egraph.newSymbol( def_name, ENODE_ARITY_0, TYPE_BOOL );  

      Enode * arg_def = egraph.mkVar( def_name ); 
      //
      // Handle boolean operators
      //
      if ( enode->isAnd( ) )
	cnfizeAnd( new_arg_list, arg_def );
      else if ( enode->isOr( ) )
	cnfizeOr( new_arg_list, arg_def );
      else if ( enode->isIff( ) )
	cnfizeIff( new_arg_list, arg_def );
      else if ( enode->isXor( ) )
	cnfizeXor( new_arg_list, arg_def );
      else if ( enode->isIfthenelse( ) )
	cnfizeIfthenelse( new_arg_list, arg_def );
      else if ( enode->isImplies( ) )
	cnfizeImplies( new_arg_list, arg_def );
      else
      {
	cerr << "Error: Operator not handled" << endl;
	exit( 1 );
      }

      result = arg_def;
    }

    assert( result );
    assert( cnf_cache.find( enode->getId( ) ) == cnf_cache.end( ) );
    cnf_cache[ enode->getId( ) ] = result;
  }

  Enode * top_enode = cnf_cache[ formula->getId( ) ];
  vector< Enode * > top_clause;
  top_clause.push_back( top_enode );
  if ( dump_cnf ) dump_list.push_back( top_clause );
  return solver.addSMTClause( top_clause );
}

void Tseitin::cnfizeAnd( Enode * list, Enode * arg_def )
{
  assert( list );
  assert( list->isList( ) );
  //
  // ( a_0 & ... & a_{n-1} ) 
  //
  // <=>
  //
  // aux = ( -aux | a_0 ) & ... & ( -aux | a_{n-1} ) & ( aux & -a_0 & ... & -a_{n-1} ) 
  //
  vector< Enode * > little_clause;
  vector< Enode * > big_clause;
  little_clause.push_back( toggleLit( arg_def ) );
  big_clause   .push_back( arg_def ); 
  for ( ; list != egraph.enil ; list = list->getCdr( ) )
  {
    Enode * arg = list->getCar( );
    little_clause.push_back( arg );
    big_clause   .push_back( toggleLit( arg ) );  
    if ( dump_cnf ) dump_list.push_back( little_clause );
    solver       .addSMTClause( little_clause );        // Adds a little clause to the solver
    little_clause.pop_back( );
  }
  if ( dump_cnf ) dump_list.push_back( big_clause );
  solver.addSMTClause( big_clause );                    // Adds a big clause to the solver
} 

void Tseitin::cnfizeOr( Enode * list, Enode * arg_def )
{
  assert( list );
  assert( list->isList( ) );
  //
  // ( a_0 | ... | a_{n-1} ) 
  //
  // <=>
  //
  // aux = ( aux | -a_0 ) & ... & ( aux | -a_{n-1} ) & ( -aux | a_0 | ... | a_{n-1} ) 
  //
  vector< Enode * > little_clause;
  vector< Enode * > big_clause;
  little_clause.push_back( arg_def );
  big_clause   .push_back( toggleLit( arg_def ) );
  for ( ; list != egraph.enil ; list = list->getCdr( ) )
  {
    Enode * arg = list->getCar( );
    little_clause.push_back( toggleLit( arg ) );
    big_clause   .push_back( arg ); 
    if ( dump_cnf ) dump_list.push_back( little_clause );
    solver       .addSMTClause( little_clause );        // Adds a little clause to the solver
    little_clause.pop_back( );
  }
  if ( dump_cnf ) dump_list.push_back( big_clause );
  solver.addSMTClause( big_clause );                    // Adds a big clause to the solver
} 

//
// Not yet handled
//
void Tseitin::cnfizeXor( Enode *, Enode * )
{
  //
  // ( a_0 xor a_1 ) 
  //
  // <=>
  //
  // aux = ( -aux | a_0  | a_1 ) & ( -aux | -a_0 | -a_1 ) &
  //	   (  aux | -a_0 | a_1 ) & (  aux |  a_0 |  a_1 ) 
  //
  assert( false );
} 

void Tseitin::cnfizeIff( Enode * list, Enode * arg_def )
{
  //
  // ( a_0 <-> a_1 ) 
  //
  // <=>
  //
  // aux = ( -aux |  a_0 | -a_1 ) & ( -aux | -a_0 |  a_1 ) &
  //	   (  aux |  a_0 |  a_1 ) & (  aux | -a_0 | -a_1 ) 
  //
  assert( list->getArity( ) == 2 );
  Enode * arg0 = list->getCar( );
  Enode * arg1 = list->getCdr( )->getCar( );
  vector< Enode * > clause;

  clause.push_back( toggleLit( arg_def ) );
  
  // First clause
  clause.push_back( arg0 );
  clause.push_back( toggleLit( arg1 ) );
  if ( dump_cnf ) dump_list.push_back( clause );
  solver.addSMTClause( clause );           // Adds a little clause to the solver
  clause.pop_back( );
  clause.pop_back( );

  // Second clause
  clause.push_back( toggleLit( arg0 ) );
  clause.push_back( arg1 );
  if ( dump_cnf ) dump_list.push_back( clause );
  solver.addSMTClause( clause );           // Adds a little clause to the solver
  clause.pop_back( );
  clause.pop_back( );

  clause.pop_back( );
  clause.push_back( arg_def );
  
  // Third clause
  clause.push_back( arg0 );
  clause.push_back( arg1 );
  if ( dump_cnf ) dump_list.push_back( clause );
  solver.addSMTClause( clause );           // Adds a little clause to the solver
  clause.pop_back( );
  clause.pop_back( );

  // Fourth clause
  clause.push_back( toggleLit( arg0 ) );
  clause.push_back( toggleLit( arg1 ) );
} 

//
// Not yet handled
//
void Tseitin::cnfizeImplies( Enode *, Enode * )
{
  //
  // ( a_0 -> a_1 ) 
  //
  // <=>
  //
  // aux = ( aux | -a_1 ) & ( aux | a_0 ) & ( -aux | -a_0 | a_1 )
  //
  assert( false );
}

void Tseitin::cnfizeIfthenelse( Enode * list, Enode * arg_def )
{
  //  (!a | !i | t) & (!a | i | e) & (a | !i | !t) & (a | i | !e) 
  //
  // ( if a_0 then a_1 else a_2 ) 
  //
  // <=>
  //
  // aux = ( -aux | -a_0 |  a_1 ) & 
  //       ( -aux |  a_0 |  a_2 ) & 
  //       (  aux | -a_0 | -a_1 ) &
  //       (  aux |  a_0 | -a_2 )
  //
  assert( list->getArity( ) == 3 );
  Enode * arg0 = list->getCar( );
  Enode * arg1 = list->getCdr( )->getCar( );
  Enode * arg2 = list->getCdr( )->getCdr( )->getCar( );
  vector< Enode * > clause;

  clause.push_back( toggleLit( arg_def ) );
  
  // First clause
  clause.push_back( toggleLit( arg0 ) );
  clause.push_back( arg1 );
  clause.pop_back( );
  clause.pop_back( );
  // Second clause
  clause.push_back( arg0 );
  clause.push_back( arg2 );
  clause.pop_back( );
  clause.pop_back( );

  clause.pop_back( );
  clause.push_back( arg_def );
  
  // Third clause
  clause.push_back( toggleLit( arg0 ) );
  clause.push_back( toggleLit( arg1 ) );
  clause.pop_back( );
  clause.pop_back( );
  // Fourth clause
  clause.push_back( arg0 );
  clause.push_back( toggleLit( arg2 ) );
}
