/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>
      , Edgar Pek           <edgar.pek@lu.unisi.ch>

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

#include "DLSolver.h"
#include "DLGraph.h"
// Included to compile templates
#include "DLGraph.C"

#define DLVERBOSE 0

//
// Allocates a new graph		
//
template< class T > void DLSolver<T>::initGraph()
{
  G = new DLGraph<T>( config, egraph );
}
//
// Deallocate graph
//
template< class T> DLSolver<T>::~DLSolver( ) 
{ 
  delete G; 
} 
//
// The solver is informed of the existence of
// atom e. It might be useful for initializing
// the solver's data structures. This function is
// called before the actual solving starts.
//
template< class T> lbool DLSolver<T>::inform( Enode * e )
{
#if NEW_SIMPLIFICATIONS
#warning "REMOVE THIS REMOVE THIS REMOVE THIS"
  return l_Undef;
#endif

  assert( e );
  assert( belongsToT( e ) );
  assert( !e->hasPolarity( ) );
  assert( e->isLeq( ) );
  G->insertStatic( e );
  return l_Undef;
}

//
// Asserts a literal into the solver. If by chance
// you are able to discover inconsistency you may
// return false. The real consistency state will
// be checked with "check"
//
template <class T> bool DLSolver<T>::assertLit ( Enode * e, bool reason )
{
  assert( e );
  assert( belongsToT( e ) );
  assert( e->hasPolarity( ) );
  //
  // If we are doing eager computation / lazy communication
  //
  DLEdge<T> * deduced_edge = G->getOppositePolarityEdge( e );    
  if ( true == reason && 0 == LAZY_GENERATION  )
  {
    // Find the shortest path p for the deduced edge
    DLPath & shortest_path = G->getShortestPath( deduced_edge );
    assert( ! shortest_path.empty( ) );
    explanation.push_back( e );
    for ( typename DLPath::iterator it = shortest_path.begin( ); it != shortest_path.end( ); ++ it )
        explanation.push_back( (*it)->c );
    return false;
  }

  if ( e->isDeduced( ) 
    && e->getDeduced( ) == e->getPolarity( ) 
    && e->getDedIndex( ) == id )
    return true;
  
  undo_stack_edges.push_back( e );
  //G->insertDynamic          ( e );
  const bool res = G->checkNegCycle( e, reason );
  //const bool res = G->checkNegCycleDFS( e, reason );

  // Return true if satisfiable
  if ( res )
  {
    assert( false == reason );
    if ( config.dlconfig.theory_propagation > 0 )
    {
      G->findHeavyEdges( e );
      sendDeductions( );
    }
    return true;
  }
  //
  // Otherwise retrieve and store negative cycle
  // 
  DLVertex<T> * s = G->getNegCycleVertex( );
  DLVertex<T> * u = s;
  DLPath & conflictEdges = G->getConflictEdges( );
  DLPath lazy_spath;
  do
  {
    DLEdge<T> *edge = conflictEdges[u->id];
    //cerr << "conflict edge: " << edge << endl;
    u = edge->u;
    explanation.push_back( edge->c );
    if (true == reason )
      lazy_spath.push_back( edge );
  }
  while(s != u);
  return false;
}

template< class T > void DLSolver<T>::pushBacktrackPoint ( )
{
  //
  // Undo_stack_deduced_edges is used in the lazy_eager schema
  //
  if ( 0 == LAZY_GENERATION )
  {
    backtrack_points.push_back( G->undo_stack_deduced_edges.size( ) );
  }
  //
  // Backtrack_points.push_back( G->undo_stack_inactive_enodes.size( ) );
  //
  backtrack_points.push_back( undo_stack_edges.size( ) );
}

template< class T >void DLSolver<T>::popBacktrackPoint ( )
{
  assert( G->heavy_edges.empty( ) );
  assert( backtrack_points.size( ) > 0 );
  //
  // Backtrack to dyn edges stack size
  //
  size_t undo_stack_new_size = backtrack_points.back( );
  backtrack_points.pop_back( );
  backtrackToDynEdgesStackSize( undo_stack_new_size );
  
  if ( 0 == LAZY_GENERATION )
  {
    undo_stack_new_size = backtrack_points.back( );
    backtrack_points.pop_back( );
    backtrackToDeducedEdgesStackSize( undo_stack_new_size );
  }
	
}

template < class T> bool DLSolver<T>::check( bool complete )
{
  (void)complete;
  //
  // Here check for consistency
  //
  return true;
}

//
// DL Atoms have one of these shapes
//
// (<= (+ (* 1 x) (* (~ 1) y)) c)
// (<= (+ (* (~ 1) x) (* 1 y)) c)
// (<= (* (~ 1) x) c)
// (<= (* 1 x) c)
//
template< class T > bool DLSolver<T>::belongsToT( Enode * e )
{
  assert( e );
  Enode * lhs = e->get1st( );
  Enode * rhs = e->get2nd( );

  if ( !lhs->isConstant( ) && !rhs->isConstant( ) )
    return false;
  
  if ( lhs->isConstant( ) )
  {
    Enode * tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  Real one_ = 1;
  Real mone_ = -1;
  Enode * one = egraph.mkNum( one_ );
  Enode * mone = egraph.mkNum( mone_ );
  //
  // Two variables
  //
  if ( lhs->isPlus( ) )
  {
    if ( lhs->getArity( ) != 2 ) return false;
    Enode * mon_1 = lhs->get1st( );
    Enode * mon_2 = lhs->get2nd( );
    if ( !mon_1->isTimes( ) ) return false;
    if ( !mon_2->isTimes( ) ) return false;
    Enode * const_1 = mon_1->get1st( );
    Enode * const_2 = mon_2->get1st( );
    Enode * var_1 = mon_1->get2nd( );
    Enode * var_2 = mon_2->get2nd( );
    if ( !var_1->isVar( ) || !var_2->isVar( ) ) return false;
    if ( !const_1->isConstant( ) || !const_2->isConstant( ) ) return false;
    if ( const_1 == one && const_2 == mone ) return true;
    if ( const_2 == one && const_1 == mone ) return true;
    return false;
  }
  //
  // One variable
  //
  Enode * con = lhs->get1st( );
  Enode * var = lhs->get2nd( );
  if ( con != one && con != mone ) return false;
  if ( !var->isVar( ) ) return false;

  return true;
}

// Backtrack stack to the size
template< class T>void DLSolver<T>::backtrackToDynEdgesStackSize( size_t size )
{
  // Restore the state at the previous backtrack point
  while ( undo_stack_edges.size( ) > size )
  {
    Enode * e = undo_stack_edges.back( );
    undo_stack_edges.pop_back( );
    G->deleteActive( e );
  }
}

//
// FIXME: make sure it is correctly handled
//
template< class T>void DLSolver<T>::backtrackToInactiveEnodesStackSize( size_t size )
{
  // Restore the state at the previous backtrack point
  while ( G->undo_stack_inactive_enodes.size( ) > size )
  { 
    Enode * e = G->undo_stack_inactive_enodes.back( );
    G->undo_stack_inactive_enodes.pop_back( );
    G->insertInactive( e );
  }

}

template< class T >void DLSolver<T>::backtrackToDeducedEdgesStackSize( size_t size )
{
  while ( G->undo_stack_deduced_edges.size( ) > size )
  {
    DLEdge<T> * e = G->undo_stack_deduced_edges.back( );
    G->undo_stack_deduced_edges.pop_back( );
    G->clearShortestPath( e );
  }
}

template< class T >void DLSolver<T>::sendDeductions ( )
{
  while ( ! (G->heavy_edges).empty() )
  {
    DLEdge<T> *edge = (G->heavy_edges).back( );
    (G->heavy_edges).pop_back( );
    assert( ! edge->c->hasPolarity( ) );
    assert ( ! edge->c->isDeduced  ( ) );
    //
    // By construction: positive edges have even id (hence, negative edges have odd id)
    //
    edge->c->setDeduced( ( edge->id % 2 ) ? l_False : l_True, id );
    deductions.push_back( edge->c );
  }
}

template< class T >void DLSolver<T>::computeModel( )
{
  
}
