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
#include "EmptySolver.h"

//
// Inform the solver about the existence of node e
//
void Egraph::inform( Enode * e ) 
{ 
  assert( theoryInitialized );
  for ( unsigned i = 0 ; i < tsolvers.size( ) ; i ++ )
    tsolvers[ i ]->inform( e );
  initializeCong( e ); 
}

//
// Initialize congruence
//
void Egraph::initializeCong( Enode * e )
{
  // Skip what is not term or list
  assert ( e->isTerm( ) || e->isList( ) );
  // Skip enil
  if ( e->isEnil( ) )
    return;
  // Skip theory atoms that are not equalities or uninterpreted predicates
  if ( e->isTAtom( ) && !e->isEq( ) && !e->isUp( ) )
    return;
  // Skip already initialized nodes
  if ( initialized.find( e->getId( ) ) != initialized.end( ) )
    return;
  // Process arguments first
  if ( e->isList( ) ) 
    initializeCong( e->getCar( ) );
  initializeCong( e->getCdr( ) );
  // Allocate congruence data
  e->allocCongData( );
  // Add parents relationships
  if ( e->isList( ) ) 
    e->getCar( )->addParent( e );
  e->getCdr( )->addParent( e );
  // Node initialized
  initialized.insert( e->getId( ) );
  // Insert in SigTab
  insertSigTab( e );
}

//
// Asserts a literal
//
bool Egraph::assertLit ( Enode * e )
{ 
  assert( theoryInitialized );
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  assert( e->getPolarity( ) == l_False 
       || e->getPolarity( ) == l_True );

  // e is asserted with the same polarity that
  // we deduced: we don't add it to the congruence
  if ( e->getPolarity( ) == e->getDeduced( ) )
    return true;

  bool res = true;
  bool n = e->getPolarity( ) == l_False;

  // Explanation must be empty
  assert( explanation.empty( ) );
  // Assert positive or negative equality
  if ( e->isEq( ) )
  {
    // Has arity 2
    assert( !e->getCdr( )->isEnil( ) );
    assert( !e->getCdr( )->getCdr( )->isEnil( ) );
    assert(  e->getCdr( )->getCdr( )->getCdr( )->isEnil( ) );
    Enode * lhs = e->get1st( );
    Enode * rhs = e->get2nd( );

    // If the polarity of the equality is negative
    // We also assert that the equality is equivalent
    // to the constant false
    if ( n ) 
    {
      // First of all, assert the negated equality
      res = assertNEq( lhs, rhs, e );
      // Also, assert that e is equivalent to false
      if ( res )
      {
	res = assertEq( e, mkFalse( ), e );
	assert( res );
      }
    }
    // Otherwise the polarity of the equality is positive
    // We also assert that the equality is equivalent
    // to the constant true
    else 
    {
      res = assertEq( lhs, rhs, e );
      // Also, assert that e is equivalent to true
      if ( res )
      {
	res = assertEq( e, mkTrue( ), e );
	assert( res );
      }
    }
  }
  // Assert Distinction
  else if ( e->isDistinct( ) )
  {
    if ( n )
    {
      cerr << "Error: we do not handle distincts with negative polarity" << endl;
      exit( 1 );
    }
    res = assertDist( e );
  }
  else if ( e->isUp( ) )
  {
    if ( n ) 
      res = assertEq( e, mkFalse( ), e );
    else
      res = assertEq( e, mkTrue( ), e );
  }
  else
  {
    cerr << "Error: can't handle " << e << endl;
    exit( 1 );
  }

  // Cleanup the eq-classes generated during expExplain
  if ( !res )
  {
    conf_index = -1;
    expCleanup( );
  }

  if ( res )
  {
    // Assert literal in the other theories
    for ( unsigned i = 0 ; i < tsolvers.size( ) && res ; i ++ )
    {
      res = tsolvers[ i ]->assertLit( e );
      if ( !res ) conf_index = i;
    }
  }

  return res;
}

//
// Checks for consistency in theories
//
bool Egraph::check( bool complete )
{
  bool res = true;
  // Assert literal in the other theories
  for ( unsigned i = 0 ; i < tsolvers.size( ) && res ; i ++ )
  {
    res = tsolvers[ i ]->check( complete );
    if ( !res ) conf_index = i;
  }
  return res;
}

//
// Pushes a backtrack point
//
void Egraph::pushBacktrackPoint( )
{
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  backtrack_points.push_back( undo_stack_term.size( ) );
  // Push ordinary theories
  for ( unsigned i = 0 ; i < tsolvers.size( ) ; i ++ )
    tsolvers[ i ]->pushBacktrackPoint( );
}

//
// Pops a backtrack point
//
void Egraph::popBacktrackPoint( )
{ 
  assert( backtrack_points.size( ) > 0 );
  size_t undo_stack_new_size = backtrack_points.back( );
  backtrack_points.pop_back( );
  backtrackToStackSize( undo_stack_new_size );
  // Pop ordinary theories
  for ( unsigned i = 0 ; i < tsolvers.size( ) ; i ++ )
    tsolvers[ i ]->popBacktrackPoint( );
}

//
// Pops until the state that deduced e
//
void Egraph::popUntilDeduced( Enode * e )
{
  assert( e );
  int ded_index = e->getDedIndex( );
  assert( -1 <= ded_index && ded_index < (int)tsolvers.size( ) );

  // Pop the ordinary theory that deduced e
  if ( ded_index >= 0 )
  {
    tsolvers[ ded_index ]->popUntilDeduced( e );
    return;
  }

  assert( (int)enode_id_to_deduced.size( ) > e->getId( ) );
  assert( enode_id_to_deduced[ e->getId( ) ] > 0 );

  size_t new_size = enode_id_to_deduced[ e->getId( ) ];
  assert( backtrack_points.size( ) > 0 );
  backtrackToStackSize( new_size );
  enode_id_to_deduced[ e->getId( ) ] = 0;
}

//
// Returns a deduction
//
Enode * Egraph::getDeduction( )
{
  // Communicate UF deductions
  while ( !deductions.empty( ) ) 
  {
    Enode * e = deductions.front( );
    deductions.pop_front( );
    // For sure this has a deduced polarity
    assert( e->getDeduced( ) != l_Undef );
    // If it hasn't been pushed it is a good candidate
    // for deduction
    if ( e->getPolarity( ) != l_Undef )
    {
      // Otherwise it has to have the same polarity as 
      // deduction. This might happen when we assert say
      // e1, e2, but after asserting e1, e2 is deduced. When
      // asserting e2, either we have a conflict, or e2
      // has the same polarity that we deduced. In this case
      // it is not anymore a deduction because the SAT solver
      // assigned it, and we skip it therefore
      assert( e->getPolarity( ) == e->getDeduced( ) );
      continue;
    }

    return e;
  }

  for ( unsigned i = 0 ; i < tsolvers.size( ) ; i ++ )
  {
    Enode * e = tsolvers[ i ]->getDeduction( );
    if ( e != NULL )
      return e;
  }

  // We have already returned all the possible deductions
  return NULL;
}

//
// Communicate conflict
//
vector< Enode * > & Egraph::getConflict( )
{
  assert( -1 <= conf_index && conf_index < (int)tsolvers.size( ) );
  if ( conf_index == -1 )
    return explanation;
  else
    return tsolvers[ conf_index ]->getConflict( );

}

// 
// Assumption: we assume that the solver is in the
// status that deduced e
//
vector< Enode * > & Egraph::getReason( Enode * e )
{
  int ded_index = e->getDedIndex( ); 
  assert( -1 <= ded_index && ded_index < (int)tsolvers.size( ) );
  if ( ded_index != -1 )
    return tsolvers[ ded_index ]->getReason( e );

  // If this literal needs explanation, it is 
  // because it has been deduced before
  assert( e->getDeduced( ) != l_Undef );
  assert( e->getPolarity( ) != l_Undef );
  assert( e->getPolarity( ) == e->getDeduced( ) );

  bool res = true;
  if ( e->isEq( ) )
  {
    assert( e->getCdr( ) );
    assert( e->getCdr( )->getCdr( ) );
    assert( e->getCdr( )->getCdr( )->getCdr( )->isEnil( ) );

    Enode * lhs = e->get1st( );
    Enode * rhs = e->get2nd( );

    // Assert the negation of e, in order to cause a conflict
    if ( e->getDeduced( ) == l_True ) 
      res = assertNEq( lhs, rhs, e );
    else 
      res = assertEq( lhs, rhs, e );
  }
  else if ( e->isUp( ) )
  {
    if ( e->getDeduced( ) == l_True )
      res = assertEq( e, mkFalse( ), e );
    else
      res = assertEq( e, mkTrue( ), e );
  }
  else
  {
    cerr << "Can't explain: " << e << endl;
    exit( 1 );
  }

  // printExplanation( cerr );
  assert( !res );
  expCleanup( );
  return explanation;
}

void Egraph::initializeTheorySolvers( )
{
  assert( !theoryInitialized );
  theoryInitialized = true;
  // No need to instantiate any other solver
  if ( config.logic == QF_UF )
    return;
  // Empty theory solver, a template for user-defined solvers
  if ( config.logic == EMPTY )
  {
    cerr << "# WARNING: Empty solver activated" << endl;
    tsolvers.push_back( new EmptySolver( tsolvers.size( ), "Empty Solver", config ) );
  }
  else
  {
#ifndef SMTCOMP
    cerr << "# WARNING: this logic is currently unsupported. Running in INCOMPLETE mode" << endl;
#endif
  }
}

//===========================================================================
// Private Routines for Core Theory Solver

//
// Assert an equality between nodes x and y
//
bool Egraph::assertEq ( Enode * x, Enode * y, Enode * r )
{
  assert( x->isTerm( ) );
  assert( y->isTerm( ) );
  assert( pending.empty( ) );
  pending.push_back( x ); 
  pending.push_back( y ); 

  bool congruence_pending = false;

  while ( !pending.empty( ) )
  {
    // Remove a pending equivalence
    assert( pending.size( ) >= 2 );
    Enode * p = pending.back( ); pending.pop_back( );
    Enode * q = pending.back( ); pending.pop_back( );

    if ( p->getRoot( ) == q->getRoot( ) )
      continue;
    
    // Store explanation, for either congruence or eq
    // The third argument is the reason for the merge
    // of p and q; they may merge because of an equality,
    // and in that case the reason is the id of the equality.
    // Or they may merge because of congruence, and in that
    // case the reason is empty (NULL). Notice that we store
    // reasons only among TERMs, and never among LISTs. That
    // means that the reason for p and q being equal has to
    // be found recursively in their arguments. We store the
    // reason even in case of unmergability, to have an
    // automatic way of retrieving a conflict

    if ( p->isTerm( ) )
      expStoreExplanation( p, q, congruence_pending ? NULL : r );

    // Check if they can't be merged
    Enode * reason = NULL;
    bool res = unmergable( p->getRoot( ), q->getRoot( ), &reason );

#if 0 && VERBOSE
    cerr << "unmergable returned " << res << endl;
#endif

    /*
    if ( config.logic == QF_BV &&
	 config.ufconfig.int_extract_concat == 1 )
    {
      // Align the coarsest slicing 
      cbAlign( p->getRoot( ), q->getRoot( ) ); 
    }
    */

    // They are not unmergable, so they can be merged
    if ( !res )
    {
      merge( p->getRoot( ), q->getRoot( ) );
      congruence_pending = true;
      continue;
    }

    // Conflict detected. We should retrieve the explanation
    // We have to distinguish 2 cases. If the reason for the
    // conflict is NULL, it means that a conflict arises because
    // we tried to merge two classes that are assigned to different
    // constants, otherwise we have a proper reason
    Enode * reason_1 = NULL;
    Enode * reason_2 = NULL;

    if ( reason == NULL )
    {
      reason_1 = p->getRoot( )->getConstant( );
      reason_2 = q->getRoot( )->getConstant( );
    }
    else if ( reason->isDistinct( ) )
    {
      // The reason is a distinction
      explanation.push_back( reason );
      // We should iterate through the elements
      // of the distinction and find which atoms
      // are causing the conflict
      Enode * list = reason->getCdr( );
      while ( !list->isEnil( ) )
      {
	Enode * arg = list->getCar( );
	if ( arg->getRoot( ) == p->getRoot( ) ) { assert( reason_1 == NULL ); reason_1 = arg; }
	if ( arg->getRoot( ) == q->getRoot( ) ) { assert( reason_2 == NULL ); reason_2 = arg; }
	list = list->getCdr( );
      }
    }
    else
    {
      // The reason is a negated equality
      assert( reason->isEq( ) );
      explanation.push_back( reason );
      reason_1 = reason->get1st( );
      reason_2 = reason->get2nd( );
    }

    assert( reason_1 != NULL );
    assert( reason_2 != NULL );

    expExplain( reason_1, reason_2 );
#if PEDANTIC_DEBUG
    assert( checkExplanation( ) );
#endif

    // Clear remaining pendings
    pending.clear( );
    // Remove the last explanation that links
    // the two unmergable classes
    expRemoveExplanation( );

    // Return conflict
    return false;
  }

  return true;
}

//
// Assert an inequality between nodes x and y
//
bool Egraph::assertNEq ( Enode * x, Enode * y, Enode * r )
{
  Enode * p = x->getRoot( );
  Enode * q = y->getRoot( );

  // They can't be different if the nodes are in the same class
  if ( p == q )
  {
    // The first reason is !r
    explanation.push_back( r );
    expExplain( x, y );
#if PEDANTIC_DEBUG
    assert( checkExplanation( ) );
#endif
    return false;
  }

  // Is it possible that x is already in the list of y
  // and viceversa ? YES. If things have
  // been done carefully (for instance, if x=y is the same atom
  // as y=x), each theory atom appears only once in the trail. 
  // However it is possible to get x!=y and x<y, and pushing
  // x<y is the same as x!=y for the UF solver. However, I don't
  // think this is going to be a big performance problem, worst
  // case it doubles the size of forbid lists. But checking the
  // lists for duplicates every time would be time-consuming, 
  // especially when we have many !='s. For now I'll leave it
  // unchecked.

  // Create new distinction in q
  Elist * pdist = new Elist;
  pdist->e = p;
  pdist->reason = r;
  // If there is no node in forbid list
  if ( q->getForbid( ) == NULL )
  {
    q->setForbid( pdist );
    pdist->link = pdist;
  }
  // Otherwise we should put the new node after the first
  // and make the first point to pdist. This is because
  // the list is circular, but could be emptq. Therefore
  // we need a "reference" element for keeping it circular.
  // So the last insertion is either the second element or
  // the only present in the list
  else
  {
    pdist->link = q->getForbid( )->link;
    q->getForbid( )->link = pdist;
  }

  // Create new distinction in p
  Elist * qdist = new Elist;
  qdist->e = q;
  qdist->reason = r;
  if ( p->getForbid( ) == NULL )
  {
    p->setForbid( qdist );
    qdist->link = qdist;
  }
  // Same arguments above
  else
  {
    qdist->link = p->getForbid( )->link;
    p->getForbid( )->link = qdist;
  }

  // Save operation in undo_stack
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  undo_stack_oper.push_back( DISEQ );
  undo_stack_term.push_back( q );

  return true;
}

bool Egraph::assertDist( Enode * r )
{
  // Retrieve distinction number
  size_t index = r->getDistIndex( );
  // While asserting, check that no two nodes are congruent
  Map( int, Enode * ) root_to_enode;
  // Nodes changed
  vector< Enode * > nodes_changed;
  // Assign distinction flag to all arguments
  Enode * list = r->getCdr( );
  while ( list != enil )
  {
    Enode * e = list->getCar( );

    pair< Map( int, Enode * )::iterator, bool > elem = root_to_enode.insert( make_pair( e->getRoot( )->getId( ), e ) );
    // Two equivalent nodes in the same distinction. Conflict
    if ( !elem.second )
    {
      // The distinction is of course part of the explanation
      explanation.push_back( r );
      // Extract the other node with the same root
      Enode * p = (*elem.first).second;
      // Check condition
      assert( p->getRoot( ) == e->getRoot( ) );
      // Retrieve explanation
      expExplain( e, p );
      // Revert changes, as the current context is inconsistent
      while( !nodes_changed.empty( ) )
      {
	Enode * n = nodes_changed.back( );
	nodes_changed.pop_back( );
	// Deactivate distinction in n
	n->setDistClasses( n->getDistClasses( ) & ~(SETBIT( index )) );
      }
      return false;
    }
    // Activate distinction in e
    e->setDistClasses( (e->getDistClasses( ) | SETBIT( index )) );
    nodes_changed.push_back( e );
    // Next elem
    list = list->getCdr( );
  }
  // Distinction pushed without conflict
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  undo_stack_oper.push_back( DIST );
  undo_stack_term.push_back( r );

  return true;
}

//
// Backtracks stack to a certain size
//
void Egraph::backtrackToStackSize ( size_t size )
{
  //
  // Restore state at previous backtrack point
  //
  while ( undo_stack_term.size( ) > size )
  {
    oper_t last_action = undo_stack_oper.back( );
    Enode * x = undo_stack_term.back( );

    undo_stack_oper.pop_back( ); 
    undo_stack_term.pop_back( ); 

    if ( last_action == MERGE )
    {
      undoMerge( x );
      if ( x->isTerm( ) )
	expRemoveExplanation( );
    }
    else if ( last_action == DISEQ )
      undoDisequality( x );
    else if ( last_action == DIST )
      undoDistinction( x );
    else if ( last_action == SYMB )
      removeSymbol( x );
    else if ( last_action == CONS )
      undoCons( x );
    /* 
     * For future use
     *
    else if ( last_action == CBE )
    {
      assert( false );
    }
    else if ( last_action == SPLIT )
      x->setCb( x );
     */
    else
    {
      cerr << "Error: unknown action" << endl;
      exit( 1 );
    }
  }

  assert( undo_stack_term.size( ) == undo_stack_oper.size( ) );

  // We have to reset all those deductions that we
  // planned but we didn't commit. For now deductions are
  // simply flushed
  while ( !deductions.empty( ) )
  {
    Enode * e = deductions.front( );
    deductions.pop_front( );
    e->setDeduced( l_Undef, -2 );
    assert( (int)enode_id_to_deduced.size( ) > e->getId( ) );
    enode_id_to_deduced[ e->getId( ) ] = 0;
  }
}

//=============================================================================
// Congruence Closure Routines

//
// Merge the class of x with the class of y
//
void Egraph::merge ( Enode * x, Enode * y )
{
  // Swap x,y if y has a larger eq class
  if ( x->getSize( ) < y->getSize( ) )
  {
    Enode * tmp = x;
    x = y;
    y = tmp;
  }

  // TODO:
  // Propagate equalities to other ordinary theories
  // 

  // Update forbid list for x by adding elements of y
  if ( y->getForbid( ) != NULL )
  {
    // We assign the same forbid list
    if ( x->getForbid( ) == NULL )
      x->setForbid( y->getForbid( ) );
    // Otherwise we splice the two lists
    else
    {
      Elist * tmp = x->getForbid( )->link;
      x->getForbid( )->link = y->getForbid( )->link;
      y->getForbid( )->link = tmp;
    }
  }

  // Merge distinction classes
  x->setDistClasses( ( x->getDistClasses( ) | y->getDistClasses( ) ) );
  // Assign w to the class with fewer parents
  Enode * w = x->getParentSize( ) < y->getParentSize( ) ? x : y ;
  // Visit each parent of w, according to the type of w
  // and remove each congruence root from the signature table
  Enode * p = w->getParent( );
  const Enode * pstart = p;
  const bool scdr = w->isList( );
  for ( ; p != NULL ; )
  {
    assert ( p->isTerm( ) || p->isList( ) );
    // If p is a congruence root
    if ( p == p->getCgPtr( ) )
      removeSigTab( p );
    // Next element
    p = scdr ? p->getSameCdr( ) : p->getSameCar( ) ;
    // End of cycle
    if ( p == pstart )
      p = NULL;
  }

  // Compute deductions that follows from
  // merging x and y. Probably this function
  // could be partially embedded into the next
  // cycle. However, for the sake of simplicity
  // we prefer to separate the two contexts
  if ( config.ufconfig.theory_propagation > 0 )
    deduce( x, y );

  // Perform the union of the two equivalence classes
  // i.e. reroot every node in y's class to point to x

  Enode * v = y;
  const Enode * vstart = v;
  for (;;)
  {
    v->setRoot( x );
    v = v->getNext( ); 
    if ( v == vstart )
      break;
  }

  // Splice next lists
  Enode * tmp = x->getNext( );
  x->setNext( y->getNext( ) );
  y->setNext( tmp );
  // Update size of the congruence class
  x->setSize( x->getSize( ) + y->getSize( ) );
  // Preserve signatures of larger parent set
  if ( x->getParentSize( ) < y->getParentSize( ) )
  {
    int tmp = x->getCid( );
    x->setCid( y->getCid( ) );
    y->setCid( tmp );
  }
  // Insert new signatures and propagate congruences
  p = w->getParent( );
  for ( ; p != NULL ; )
  {
    // If p is a congruence root
    if ( p == p->getCgPtr( ) )
    {
      Enode * q = insertSigTab( p );
      if ( q != p )
      {
	p->setCgPtr( q );
	pending.push_back( p );
	pending.push_back( q );
      }
    }
    // Next element
    p = scdr ? p->getSameCdr( ) : p->getSameCar( ) ;
    // Exit if cycle complete
    if ( p == pstart )
      p = NULL;
  }
  // Merge parent lists
  if ( y->getParent( ) != NULL )
  {
    // If x hasn't got parents, we assign y's one
    if ( x->getParent( ) == NULL )
      x->setParent( y->getParent( ) );
    // Splice the parent lists
    else
    {
      if ( x->isList( ) )
      {
	Enode * tmp = x->getParent( )->getSameCdr( );
        x->getParent( )->setSameCdr( y->getParent( )->getSameCdr( ) );	
        y->getParent( )->setSameCdr( tmp );
      }
      else
      {
	Enode * tmp = x->getParent( )->getSameCar( );
        x->getParent( )->setSameCar( y->getParent( )->getSameCar( ) );	
        y->getParent( )->setSameCar( tmp );
      }
    }
  }
  // Adjust parent size
  x->setParentSize( x->getParentSize( ) + y->getParentSize( ) );

  // Store info about the constant
  if ( y->getConstant( ) != NULL )
  {
    assert( x->getConstant( ) == NULL );
    x->setConstant( y->getConstant( ) );
  }
  // Store info about the constant
  else if ( x->getConstant( ) != NULL )
  {
    assert( y->getConstant( ) == NULL );
    y->setConstant( x->getConstant( ) );
  }
    
  // Push undo record
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  undo_stack_oper.push_back( MERGE );
  undo_stack_term.push_back( y );

#if PEDANTIC_DEBUG
  assert( checkInvariants( ) );
#endif
}

//
// Deduce facts from the merge of x and y
//
void Egraph::deduce( Enode * x, Enode * y )
{
  lbool deduced_polarity = l_Undef;

  if ( x->getConstant( ) == etrue )
    deduced_polarity = l_True;
  else if ( x->getConstant( ) == efalse )
    deduced_polarity = l_False;

  // Let be y store the representant of the class
  // containing the facts that we are about to deduce
  if ( deduced_polarity == l_Undef )
  {
    Enode * tmp = x;
    x = y;
    y = tmp;
  }

  if ( x->getConstant( ) == etrue )
    deduced_polarity = l_True;
  else if ( x->getConstant( ) == efalse )
    deduced_polarity = l_False;

  if ( deduced_polarity == l_Undef )
    return;

  Enode * v = y;
  const Enode * vstart = v;
  for (;;)
  {
    // We deduce only things that aren't currently assigned or
    // that we previously deduced on this branch
    if ( v->getPolarity( ) == l_Undef && v->getDeduced( ) == l_Undef )
    {
      v->setDeduced( deduced_polarity, id );
      deductions.push_back( v );
    if ( v->getId( ) >= (int)enode_id_to_deduced.size( ) )
      enode_id_to_deduced.resize( v->getId( ) + 1, 0 );	
    // + 1 because we want to take into account the merge of 
    // true/false with the representant of x, and this will occur
    // at the end of function merge
    enode_id_to_deduced[ v->getId( ) ] = undo_stack_term.size( ) + 1;
    }
    v = v->getNext( ); 
    if ( v == vstart )
      break;
  }
  
#if PEDANTIC_DEBUG
  assert( checkInvariants( ) );
#endif
}

//
// Starts with the E-graph state that existed after the
// pertinent merge and restores the E-graph to the state
// it had before the pertinent merge
//
void Egraph::undoMerge( Enode * y ) 
{ 
  assert( y );

  // x is the node that was merged with y
  Enode * x = y->getRoot( );
  assert( x );

  // Undoes the merge of the parent lists
  x->setParentSize( x->getParentSize( ) - y->getParentSize( ) );
  // Restore the correct parents 
  if ( y->getParent( ) != NULL )
  {
    // If the parent are equal, that means that
    // y's parent has been assigned to x
    if ( x->getParent( ) == y->getParent( ) )
      x->setParent( NULL );
    // Unsplice the parent lists
    else
    {
      if ( x->isList( ) )
      {
	Enode * tmp = x->getParent( )->getSameCdr( );
        x->getParent( )->setSameCdr( y->getParent( )->getSameCdr( ) );	
        y->getParent( )->setSameCdr( tmp );
      }
      else
      {
	Enode * tmp = x->getParent( )->getSameCar( );
        x->getParent( )->setSameCar( y->getParent( )->getSameCar( ) );	
        y->getParent( )->setSameCar( tmp );
      }
    }
  }
  // Assign w to the smallest parent class
  Enode * w = x->getParentSize( ) < y->getParentSize( ) ? x : y ;
  // Undoes the insertion of the modified signatures 
  Enode * p = w->getParent( );
  const Enode * pstart = p;
  // w might be NULL, i.e. it may not have fathers
  const bool scdr = w == NULL ? false : w->isList( );
  for ( ; p != NULL ; )
  {
    assert( p->isTerm( ) || p->isList( ) );

    // If p is a congruence root
    if ( p == p->getCgPtr( ) )
    {
      assert( lookupSigTab( p ) != NULL );
      removeSigTab( p );
    }
    // Next element
    p = scdr ? p->getSameCdr( ) : p->getSameCar( ) ;
    // End of cycle
    if ( p == pstart )
      p = NULL;
  }
  // Restore the size of x's class
  x->setSize( x->getSize( ) - y->getSize( ) );
  // Unsplice next lists
  Enode * tmp = x->getNext( );
  x->setNext( y->getNext( ) );
  y->setNext( tmp );
  // Reroot each node of y's eq class back to y
  Enode * v = y;
  const Enode * vstart = v;
  for (;;)
  {
    v->setRoot( y );
    v = v->getNext( ); 
    if ( v == vstart )
      break;
  }
  // Undo swapping
  if ( x->getParentSize( ) < y->getParentSize( ) )
  {
    // cerr << "Undo swapping cid" << endl;
    int tmp = x->getCid( );
    x->setCid( y->getCid( ) );
    y->setCid( tmp );
  }
  // Reinsert back signatures that have been removed during
  // the merge operation
  p = w->getParent( );
  for ( ; p != NULL ; )
  {
    assert( p->isTerm( ) || p->isList( ) );

    Enode * cg = p->getCgPtr( );
    // If p is a congruence root 
    if ( p == cg || p->getCar( )->getRoot( ) != cg->getCar( )->getRoot( ) || p->getCdr( )->getRoot( ) != cg->getCdr( )->getRoot( ) )
    {
      Enode * res = insertSigTab( p );
      assert( res == p );
      p->setCgPtr( p );
    }
    // Next element
    p = scdr ? p->getSameCdr( ) : p->getSameCar( ) ;
    // End of cycle
    if ( p == pstart )
      p = NULL;
  }
  
  // Restore distinction classes for x, with a set difference operation
  x->setDistClasses( ( x->getDistClasses( ) & ~(y->getDistClasses( ))) );

  // Restore forbid list for x and y
  if ( x->getForbid( ) == y->getForbid( ) )
    x->setForbid( NULL );
  // Unsplice back the two lists
  else if ( y->getForbid( ) != NULL )
  {
    Elist * tmp = x->getForbid( )->link;
    x->getForbid( )->link = y->getForbid( )->link;
    y->getForbid( )->link = tmp;
  }

  if ( y->getConstant( ) != NULL )
  {
    Enode * yc = y->getConstant( ); 
    Enode * xc = x->getConstant( ); 
    assert( yc == xc );
    // Invariant: the constant comes from one class only
    // No merge can occur beteween terms that point to the
    // same constant, as they would be in the same class already
    assert( ( yc->getRoot( ) == y && xc->getRoot( ) != x ) 
	 || ( yc->getRoot( ) != y && xc->getRoot( ) == x ) );
    // Determine from which class the constant comes from
    if ( yc->getRoot( ) == y )
      x->setConstant( NULL );
    else
      y->setConstant( NULL );
  }
  
  //
  // TODO: unmerge for ordinary theories
  //
  
#if PEDANTIC_DEBUG
  assert( checkInvariants( ) );
#endif
}

//
// Restore the state before the addition of a disequality
//
void Egraph::undoDisequality ( Enode * x )
{
  assert( x->getForbid( ) != NULL );

  // We have to distinct two cases:
  // If there is only one node, that is the
  // distinction to remove
  Elist * xfirst = x->getForbid( );
  Enode * y = NULL;
  if ( xfirst->link == xfirst )
    y = xfirst->e;         
  else
    y = xfirst->link->e;   

  Elist * yfirst = y->getForbid( );
  // Some checks
  assert( yfirst != NULL );
  assert( yfirst->link != yfirst || yfirst->e == x );
  assert( yfirst->link == yfirst || yfirst->link->e == x );
  assert( x->getRoot( ) != y->getRoot( ) );

  Elist * ydist = xfirst->link == xfirst ? xfirst : xfirst->link;

  // Only one node in the list
  if ( ydist->link == ydist )
    x->setForbid( NULL ); 
  // Other nodes in the list
  else
    xfirst->link = ydist->link;
  delete ydist;

  Elist * xdist = yfirst->link == yfirst ? yfirst : yfirst->link;
  
  // Only one node in the list
  if ( xdist->link == xdist )
    y->setForbid( NULL ); 
  // Other nodes in the list
  else
    yfirst->link = xdist->link;
  delete xdist;

#if PEDANTIC_DEBUG
  assert( checkInvariants( ) );
#endif
}

//
// Undoes the effect of pushing a distinction
//
void Egraph::undoDistinction ( Enode * r )
{
  // Retrieve distinction index
  size_t index = r->getDistIndex( );
  // Iterate through the list
  Enode * list = r->getCdr( );
  while ( list != enil )
  {
    Enode * e = list->getCar( );
    // Deactivate distinction in e
    e->setDistClasses( (e->getDistClasses( ) & ~(SETBIT( index ))) );
    // Next elem
    list = list->getCdr( );
  }

#if PEDANTIC_DEBUG
  assert( checkInvariants( ) );
#endif
}

bool Egraph::unmergable ( Enode * x, Enode * y, Enode ** r )
{ 
  assert( x );
  assert( y );
  assert( (*r) == NULL );
  Enode * p = x->getRoot( );
  Enode * q = y->getRoot( );
  // If they are in the same class, they can merge
  if ( p == q ) return false;
  // Check if they have different constants. It is sufficient
  // to check that they both have a constant. It is not
  // possible that the constant is the same. In fact if it was
  // the same, they would be in the same class, bu they are not
  if ( p->getConstant( ) != NULL && q->getConstant( ) != NULL ) return true;
  // Check if they are part of the same distinction
  dist_t intersection = ( p->getDistClasses( ) & q->getDistClasses( ) );
  if ( intersection )
  {
    // Compute the first index in the intersection
    // TODO: Use hacker's delight
    unsigned index = 0;
    while ( ( intersection & 1 ) == 0 ) 
    {
      intersection = intersection >> 1;
      index ++;
    }
    (*r) = indexToDist( index );
    assert( (*r) != NULL );
    return true;
  }
  // Check forbid lists
  const Elist * pstart = p->getForbid( );
  const Elist * qstart = q->getForbid( );
  // If at least one is empty, they can merge
  if ( pstart == NULL || qstart == NULL )
    return false;

  Elist * pptr = (Elist *)pstart;
  Elist * qptr = (Elist *)qstart;

  for (;;)
  {
    // They are unmergable if they are on the other forbid list
    if ( pptr->e->getRoot( ) == q ){ (*r) = pptr->reason; return true; }
    if ( qptr->e->getRoot( ) == p ){ (*r) = qptr->reason; return true; }
    // Pass to the next element
    pptr = pptr->link;
    qptr = qptr->link;
    // If either list finishes, exit. This is ok because
    // if x is on y's forbid list, then y is on x's forbid
    // list as well 
    if ( pptr == pstart ) break;
    if ( qptr == qstart ) break;
  } 
  // If here they are mergable
  assert( (*r) == NULL );
  return false;
}

//=============================================================================
// Explanation Routines: details about these routines are in paper
// Proof Producing Congruence Closure, by NO

// 
// Store explanation for an eq merge
//  
void Egraph::expStoreExplanation ( Enode * x, Enode * y, Enode * reason )
{
  assert( x->isTerm( ) );
  assert( y->isTerm( ) );
  // They must be different because the merge hasn't occured yet
  assert( x->getRoot( ) != y->getRoot( ) );
  // The main observation here is that the explanation tree, altough
  // differently oriented, has the same size as the equivalence tree
  // (actually we don't keep the equivalence tree, because we use
  // the quick-find approach, but here we just need the size). So we
  // can use x->getRoot( )->getSize( ) to know the size of the class of a node
  // x and therefore be sure that the explanation tree is kept
  // balanced (which is a requirement to keep the O(nlogn) bound

  // Make sure that x is the node with the larger number of edges to switch
  if ( x->getRoot( )->getSize( ) < y->getRoot( )->getSize( ) )
  {
    Enode * tmp = x;
    x = y;
    y = tmp;
  } 
  
  // Reroot the explanation tree on y. It has an amortized cost of logn
  expReRootOn( y );
  y->setExpParent( x );
  y->setExpReason( reason );

  // Store both nodes. Because of rerooting operations 
  // we don't know whether x --> y or x <-- y at the moment of 
  // backtracking. So we just save reason and check both parts
  exp_undo_stack.push_back( x );
  exp_undo_stack.push_back( y );

#if PEDANTIC_DEBUG
  assert( checkExplanationTree( x ) );
  assert( checkExplanationTree( y ) );
#endif
}

//
// Subroutine of explainStoreExplanation
// Re-root the tree containing x, in such a way that
// the new root is x itself
//
void Egraph::expReRootOn ( Enode * x )
{
  Enode * p = x;
  Enode * parent = p->getExpParent( );
  Enode * reason = p->getExpReason( );
  x->setExpParent( NULL );
  x->setExpReason( NULL );

  while( parent != NULL )
  {
    // Save grandparent
    Enode * grandparent = parent->getExpParent( );

    // Save reason
    Enode * saved_reason = reason; 
    reason = parent->getExpReason( );

    // Reverse edge & reason
    parent->setExpParent( p );
    parent->setExpReason( saved_reason );

#if PEDANTIC_DEBUG
    assert( checkExplanationTree( parent ) );
#endif

    // Move the two pointers 
    p = parent;
    parent = grandparent; 
  }
}

void Egraph::expExplain ( )
{
  while ( !exp_pending.empty( ) )
  {
    assert( exp_pending.size( ) >= 2 );

    Enode * p = exp_pending.back( ); exp_pending.pop_back( );
    Enode * q = exp_pending.back( ); exp_pending.pop_back( );

    if ( p == q ) continue;

#if PEDANTIC_DEBUG
    assert( checkExplanationTree( p ) );
    assert( checkExplanationTree( q ) );
#endif

    Enode * w = expNCA( p, q );

    expExplainAlongPath( p, w );
    expExplainAlongPath( q, w );
  } 
}

//
// Produce an explanation between nodes x and y
//
void Egraph::expExplain ( Enode * x, Enode * y )
{
  exp_pending.push_back( x ); 
  exp_pending.push_back( y ); 
  expExplain( );
}

void Egraph::expCleanup ( )
{
  // Destroy the eq classes of the explanation
  while ( !exp_cleanup.empty( ) )
  {
    Enode * x = exp_cleanup.back( );
    x->setExpRoot( x );
    x->setExpHighestNode( x );
    exp_cleanup.pop_back( );
  }
}

//
// Subrouine of explain
// A step of explanation for x and y
//
void Egraph::expExplainAlongPath ( Enode * x, Enode * y )
{
  Enode * v = expHighestNode( x );
  Enode * to = expHighestNode( y );

  while ( v != to )
  {
    Enode * p = v->getExpParent( );
    assert( p != NULL );

    // If it is not a congruence edge
    if ( v->getExpReason( ) != NULL )
      explanation.push_back( v->getExpReason( ) );
    // Otherwise it is a congruence edge
    // This means that the edge is linking nodes
    // like (v)f(a1,...,an) (p)f(b1,...,bn), and that
    // a1,...,an = b1,...bn. For each pair ai,bi
    // we have therefore to compute the reasons
    else
    {
      expEnqueueArguments( v, p );
    }

    expUnion( v, p ); 
    v = expHighestNode( p );
  }
}

void Egraph::expEnqueueArguments( Enode * x, Enode * y )
{
  assert( x->isTerm( ) );
  assert( y->isTerm( ) );
  assert( x->getCar( )->getArity( ) == y->getCar( )->getArity( ) );
  // No explanation needed if they are the same
  if ( x == y )
    return;
  // Simple explanation if they are arity 0 temrs
  if ( x->getCar( )->getArity( ) == ENODE_ARITY_0 )
  {
    exp_pending.push_back( x );
    exp_pending.push_back( y );
    return;
  }
  // Otherwise they are the same function symbol
  // Recursively enqueue the explanations for the args
  assert( x->getCar( ) == y->getCar( ) );
  Enode * xptr = x->getCdr( );
  Enode * yptr = y->getCdr( );
  while ( !xptr->isEnil( ) )
  {
    exp_pending.push_back( xptr->getCar( ) );
    exp_pending.push_back( yptr->getCar( ) );
    xptr = xptr->getCdr( );
    yptr = yptr->getCdr( );
  }
  assert( yptr->isEnil( ) );
}

void Egraph::expUnion ( Enode * x, Enode * y )
{
  // Unions are always between a node and its parent
  assert( x->getExpParent( ) == y );
  // Retrieve the representant for the explanation class for x and y
  Enode * x_exp_root = expFind( x ); 
  Enode * y_exp_root = expFind( y );
  // No need to merge elements of the same class
  if ( x_exp_root == y_exp_root )
    return;
  // Save highest node. It is always the node of the parent, 
  // as it is closest to the root of the explanation tree
  x_exp_root->setExpHighestNode( y_exp_root->getExpHighestNode( ) );
  // Make sure that x_exp_root is the class with more elements
  if ( x_exp_root->getExpClassSize( ) < y_exp_root->getExpClassSize( ) )
  {
    Enode * tmp = x_exp_root;
    x_exp_root = y_exp_root;
    y_exp_root = tmp;
  }
  // Reroot y to x and fix the size of the class
  y_exp_root->setExpRoot( x_exp_root );
  x_exp_root->setExpClassSize( x_exp_root->getExpClassSize( ) + y_exp_root->getExpClassSize( ) );
  // Keep track of this union
  exp_cleanup.push_back( x_exp_root );
  exp_cleanup.push_back( y_exp_root );
}

//
// Find the representant of x's equivalence class
// and meanwhile do path compression
//
Enode * Egraph::expFind ( Enode * x )
{
  // If x is the root, return x
  if ( x->getExpRoot( ) == x )
    return x;
  // Recursively find the representant
  Enode * exp_root = expFind( x->getExpRoot( ) );
  // Path compression
  if ( exp_root != x->getExpRoot( ) )
  {
    x->setExpRoot( exp_root );
    exp_cleanup.push_back( x );
  }
  return exp_root;
}

Enode * Egraph::expHighestNode ( Enode * x )
{
  Enode * x_exp_root = expFind( x );
  return x_exp_root->getExpHighestNode( );
}

Enode * Egraph::expNCA ( Enode * x, Enode * y )
{
  // Increase time stamp
  time_stamp ++;
  // Retrieve highest nodes 
  Enode * h_x = expHighestNode( x );
  Enode * h_y = expHighestNode( y );
  
  while ( h_x != h_y )
  {
    if ( h_x != NULL )
    {
      // We reached a node already marked by h_y
      if ( h_x->getExpTimeStamp( ) == time_stamp )
	return h_x;
      // Mark the node and move to the next
      if ( h_x->getExpParent( ) != h_x )
      {
	h_x->setExpTimeStamp( time_stamp );
	h_x = h_x->getExpParent( );
      }
    }
    if ( h_y != NULL )
    {
      // We reached a node already marked by h_x
      if ( h_y->getExpTimeStamp( ) == time_stamp )
	return h_y;
      // Mark the node and move to the next
      if ( h_y->getExpParent( ) != h_y )
      {
	h_y->setExpTimeStamp( time_stamp );
	h_y = h_y->getExpParent( );
      }
    }
  } 
  // Since h_x == h_y, we return h_x
  return h_x;
}

//
// Undoes the effect of expStoreExplanation
//
void Egraph::expRemoveExplanation( )
{
  assert( exp_undo_stack.size( ) >= 2 );

  Enode * x = exp_undo_stack.back( );
  exp_undo_stack.pop_back( );
  Enode * y = exp_undo_stack.back( );
  exp_undo_stack.pop_back( );
  
  assert( x );
  assert( y );
  assert( !x->isEnil( ) );
  assert( !y->isEnil( ) );

  // We observe that we don't need to undo the rerooting 
  // of the explanation trees, because it doesn't affect
  // correctness. We just have to reroot y on itself
  assert( x->getExpParent( ) == y || y->getExpParent( ) == x );
  if ( x->getExpParent( ) == y )
  {
    x->setExpParent( NULL );
    x->setExpReason( NULL );
  }
  else
  {
    y->setExpParent( NULL );
    y->setExpReason( NULL );
  }
}

//=============================================================================
// Debugging Routines

void Egraph::printExplanation( ostream & os )
{
  os << "# Conflict: ";
  for ( unsigned i = 0 ; i < explanation.size( ) ; i ++ )
  {
    if ( i > 0 ) 
      os << ", ";

    if ( explanation[ i ]->getPolarity( ) == l_False )
      os << "!";

    explanation[ i ]->print( os );
  } 
  os << endl;
}

void Egraph::printExplanationTree( ostream & os, Enode * x )
{
  while ( x != NULL )
  {
    os << x;
    if ( x->getExpParent( ) != NULL )
      os << " --[";
    if ( x->getExpReason( ) != NULL )
      os << x->getExpReason( );
    if ( x->getExpParent( ) != NULL )
      os << "]--> ";
    x = x->getExpParent( ); 
  }
}

void Egraph::printExplanationTreeDotty( ostream & os, Enode * x )
{
  os << "digraph expl" << endl;
  os << "{" << endl;

  while ( x != NULL )
  {
    os << x;
    if ( x->getExpParent( ) != NULL )
      os << " -> ";
    x = x->getExpParent( ); 
  }

  os << endl << "}" << endl;
}

void Egraph::printDistinctionList( ostream & os, Enode * x )
{
  Elist * l = x->getForbid( );

  if ( l == NULL )
    return;

  string sep = "";
  do
  {
    os << sep;
    sep = ", ";
    l->e->print( os );
    l = l->link; 
  }
  while( l != x->getForbid( ) );
}


#ifdef PEDANTIC_DEBUG
bool Egraph::checkInvariants ( )
{
  return checkUFInvariants( );
}

bool Egraph::checkExplanation ( )
{
  assert( explanation.size( ) > 1 );
  // Check for duplicated literals in conflict
  set< int > visited;
  for ( unsigned i = 0 ; i < explanation.size( ) ; i ++ )
  {
    if ( visited.find( explanation[ i ]->getId( ) ) != visited.end( ) )
    {
      cerr << "Error: explanation " << explanation[ i ] << " is present twice" << endl;
      return false;
    }
    visited.insert( explanation[ i ]->getId( ) );
  } 

  return true;
}

bool Egraph::checkExplanationTree ( Enode * x )
{
  assert( x );
  set< int > visited;
  
  while ( x->getExpParent( ) != NULL )
  {
    if ( x->getExpReason( ) != NULL )
    {
      if ( visited.find( x->getExpReason( )->getId( ) ) != visited.end( ) )
      {
	cerr << "Error: explanation " << x->getExpReason( ) << " is present twice" << endl;
	return false;
      }
      visited.insert( x->getExpReason( )->getId( ) );
    }
    x = x->getExpParent( ); 
  }

  return true;
}
#endif
