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

//
// Main Routine. Examine formula and give it to the solver.
// Optionally dump CNF encoding to file
//
lbool Cnfizer::cnfizeAndGiveToSolver( Enode * formula, bool dump )
{
  assert( formula );

  // Dump CNF if requested
  dump_cnf = dump;
  if ( dump_cnf )
  {
    const char * name = "/tmp/cnf.smt";
    dump_out.open( name );
    dump_out << "(benchmark cnf_encoding" << endl;
    dump_out << ":logic ";
    dump_out << logicStr( config.logic ) << endl;
    dump_out << ":extrasorts (I)" << endl;
  }

  vector< Enode * > top_level_formulae;
  // Retrieve top-level formulae
  retrieveTopLevelFormulae( formula, top_level_formulae ); 
  assert( !top_level_formulae.empty( ) );

  Map( int, Enode * ) cnf_cache;
  bool res = true;
  // For each top-level conjunct
  for ( unsigned i = 0 ; i < top_level_formulae.size( ) && res ; i ++ )
  {
    Enode * f = top_level_formulae[ i ];
    // Give it to the solver if already in CNF
    if ( checkCnf( f ) )
    {
      res = giveToSolver( f );
    }
    // Check whether it can be rewritten using deMorgan laws
    else if ( checkDeMorgan( f ) )
    {
      res = deMorganize( f );
    }
    // Otherwise perform cnfization
    else
    {
      Map( int, int ) enodeid_to_incoming_edges;
      enodeid_to_incoming_edges[ f->getId( ) ] = 1;
      computeIncomingEdges( f, enodeid_to_incoming_edges ); // Compute incoming edges for f and children
      f = rewriteMaxArity( f, enodeid_to_incoming_edges );  // Rewrite f with maximum arity for operators
      res = cnfize( f, cnf_cache );                         // Perform actual cnfization (implemented in subclasses)
    }
  }

  // Dump encoding if requested
  if ( dump_cnf )
  {
    set< int > cache;
    for ( vector< vector< Enode * > >::iterator it = dump_list.begin( );
	  it != dump_list.end( );
	  it ++ )
    {
      for ( vector< Enode * >::iterator jt = (*it).begin( );
	    jt != (*it).end( );
	    jt ++ )
      {
	declareStuff( cache, *jt );
      }
    }

    dump_out << ":formula " << endl;
    dump_out << "(and " << endl;

    for ( vector< vector< Enode * > >::iterator it = dump_list.begin( );
	  it != dump_list.end( );
	  it ++ )
    {
      dumpClause( *it );
    }
    dump_out << "))" << endl;
    dump_out.close( );
  }

  if ( !res ) return l_False;

  return l_Undef;
}

//
// Apply simple de Morgan laws to the formula
//
bool Cnfizer::deMorganize( Enode * formula )
{
  assert( !formula->isAnd( ) );
  //
  // Case (not (and a b)) --> (or (not a) (not b))
  //
  if ( formula->isNot( ) && formula->get1st( )->isAnd( ) )
  {
    Enode * conj = formula->get1st( );
    // Retrieve conjuncts
    vector< Enode * > conjuncts;
    retrieveConjuncts( conj, conjuncts );
    vector< Enode * > clause;
    for ( unsigned i = 0 ; i < conjuncts.size( ) ; i ++ )
      clause.push_back( toggleLit( conjuncts[ i ] ) ); 
    if ( dump_cnf ) dump_list.push_back( clause );
    return solver.addSMTClause( clause );
  }

  // Other cases are not contemplated yet
  assert( false );
  return false;
}

//
// Compute the number of incoming edges for e and children
//
void Cnfizer::computeIncomingEdges( Enode * e, Map( int, int ) & enodeid_to_incoming_edges )
{
  assert( e );

  if ( !e->isBooleanOperator( ) ) 
    return;

  for ( Enode * list = e->getCdr( ) ; 
        !list->isEnil( ) ; 
	list = list->getCdr( ) )
  {
    Enode * arg = list->getCar( );
    Map( int, int )::iterator it = enodeid_to_incoming_edges.find( arg->getId( ) );
    if ( it == enodeid_to_incoming_edges.end( ) )
      enodeid_to_incoming_edges[ arg->getId( ) ] = 1;
    else
      it->second ++;
    computeIncomingEdges( arg, enodeid_to_incoming_edges );
  }
}

//
// Rewrite formula with maximum arity for operators
//
Enode * Cnfizer::rewriteMaxArity( Enode * formula, Map( int, int ) & enodeid_to_incoming_edges )
{
  assert( formula );

  vector< Enode * > unprocessed_enodes;       // Stack for unprocessed enodes
  unprocessed_enodes.push_back( formula );    // formula needs to be processed
  Map( int, Enode * ) cache;                  // Cache of processed nodes
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    // 
    // Skip if the node has already been processed before
    //
    if ( cache.find( enode->getId( ) ) != cache.end( ) )
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
      if ( arg->isBooleanOperator( ) 
	&& cache.find( arg->getId( ) ) == cache.end( ) )
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
	cache.insert( make_pair( arg->getId( ), arg ) );
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
    assert ( enode->isBooleanOperator( ) );

    if ( !enode->isNot( ) )
    {
      assert( enode->isAnd( ) || enode->isOr( ) );
      //
      // Construct the new lists for the operators
      //
      result = mergeEnodeArgs( enode, cache, enodeid_to_incoming_edges );
    }
    else
    {
      result = enode;
    }

    assert( result );
    assert( cache.find( enode->getId( ) ) == cache.end( ) );
    cache[ enode->getId( ) ] = result;
  }

  Enode * top_enode = cache[ formula->getId( ) ];
  return top_enode;
}

//
// Merge collected arguments for nodes
//
Enode * Cnfizer::mergeEnodeArgs( Enode * e
                               , Map( int, Enode * ) & cache
		               , Map( int, int ) & enodeid_to_incoming_edges )
{
  assert( e->isAnd( ) || e->isOr( ) );

  Enode * e_symb = e->getCar( );
  vector< Enode * > new_args;
  
  for ( Enode * list = e->getCdr( ) ; 
        !list->isEnil( ) ;
	list = list->getCdr( ) )
  {
    Enode * arg = list->getCar( );
    Enode * sub_arg = cache[ arg->getId( ) ];
    Enode * sym = arg->getCar( );

    if ( sym->getId( ) != e_symb->getId( ) )
    {
      new_args.push_back( sub_arg );
      continue;
    }

    assert( enodeid_to_incoming_edges.find( arg->getId( ) ) != enodeid_to_incoming_edges.end( ) );
    assert( enodeid_to_incoming_edges[ arg->getId( ) ] >= 1 );

    if ( enodeid_to_incoming_edges[ arg->getId( ) ] > 1 )
    {
      new_args.push_back( sub_arg );
      continue;
    }

    for ( Enode * sub_arg_list = sub_arg->getCdr( ) ; 
	  !sub_arg_list->isEnil( ) ; 
	  sub_arg_list = sub_arg_list->getCdr( ) )
      new_args.push_back( sub_arg_list->getCar( ) );
  }

  Enode * new_list = const_cast< Enode * >(egraph.enil);

  while ( !new_args.empty( ) )
  {
    new_list = egraph.cons( new_args.back( ), new_list );
    new_args.pop_back( );
  }

  return egraph.cons( e_symb, new_list );
}

//
// Check whether a formula is in cnf
//
bool Cnfizer::checkCnf( Enode * formula ) 
{ 
  Set( int ) check_cache;
  bool res = checkConj( formula, check_cache ) || checkClause( formula, check_cache ); 
  return res;
}

//
// Check if a formula is a conjunction of clauses
//
bool Cnfizer::checkConj( Enode * e, Set( int ) & check_cache )
{
  if ( !e->isAnd( ) )
    return false;

  if ( check_cache.find( e->getId( ) ) != check_cache.end( ) )   // Already visited term
    return true;

  Enode * list = e->getCdr( );
  for ( ; list != egraph.enil ; list = list->getCdr( ) )
  {
    Enode * arg = list->getCar( );
    if( !checkConj( arg, check_cache ) && !checkClause( arg, check_cache ) )
      return false;
  }

  check_cache.insert( e->getId( ) );

  return true;
}

//
// Check if a formula is a clause
//
bool Cnfizer::checkClause( Enode * e, Set( int ) & check_cache )
{
  assert( e );

  if ( e->isLit( ) )
  {
    check_cache.insert( e->getId( ) );                           // Don't check again
    return true;
  }

  if ( !e->isOr( ) )
    return false;

  if ( check_cache.find( e->getId( ) ) != check_cache.end( ) )   // Already visited term
    return true;
  
  bool is_clause = true;
  for ( Enode * list = e->getCdr( ) ; 
        list != egraph.enil && is_clause ; 
	list = list->getCdr( ) )
    is_clause = checkClause( list->getCar( ), check_cache );

  if ( !is_clause )
    return false;

  check_cache.insert( e->getId( ) );                             // Don't check again

  return true;
}

//
// Check if it can be easily put in clausal for by means of DeMorgan's Rules
//
bool Cnfizer::checkDeMorgan( Enode * e )
{
  Set( int ) check_cache;
  if ( e->isNot( ) && checkPureConj( e->get1st( ), check_cache ) ) return true;
  return false;
}

//
// Check if its a pure conjunction of literals
//
bool Cnfizer::checkPureConj( Enode * e, Set( int ) & check_cache )
{
  if ( check_cache.find( e->getId( ) ) != check_cache.end( ) )
    return true;

  if ( e->isLit( ) ) 
  {
    check_cache.insert( e->getId( ) );
    return true;
  }

  if ( !e->isAnd( ) )
    return false;

  bool is_pure_conj = true;
  for ( Enode * list = e->getCdr( ) ; 
        list != egraph.enil && is_pure_conj ; 
        list = list->getCdr( ) )
    is_pure_conj = checkPureConj( list->getCar( ), check_cache );

  if ( !is_pure_conj )
   return false; 

  check_cache.insert( e->getId( ) );

  return true;
}

//
// Give the formula to the solver
//
bool Cnfizer::giveToSolver( Enode * f )
{
  vector< Enode * > clause;
  //
  // A unit clause
  //
  if ( f->isLit( ) )
  {
    clause.push_back( f );
    if ( dump_cnf ) dump_list.push_back( clause );
    return solver.addSMTClause( clause );
  }
  //
  // A clause
  //
  if ( f->isOr( ) )
  {
    vector< Enode * > lits;
    retrieveClause( f, lits );
    for ( unsigned i = 0 ; i < lits.size( ) ; i ++ )
    {
      Enode * arg = lits[ i ];
      assert( arg->isLit( ) );
      clause.push_back( arg );
    }
    if ( dump_cnf ) dump_list.push_back( clause );
    return solver.addSMTClause( clause );
  } 
  //
  // Conjunction
  //
  assert( f->isAnd( ) );
  vector< Enode * > conj;
  retrieveTopLevelFormulae( f, conj );
  bool result = true;
  for ( unsigned i = 0 ; i < conj.size( ) && result ; i ++ )
    result = giveToSolver( conj[ i ] );
  
  return result;
}

//
// Retrieve the formulae at the top-level
//
void Cnfizer::retrieveTopLevelFormulae( Enode * f, vector< Enode * > & top_level_formulae )
{
  if ( f->isAnd( ) )
    for ( Enode * list = f->getCdr( ) ; 
	  list != egraph.enil ; 
	  list = list->getCdr( ) )
      retrieveTopLevelFormulae( list->getCar( ), top_level_formulae );
  else
    top_level_formulae.push_back( f );
}

//
// Retrieve a clause
//
void Cnfizer::retrieveClause( Enode * f, vector< Enode * > & clause )
{
  assert( f->isLit( ) || f->isOr( ) );

  if ( f->isLit( ) )
  {
    clause.push_back( f );
  }
  else if ( f->isOr( ) )
  {
    for ( Enode * list = f->getCdr( ) ; 
	  list != egraph.enil ; 
	  list = list->getCdr( ) )
      retrieveClause( list->getCar( ), clause );
  }
}

//
// Retrieve conjuncts
//
void Cnfizer::retrieveConjuncts( Enode * f, vector< Enode * > & conjuncts )
{
  assert( f->isLit( ) || f->isAnd( ) );

  if ( f->isLit( ) )
  {
    conjuncts.push_back( f );
  }
  else if ( f->isAnd( ) )
  {
    for ( Enode * list = f->getCdr( ) ; 
	  list != egraph.enil ; 
	  list = list->getCdr( ) )
      retrieveConjuncts( list->getCar( ), conjuncts );
  }
}

//
// A shortcut for literal negation
//
Enode * Cnfizer::toggleLit ( Enode * arg )
{
  assert( arg->isTerm( ) );
  return egraph.mkNot( egraph.cons( arg ) );
}

//
// Dump a clause to file
//
void Cnfizer::dumpClause( vector< Enode * > & clause )
{
  if ( clause.size( ) > 1 ) dump_out << "(or";
  for ( vector< Enode * >::iterator it = clause.begin( ) ;
        it != clause.end( ) ;
        it ++ )
    dump_out << " " << *it;    
  if ( clause.size( ) > 1 ) dump_out << ")";
  dump_out << endl;
}

//
// Dump variable declarations to file
//
void Cnfizer::declareStuff( set< int > & declaredStuff, Enode * e )
{
  if ( declaredStuff.find( e->getId( ) ) != declaredStuff.end( ) )
    return;

  if ( e->isEnil( ) )
    return;

  if ( e->isTerm( ) || e->isList( ) )
  {
    declareStuff( declaredStuff, e->getCar( ) );
    declareStuff( declaredStuff, e->getCdr( ) );
  }
  else if ( e->isSymb( ) && e->getId( ) > ENODE_ID_LAST )
  {
    if ( e->getType( ) == TYPE_BOOL )
    {
      dump_out << ":extrapreds(( " << e->getName( );
    }
    else
    {
      dump_out << ":extrafuns(( " << e->getName( );
    }
    for ( char i = 0 ; i < e->getArity( ) ; i ++ )
    {
      if ( e->getType( ) == TYPE_U )
	dump_out << " U";
      else 
	dump_out << " I";
    }
    if ( e->getType( ) != TYPE_BOOL )
      if ( e->getType( ) == TYPE_U )
	dump_out << " U";
      else 
	dump_out << " I";

    dump_out << " ))";
    dump_out << endl;
    declaredStuff.insert( e->getId( ) );
  }
}
