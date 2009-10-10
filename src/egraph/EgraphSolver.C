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

#include "Egraph.h"
#include "LA.h"
#include "EmptySolver.h"
#include "BVSolver.h"
#include "LRASolver.h"
#include "OSolver.h"
#include "DLSolver.h"
// TODO: check this - added to support compiling templates
#include "DLSolver.C"

//
// Inform the solver about the existence of node e
//
lbool Egraph::inform( Enode * e )
{
  if ( config.ufconfig.disable && config.logic == QF_UF )
    error( "EUF solver is disabled. Enable it in the configure file.", "" );

  assert( theoryInitialized );
  lbool status;

  Map( enodeid_t, lbool )::iterator it = informed.find( e->getId( ) );

  if ( it == informed.end( ) )
  {
    if ( e->getId( ) >= (enodeid_t)id_to_belong_mask.size( ) )
      id_to_belong_mask.resize( e->getId( ) + 1, 0 );

    assert( id_to_belong_mask[ e->getId( ) ] == 0 );

    if ( !config.ufconfig.disable )
      initializeCong( e );

    bool unassigned_atom = config.logic != QF_UF;
    for ( unsigned i = 1 ; i < tsolvers.size( ) && status == l_Undef ; i ++ )
    {
      if ( tsolvers[ i ]->belongsToT( e ) )
      {
	status = tsolvers[ i ]->inform( e );
	id_to_belong_mask[ e->getId( ) ] |= SETBIT( i );
	unassigned_atom = false;
      }
    }

    if ( unassigned_atom )
      error( e, " cannot be handled by any TSolver. Did you disable some solver in the configure file ?" );

    informed[ e->getId( ) ] = status;
  }
  else
  {
    status = it->second;
  }

  return status;
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
  if ( e->isTerm( ) &&
     ( e->isTrue( ) || e->isFalse( ) ) )
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
  // Set constant for constants
  if ( e->isConstant( ) )
    e->setConstant( e );
  // Add parents relationships
  if ( e->isList( ) )
    e->getCar( )->addParent( e );
  e->getCdr( )->addParent( e );

  // Node initialized
  initialized.insert( e->getId( ) );

  // Insert in SigTab
  insertSigTab( e );
  // Set CB for concatenations
  if ( config.ufconfig.int_extract_concat && e->isTerm( ) && e->isConcat( ) )
    e->setCb( mkCbe( e->getCdr( ) ) );
}

//
// Asserts a literal
//
bool Egraph::assertLit_ ( Enode * e )
{
  //
  // Enable undoable conses if CBE computation is requested
  //
  if ( !enable_undo
    && config.ufconfig.int_extract_concat )
    enable_undo = true;

  assert( theoryInitialized );
  assert( e->isTAtom( ) );
  assert( undo_stack_oper.size( ) == undo_stack_term.size( ) );
  assert( e->hasPolarity( ) );
  assert( e->getPolarity( ) == l_False
       || e->getPolarity( ) == l_True );

  // e is asserted with the same polarity that
  // we deduced: we don't add it to the congruence
  if ( e->isDeduced( )
    && e->getPolarity( ) == e->getDeduced( ) )
    return true;

  bool res = true;
  bool n = e->getPolarity( ) == l_False;

  // Explanation must be empty
  assert( explanation.empty( ) );
  // Assert positive or negative equality
  if ( e->isEq( ) )
  {
    // Has arity 2
    assert( e->getArity( ) == 2 );
    Enode * lhs = e->get1st( );
    Enode * rhs = e->get2nd( );

    // If the polarity of the equality is negative
    if ( n )
    {
      // First of all, assert the negated equality
      if ( config.ufconfig.int_extract_concat )
	res = assertBv( lhs, rhs, e, true );
      else
	res = assertNEq( lhs, rhs, new Reason( e ) );

      // Also, assert that e is equivalent to false
      // to trigger theory-deductions automatically
      if ( res )
      {
	res = assertEq( e, mkFalse( ), new Reason( e ) );
	assert( res );
      }
    }
    // Otherwise the polarity of the equality is positive
    else
    {
      if ( config.ufconfig.int_extract_concat )
	res = assertBv( lhs, rhs, e, false );
      else
	res = assertEq( lhs, rhs, new Reason( e ) );

      // Also, assert that e is equivalent to true
      // to trigger theory-deductions automatically
      if ( res )
      {
	res = assertEq( e, mkTrue( ), new Reason( e ) );
	assert( res );
      }
    }
  }
  // Assert an explicit strict inequality which are
  // (partially) interpreted as negated equalities
  else if ( n &&
	  ( e->isLeq  ( )
         || e->isBvsle( )
	 || e->isBvule( ) ) )
  {
    // Has arity 2
    assert( e->getArity( ) == 2 );
    Enode * lhs = e->get1st( );
    Enode * rhs = e->get2nd( );
    // Special handling for BVs
    if ( config.ufconfig.int_extract_concat )
      res = assertBv( lhs, rhs, e, true );
    else
      res = assertNEq( lhs, rhs, new Reason( e ) );
    //
    // Also, assert that e is equivalent to false
    // Notice that it may trigger a conflict, for instance
    // if we have
    //
    // a <= b, a=c
    //
    // and we push
    //
    // !(c <= b)
    //
    // the last was deduced positive in a previous call,
    // but it can be pushed by BCP negated. This is a little
    // bit asimmetric w.r.t. equality, but it is the
    // same for uninterpreted predicates
    //
    if ( res )
      res = assertEq( e, mkFalse( ), new Reason( e ) );
  }
  // Assert Distinction
  else if ( e->isDistinct( ) )
  {
    if ( n && config.logic == QF_UF )
      error( "can't handle distincts with negative polarity in QF_UF", "" );

    if ( !n )
      res = assertDist( e );
  }
  // Considers <= as uninterpreted if pushed positively
  else if ( e->isUp   ( )
         || e->isLeq  ( )
	 || e->isBvsle( )
	 || e->isBvule( ) )
  {
    if ( n )
      res = assertEq( e, mkFalse( ), new Reason( e ) );
    else
      res = assertEq( e, mkTrue( ), new Reason( e ) );
  }
  else
    error( "can't handle ", e );

  // Cleanup the eq-classes generated during expExplain
  if ( !res )
  {
    conf_index = 0;
    expCleanup( );
  }

#ifdef STATISTICS
  if ( res ) tsolvers_stats[ 0 ]->sat_calls ++;
  else       tsolvers_stats[ 0 ]->uns_calls ++;
#endif

  // Save assert on stack (useful for pop)
  undo_stack_term.push_back( e );
  undo_stack_oper.push_back( ASSERT );

  assert( !res || explanation.empty( ) );
  assert( exp_cleanup.empty( ) );

  return res;
}

bool Egraph::assertLit( Enode * e, bool reason )
{
  suggestions.clear( );

  bool res = config.ufconfig.disable ? true : assertLit_( e );

  if ( res )
  {
    assert( explanation.empty( ) );
    // Assert literal in the other theories
    for ( unsigned i = 1 ; i < tsolvers.size( ) && res ; i ++ )
    {
      OrdinaryTSolver & t = *tsolvers[ i ];
#ifdef STATISTICS
      TSolverStats & ts = *tsolvers_stats[ i ];
#endif

      // Skip solver if this atom does not belong to T 
      if ( (id_to_belong_mask[ e->getId( ) ] & SETBIT( i )) == 0)
	continue;

#ifdef STATISTICS
      size_t deductions_old = deductions.size( );
#endif

      res = t.assertLit( e, reason );
      if ( !res ) 
	conf_index = i;
#ifdef STATISTICS
      if ( res ) 
      {
	ts.sat_calls ++;
	ts.deductions_done += deductions.size( ) - deductions_old;
      }
      else       
	ts.uns_calls ++;
#endif
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
  //
  // Check consistency of negated equalities
  // w.r.t. the current state of the cbes
  // It is currently done in a very dumb and
  // possibly expensive way, but apparently it
  // is still faster than bit-blasting
  //
#define CHECK_NE_IN_COMPLETE_CALL 0
#if CHECK_NE_IN_COMPLETE_CALL
  if ( complete && config.ufconfig.int_extract_concat )
  {
    initDup2( );
    for ( unsigned i = 0 ; i < neq_stored.size( ) && res ; i ++ )
    {
      Enode * lhs = neq_stored[ i ]->get1st( );
      Enode * rhs = neq_stored[ i ]->get2nd( );
      if ( !isDup2( lhs ) && lhs->getCb( )->isCbe( ) )
      {
	Enode * cbe_lhs = cbe( lhs );
	res = assertEq( cbe_lhs, lhs, new Reason( REASON_CBE, lhs ) );
	storeDup2( lhs );
      }
      if ( res && !isDup2( rhs ) && rhs->getCb( )->isCbe( ) )
      {
	Enode * cbe_rhs = cbe( rhs );
	res = assertEq( cbe_rhs, rhs, new Reason( REASON_CBE, rhs ) );
	storeDup2( rhs );
      }
    }
    doneDup2( );

    if ( !res )
    {
      conf_index = 0;
      expCleanup( );
    }
#ifdef STATISTICS
    if ( res ) tsolvers_stats[ 0 ].sat_calls ++;
    else       tsolvers_stats[ 0 ].uns_calls ++;
#endif
  }
#endif

  // Assert literal in the other theories
  for ( unsigned i = 1 ; i < tsolvers.size( ) && res ; i ++ )
  {
    OrdinaryTSolver & t = *tsolvers[ i ];
#ifdef STATISTICS
    TSolverStats & ts = *tsolvers_stats[ i ];
#endif

#ifdef STATISTICS
    size_t deductions_old = deductions.size( );
#endif

    res = t.check( complete );
    if ( !res ) conf_index = i;

#ifdef STATISTICS
    if ( res ) 
    {
      ts.sat_calls ++;
      ts.deductions_done += deductions.size( ) - deductions_old;
    }
    else       
      ts.uns_calls ++;
#endif
  }

  assert( !res || explanation.empty( ) );
  assert( exp_cleanup.empty( ) );

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
  for ( unsigned i = 1 ; i < tsolvers.size( ) ; i ++ )
    tsolvers[ i ]->pushBacktrackPoint( );

  deductions_lim .push_back( deductions.size( ) );
  deductions_last.push_back( deductions_next );
  assert( deductions_last.size( ) == deductions_lim.size( ) );
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
  for ( unsigned i = 1 ; i < tsolvers.size( ) ; i ++ )
    tsolvers[ i ]->popBacktrackPoint( );

  // Restore deduction next
  deductions_next = deductions_last.back( );
  deductions_last.pop_back( );
  // Restore deductions
  size_t new_deductions_size = deductions_lim.back( );
  deductions_lim.pop_back( );
  while( deductions.size( ) > new_deductions_size )
  {
    Enode * e = deductions.back( );
    assert( e->isDeduced( ) );
    e->resetDeduced( );
    deductions.pop_back( );
  }
  assert( deductions_next <= deductions.size( ) );
  assert( deductions_last.size( ) == deductions_lim.size( ) );
}

//
// Returns a deduction
//
Enode * Egraph::getDeduction( )
{
  // Communicate UF deductions
  while ( deductions_next < deductions.size( ) )
  {
    Enode * e = deductions[ deductions_next++ ];
    // For sure this has a deduced polarity
    assert( e->isDeduced( ) );
    // If it has been pushed it is not a good candidate
    // for deduction
    if ( e->hasPolarity( ) )
      continue;

#ifdef STATISTICS
    const int index = e->getDedIndex( );
    tsolvers_stats[ index ]->deductions_sent ++;
#endif

    return e;
  }

  // We have already returned all the possible deductions
  return NULL;
}

//
// Returns a suggestion
//
Enode * Egraph::getSuggestion( )
{
  // Communicate suggestion
  while ( !suggestions.empty( ) )
  {
    Enode * e = suggestions.back( );
    suggestions.pop_back( );
    if ( e->hasPolarity( ) )
      continue;
    if ( e->isDeduced( ) )
      continue;

    return e;
  }

  // We have already returned all 
  // the possible suggestions
  return NULL;
}

//
// Communicate conflict
//
vector< Enode * > & Egraph::getConflict( bool deduction )
{
  assert( 0 <= conf_index && conf_index < (int)tsolvers.size( ) );
#ifdef STATISTICS
  TSolverStats & ts = *tsolvers_stats[ conf_index ];
  if ( deduction )
  {
    if ( (long)explanation.size( ) > ts.max_reas_size )
      ts.max_reas_size = explanation.size( );
    if ( (long)explanation.size( ) < ts.min_reas_size )
      ts.min_reas_size = explanation.size( );
    ts.reasons_sent ++;
    ts.avg_reas_size += explanation.size( );
  }
  else
  {
    if ( (long)explanation.size( ) > ts.max_conf_size )
      ts.max_conf_size = explanation.size( );
    if ( (long)explanation.size( ) < ts.min_conf_size )
      ts.min_conf_size = explanation.size( );
    ts.conflicts_sent ++;
    ts.avg_conf_size += explanation.size( );
  }
#endif
  return explanation;
}

void Egraph::initializeTheorySolvers( )
{
  // Enable undoable cons - New terms are
  // always created w.r.t. the current 
  // status of the congruence closure
  enable_undo = config.incremental;

  assert( !theoryInitialized );
  theoryInitialized = true;

  // Reserve empty space
  tsolvers      .push_back( NULL );
#ifdef STATISTICS
  tsolvers_stats.push_back( new TSolverStats( ) );
#endif

  // No need to instantiate any other solver
  if ( config.logic == QF_UF )
    return;
  // Empty theory solver, a template for user-defined solvers
  if ( config.logic == EMPTY )
  {
    cerr << "# WARNING: Empty solver activated" << endl;
    tsolvers      .push_back( new EmptySolver( tsolvers.size( ), "Empty Solver", config, *this, explanation, deductions, suggestions ) );
#ifdef STATISTICS
    tsolvers_stats.push_back( new TSolverStats( ) );
#endif
  }
  else if ( config.logic == QF_BV )
  {
    if ( !config.bvconfig.disable )
    {
      tsolvers      .push_back( new BVSolver( tsolvers.size( ), "BV Solver", config, *this, explanation, deductions, suggestions ) );
#ifdef STATISTICS
      tsolvers_stats.push_back( new TSolverStats( ) );
#endif
    }
  }
  else if ( config.logic == QF_O )
  {
    if ( !config.oconfig.disable )
    {
      tsolvers.push_back( new OSolver( tsolvers.size( ), "O Solver", config, *this, explanation, deductions, suggestions ) );
#ifdef STATISTICS
      tsolvers_stats.push_back( new TSolverStats( ) );
#endif
    }
  }
  else if ( config.logic == QF_RDL 
         || config.logic == QF_IDL 
	 || config.logic == QF_UFIDL )
  {
#if 0
    if ( !config.oconfig.disable )
    {
      tsolvers.push_back( new OSolver( tsolvers.size( ), "O Solver", config, *this, explanation, deductions, suggestions ) );
#ifdef STATISTICS
      tsolvers_stats.push_back( new TSolverStats( ) );
#endif
    }
#else
    if ( !config.dlconfig.disable )
    {
      if ( getUseGmp( ) )
        tsolvers    .push_back( new DLSolver<Real>( tsolvers.size( ), "DL Solver", config, *this, explanation, deductions, suggestions ) );
      else
        tsolvers    .push_back( new DLSolver<long>( tsolvers.size( ), "DL Solver", config, *this, explanation, deductions, suggestions ) );
#ifdef STATISTICS
      tsolvers_stats.push_back( new TSolverStats( ) );
#endif
    }
#endif
  }
  else if ( config.logic == QF_LRA
         || config.logic == QF_UFLRA )
  {
    if ( !config.lraconfig.disable )
    {
      tsolvers      .push_back( new LRASolver( tsolvers.size( ), "LRA Solver", config, *this, explanation, deductions, suggestions ) );
#ifdef STATISTICS
      tsolvers_stats.push_back( new TSolverStats( ) );
#endif
    }
  }
  else
  {
#ifndef SMTCOMP
    cerr << "# WARNING: Running in INCOMPLETE mode" << endl;
#endif
  }

#ifdef STATISTICS
  assert( tsolvers.size( ) == tsolvers_stats.size( ) );
#endif
}

//===========================================================================
// Private Routines for Core Theory Solver

//
// Assert an equality between nodes x and y
//
bool Egraph::assertEq ( Enode * x, Enode * y, Reason * r )
{
  assert( x->isTerm( ) );
  assert( y->isTerm( ) );
  assert( pending.empty( ) );
  assert( x->isAtom( ) || config.logic != QF_BV   || x->getWidth( ) == y->getWidth( ) );

  expPushNewReason( r );
  pending.push_back( x );
  pending.push_back( y );

  bool congruence_pending = false;

  while ( !pending.empty( ) )
  {
    // Remove a pending equivalence
    assert( pending.size( ) >= 2 );
    assert( pending.size( ) % 2 == 0 );
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

    //
    // Different constants
    //
    if ( reason == NULL )
    {
      assert( p->getRoot( )->getConstant( ) != NULL );
      assert( q->getRoot( )->getConstant( ) != NULL );
      assert( p->getRoot( )->getConstant( ) != q->getRoot( )->getConstant( ) );
      //
      // We need explaining
      //
      // 1. why p and p->constant are equal
      exp_pending.push_back( p );
      exp_pending.push_back( p->getRoot( )->getConstant( ) );
      // 2. why q and q->constant are equal
      exp_pending.push_back( q );
      exp_pending.push_back( q->getRoot( )->getConstant( ) );
      // 3. why p and q are equal
      exp_pending.push_back( q );
      exp_pending.push_back( p );

      initDup1( );
      expExplain( );
      doneDup1( );
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
      assert( reason_1 != NULL );
      assert( reason_2 != NULL );
      expExplain( reason_1, reason_2 );
    }
    else
    {
      // The reason is a negated equality
      assert( reason->isEq( )
	   || reason->isLeq( )
	   || reason->isBvsle( )
	   || reason->isBvule( ) );

      explanation.push_back( reason );

      reason_1 = reason->get1st( );
      reason_2 = reason->get2nd( );

      assert( reason_1 != NULL );
      assert( reason_2 != NULL );

      expExplain( reason_1, reason_2 );
    }

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
bool Egraph::assertNEq ( Enode * x, Enode * y, Reason * r )
{
  expPushNewReason( r );
  Enode * p = x->getRoot( );
  Enode * q = y->getRoot( );

  // They can't be different if the nodes are in the same class
  if ( p == q )
  {
    // The first reason is !r
    explanation.push_back( r->reason );
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
  pdist->reason = r->reason;
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
  qdist->reason = r->reason;
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
  Map( enodeid_t, Enode * ) root_to_enode;
  // Nodes changed
  vector< Enode * > nodes_changed;
  // Assign distinction flag to all arguments
  Enode * list = r->getCdr( );
  while ( list != enil )
  {
    Enode * e = list->getCar( );

    pair< Map( enodeid_t, Enode * )::iterator, bool > elem = root_to_enode.insert( make_pair( e->getRoot( )->getId( ), e ) );
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
    Enode * e = undo_stack_term.back( );

    undo_stack_oper.pop_back( );
    undo_stack_term.pop_back( );

    if ( last_action == MERGE )
    {
      undoMerge( e );
      if ( e->isTerm( ) )
	expRemoveExplanation( );
    }
    else if ( last_action == DISEQ )
      undoDisequality( e );
    else if ( last_action == DIST )
      undoDistinction( e );
    else if ( last_action == SYMB )
      removeSymbol( e );
    else if ( last_action == NUMB )
      removeNumber( e );
    else if ( last_action == CONS )
      undoCons( e );
    else if ( last_action == ASSERT )
      ;// Do nothing
    else if ( last_action == CBETSTORE )
      cbeUndoStore( e );
    else if ( last_action == CBENEQSTORE )
    {
      assert( e == NULL );
      neq_stored.pop_back( );
    }
    else if ( last_action == SPLIT )
    {
      //
      // Clear terms from cb
      //
      for ( Enode * list = e->getCb( )->getCdr( )
	  ; !list->isEnil( )
	  ; list = list->getCdr( ) )
      {
	Enode * arg = list->getCar( );
	cbeRemoveFromCb( arg );
      }
      //
      // Reinitialize cb
      //
      assert( !e->isConcat( ) );
      e->setCb( e );
    }
    else if ( last_action == REASON )
    {
      assert( e == NULL );
      expDeleteLastReason( );
    }
    else
      error( "unknown action ", "" );
  }

  assert( undo_stack_term.size( ) == undo_stack_oper.size( ) );
}

//=============================================================================
// Congruence Closure Routines

//
// Merge the class of x with the class of y
//
void Egraph::merge ( Enode * x, Enode * y )
{
  assert( !x->isConstant( ) || !y->isConstant( ) );
  assert( !x->isConstant( ) || x->getSize( ) == 1 );
  assert( !y->isConstant( ) || y->getSize( ) == 1 );

  // Swap x,y if y has a larger eq class
  if ( x->getSize( ) < y->getSize( )
    || x->isConstant( ) )
  {
    Enode * tmp = x;
    x = y;
    y = tmp;
  }

  assert( !x->isConstant( ) );

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
      // removeSigTab( p );
      // p->isList( ) ? sig_tab_list.erase( p ) : sig_tab_term.erase( p );
      sig_tab.erase( p );
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
    enodeid_t tmp = x->getCid( );
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
      Enode * q = sig_tab.insert( p );
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
    // if ( v->getPolarity( ) == l_Undef && v->getDeduced( ) == l_Undef )
    if ( !v->hasPolarity( ) && !v->isDeduced( ) )
    {
      v->setDeduced( deduced_polarity, id );
      deductions.push_back( v );
#ifdef STATISTICS
      tsolvers_stats[ 0 ]->deductions_done ++;
#endif
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
      // removeSigTab( p );
      // p->isList( ) ? sig_tab_list.erase( p ) : sig_tab_term.erase( p );
      sig_tab.erase( p );
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
    enodeid_t tmp = x->getCid( );
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
    if ( p == cg
      || p->getCar( )->getRoot( ) != cg->getCar( )->getRoot( )
      || p->getCdr( )->getRoot( ) != cg->getCdr( )->getRoot( ) )
    {
      Enode * res = sig_tab.insert( p );
      (void)res;
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
    (void)xc;
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
//
// Robert Nieuwenhuis and Albert Oliveras
// "Proof Producing Congruence Closure"

//
// Store explanation for an eq merge
//
void Egraph::expStoreExplanation ( Enode * x, Enode * y, Reason * reason )
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
  Reason * reason = p->getExpReason( );
  x->setExpParent( NULL );
  x->setExpReason( NULL );

  while( parent != NULL )
  {
    // Save grandparent
    Enode * grandparent = parent->getExpParent( );

    // Save reason
    Reason * saved_reason = reason;
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
    assert( exp_pending.size( ) % 2 == 0 );

    Enode * p = exp_pending.back( ); exp_pending.pop_back( );
    Enode * q = exp_pending.back( ); exp_pending.pop_back( );

    if ( p == q ) continue;

#if PEDANTIC_DEBUG
    assert( checkExplanationTree( p ) );
    assert( checkExplanationTree( q ) );
#endif

    Enode * w = expNCA( p, q );
    assert( w );

    expExplainAlongPath( p, w );
    expExplainAlongPath( q, w );
  }
}

//
// Produce an explanation between nodes x and y
// Wrapper for expExplain
//
void Egraph::expExplain ( Enode * x, Enode * y )
{
  exp_pending.push_back( x );
  exp_pending.push_back( y );

  initDup1( );
  expExplain( );
  doneDup1( );
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
  Enode * v  = expHighestNode( x );
  Enode * to = expHighestNode( y );

  while ( v != to )
  {
    Enode * p = v->getExpParent( );
    assert( p != NULL );
    Reason * r = v->getExpReason( );

    if ( r != NULL && r->type != REASON_DEFAULT )
    {
      assert( config.ufconfig.int_extract_concat );

      if ( r->type == REASON_CONSTANT )
      {
	assert( r->reason->isTerm( ) );
	assert( r->reason->getConstant( ) != NULL );
	assert( p->isConstant( ) || v->isConstant( ) );
	assert( p->isExtract ( ) || v->isExtract ( ) );

	Enode * arg = p->isExtract( ) ? p->get1st( ) : v->get1st( );
	exp_pending.push_back( r->reason );
	exp_pending.push_back( r->reason->getConstant( ) );
	if ( !r->reason->isExtract( ) )
	{
	  assert( r->reason->getWidth( ) == arg->getWidth( ) );
	  assert( r->reason->getRoot( ) == arg->getRoot( ) );
	  exp_pending.push_back( r->reason );
	  exp_pending.push_back( arg );
	}
	else
	{
	  assert( r->reason->get1st( )->getWidth( ) == arg->getWidth( ) );
	  assert( r->reason->get1st( )->getRoot( ) == arg->getRoot( ) );
	  exp_pending.push_back( r->reason->get1st( ) );
	  exp_pending.push_back( arg );
	}
      }
      else if ( r->type == REASON_SLICE )
      {
	Enode * reason = r->reason;
	if( reason->isEq( ) )
	{
	  Enode * lhs = reason->get1st( );
	  Enode * rhs = reason->get2nd( );
	  if ( !isDup1( reason ) )
	  {
	    explanation.push_back( reason );
	    storeDup1( reason );
	  }
	  cbeExplainSlice( r->msb, r->lsb, lhs );
	  cbeExplainSlice( r->msb, r->lsb, rhs );
	}
	else
	{
	  cbeExplainSlice( r->msb, r->lsb, reason );
	}
      }
      else if ( r->type == REASON_CBE )
      {
	Enode * reason = r->reason;
	assert( reason->isTerm( ) );
	cbeExplainCb( reason );
      }
      else
	assert( false );
    }
    // If it is not a congruence edge
    else if ( r != NULL )
    {
      if ( !isDup1( r->reason ) )
      {
	assert( r->reason->isTerm( ) );
	explanation.push_back( r->reason );
	storeDup1( r->reason );
      }
    }
    // Otherwise it is a congruence edge
    // This means that the edge is linking nodes
    // like (v)f(a1,...,an) (p)f(b1,...,bn), and that
    // a1,...,an = b1,...bn. For each pair ai,bi
    // we have therefore to compute the reasons
    else
    {
      assert( v->getCar( ) == p->getCar( ) );
      assert( v->getArity( ) == p->getArity( ) );
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
  assert( x->getArity( ) == y->getArity( ) );
  // No explanation needed if they are the same
  if ( x == y )
    return;

  // Simple explanation if they are arity 0 terms
  if ( x->getArity( ) == 0 )
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

#if PEDANTIC_DEBUG
  assert( checkReachable( x, x_exp_root ) );
  assert( checkReachable( y, y_exp_root ) );
#endif

  // No need to merge elements of the same class
  if ( x_exp_root == y_exp_root )
    return;
  // Save highest node. It is always the node of the parent,
  // as it is closest to the root of the explanation tree
  x_exp_root->setExpRoot( y_exp_root );
  x_exp_root->setExpClassSize( x_exp_root->getExpClassSize( ) + y_exp_root->getExpClassSize( ) );
  // Keep track of this union
  exp_cleanup.push_back( x_exp_root );
  exp_cleanup.push_back( y_exp_root );

#if PEDANTIC_DEBUG
  assert( checkReachable( x, x_exp_root ) );
  assert( checkReachable( y, y_exp_root ) );
#endif
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
  return x_exp_root;
}

Enode * Egraph::expNCA ( Enode * x, Enode * y )
{
  // Increase time stamp
  time_stamp ++;

  Enode * h_x = expHighestNode( x );
  Enode * h_y = expHighestNode( y );

#if PEDANTIC_DEBUG
  assert( checkReachable( x, h_x ) );
  assert( checkReachable( y, h_y ) );
#endif

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
    //if ( y->getExpReason( ) != NULL ) delete y->getExpReason( );
    y->setExpParent( NULL );
    y->setExpReason( NULL );
  }
}

void
Egraph::expPushNewReason( Reason * r )
{
  assert( r );
  undo_stack_term.push_back( NULL );
  undo_stack_oper.push_back( REASON );
  reasons.push_back( r );
}

void
Egraph::expDeleteLastReason( )
{
  assert( !reasons.empty( ) );
  Reason * r = reasons.back( );
  assert( r );
  delete r;
  reasons.pop_back( );
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

    assert( explanation[ i ]->hasPolarity( ) );
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
bool Egraph::checkExplanation ( )
{
  assert( explanation.size( ) > 1 );
  // Check for duplicated literals in conflict
  set< enodeid_t > visited;
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
  set< Reason * > visited;

  while ( x->getExpParent( ) != NULL )
  {
    if ( x->getExpReason( ) != NULL )
    {
      if ( visited.find( x->getExpReason( ) ) != visited.end( ) )
      {
	cerr << "Error: explanation is present twice" << endl;
	return false;
      }
      visited.insert( x->getExpReason( ) );
    }
    x = x->getExpParent( );
  }

  return true;
}

bool Egraph::checkReachable( Enode * x, Enode * h_x )
{
  Enode * orig = x;

  if ( x == h_x )
    return true;

  while ( x->getExpParent( ) != h_x )
  {
    x = x->getExpParent( );
    if ( x == NULL )
    {
      cerr << h_x << " is unreachable from " << orig << endl;
      return false;
    }
  }

  return true;
}
#endif
