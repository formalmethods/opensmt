/*********************************************************************
Author: Edgar Pek <edgar.pek@lu.unisi.ch>
      , Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

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
********************************************************************/

#include "OGraph.h"
#include "OSolver.h"

#define KEEP_UNSAFE 0

//
// Destructor
//
OGraph::~OGraph( )
{
  Enode2Vertex::iterator i2v;
  for ( i2v = vertexMap.begin(); i2v != vertexMap.end(); ++i2v )
    delete i2v->second;

  Enode2Edge::iterator i2e;
  for ( i2e = edgeMap.begin(); i2e != edgeMap.end(); ++i2e ){
    delete (i2e->second).pos; delete (i2e->second).neg;
  }

#if KEEP_UNSAFE
  initVertexUnsafe( );
#endif
}

//
// O Atoms have these shapes
//
// (<= (+ (* 1 x) (* (~ 1) y)) c)
// (<= (+ (* (~ 1) x) (* 1 y)) c)
//
// c in { 0, -1 }
//
OComplEdges OGraph::getOEdge( Enode * e )
{
  Enode2Edge::iterator it = edgeMap.find( e );
  if ( it == edgeMap.end( ) )
  {
    assert( !e->hasPolarity( ) );
    Enode * lhs = e->get1st( );
    Enode * rhs = e->get2nd( );
    assert( !lhs->isConstant( ) );
    Real one_  = 1;
    Real zero_ = 0;
    Real mone_ = -1;
    Enode * one  = egraph.mkNum( one_ );
    Enode * zero = egraph.mkNum( zero_ );
    Enode * mone = egraph.mkNum( mone_ );
    assert( rhs->isConstant( ) );
    assert( rhs == mone || rhs == zero );
    //
    // Solid edges represents 0, dashed represents -1
    //
    bool solid = rhs == zero;
    Enode * x = NULL;
    Enode * y = NULL;
    //
    // Standard constraint
    //
    if ( lhs->isPlus( ) )
    {
      assert( lhs->get1st( )->isTimes( ) );
      assert( lhs->get2nd( )->isTimes( ) );
      assert( lhs->get1st( )->get1st( ) == one || lhs->get1st( )->get1st( ) == mone );
      assert( lhs->get2nd( )->get1st( ) == one || lhs->get2nd( )->get1st( ) == mone );
      if ( lhs->get1st( )->get1st( ) == mone )
      {
	y = lhs->get1st( )->get2nd( );
	x = lhs->get2nd( )->get2nd( );
      }
      else
      {
	x = lhs->get1st( )->get2nd( );
	y = lhs->get2nd( )->get2nd( );
      }
    }
    // 
    // Bound constraint ~1*x < c
    //
    else
    {
      assert( lhs->isTimes( ) );
      assert( lhs->get1st( ) == one || lhs->get1st( ) == mone );
      assert( lhs->get2nd( )->isVar( ) );
      assert( rhs->isConstant( ) );
      assert( rhs == mone || rhs == zero );
      if ( lhs->get1st( ) == one )
      {
	x = lhs->get2nd( );
	y = NULL;
      }
      else
      {
	y = lhs->get2nd( );
	x = NULL;
      }
    }

    OVertex * u = getOVertex( x );
    OVertex * v = getOVertex( y );
    OEdge * pos = new OEdge( e, 2 * edgeMap.size( ), u, v, solid );
    OEdge * neg = new OEdge( e, 2 * edgeMap.size( ) + 1, v, u, !solid );
    OComplEdges edges = OComplEdges( pos, neg );
    edgeMap.insert( pair< Enode *, OComplEdges > ( e, edges ) );
    return edges;
  }

  return it->second;
}

void OGraph::insertStatic( Enode * c )
{
  OEdge * pos = getOEdge( c ).pos;
  OEdge * neg = getOEdge( c ).neg;

  assert( pos );
  assert( neg );

#if OVERB
  cerr << "Created edges: " << endl;
  cerr << "  pos: " << pos << endl;
  cerr << "  neg: " << neg << endl;
  cerr << "  for: " << c << endl;
#endif

  Vcnt = vertexMap.size();
  sAdj.resize( Vcnt );

  dAdj.resize( Vcnt ); dAdjInc.resize( Vcnt );
  hAdj.resize( Vcnt ); hAdjInc.resize( Vcnt );
  iAdj.resize( Vcnt );

  sAdj[ pos->u->id ].push_back( pos );
  sAdj[ neg->u->id ].push_back( neg );
  sEdges.push_back( pos );
  sEdges.push_back( neg );
  Ecnt = Ecnt + 2;
  assert( sEdges.size( ) == Ecnt );

  if ( config.dlconfig.theory_propagation > 0 )
    insertInactive( c );
}

void OGraph::deleteActive( Enode * c )
{
  assert ( c->hasPolarity( ) );
  assert ( edgeMap.find( c ) != edgeMap.end( ) );
  OComplEdges edges = edgeMap.find( c )->second;

  OEdge * e = c->getPolarity( ) == l_True ? edges.pos : edges.neg;
  OEdge * d = dAdj[ e->u->id ].back( );
  (void)d;
  assert ( d == e );
  dAdj[ e->u->id ].pop_back( );
  dEdges.back( );
  dEdges.pop_back( );
  assert( e->v->id < (int) dAdjInc.size( ) );
  OEdge * i = dAdjInc[ e->v->id ].back( );
  (void)i;
  assert ( i == d );
  dAdjInc[ e->v->id ].pop_back( );
  after_backtrack = true;
  updateDynDegree( e );

  if ( config.dlconfig.theory_propagation > 0 )
  {
    insertInactive( c );
  }
}

//
// TODO: Call "inactive" functions only if the
// call may trigger some deduction. If deduction
// is disabled, or in case we know a priori
// that the call will be unsat (TODO: this is the case
// for getting reasons) do not update "inactive"
// data structures
//
void OGraph::insertInactive( Enode * e )
{
  assert ( edgeMap.find( e ) != edgeMap.end( ) );
  OComplEdges edges = edgeMap.find( e )->second;
  OEdge * pos = edges.pos;
  hAdj   [ pos->u->id ].push_back( pos );
  hAdjInc[ pos->v->id ].push_back( pos );
  updateHDegree( pos );

  OEdge * neg = edges.neg;
  hAdj   [ neg->u->id ].push_back( neg );
  hAdjInc[ neg->v->id ].push_back( neg );
  updateHDegree( neg );
}

void OGraph::insertImplied( Enode * c )
{
  assert( config.dlconfig.theory_propagation > 0 );
  deleteInactive( c );
}

OEdge * OGraph::insertDynamic( Enode * c, bool reason )
{
  (void)reason;
  assert( c->hasPolarity( ) );
  assert( edgeMap.find( c ) != edgeMap.end( ) );

  OComplEdges edges = edgeMap.find( c )->second;
  OEdge *e = c->getPolarity ( ) == l_True ? edges.pos : edges.neg;
  assert( e );

  dAdj[ e->u->id ].push_back( e );
  dEdges.push_back( e );

  assert( e->v->id < (int) dAdjInc.size( ) );
  dAdjInc[ e->v->id ].push_back( e );

  updateDynDegree( e );

  if ( config.dlconfig.theory_propagation > 0 )
    deleteInactive( c );

  return e;
}

void OGraph::deleteInactive( Enode * e )
{

  assert ( edgeMap.find( e ) != edgeMap.end( ) );
  OComplEdges edges = edgeMap.find( e )->second;
  OEdge * pos = edges.pos;
  OEdge * neg;
  neg = edges.neg;

  // delete inserted edge from the set of inactive edges
  assert( pos->u->id < (int) hAdj.size( ) );
  deleteFromAdjList( hAdj   [ pos->u->id ], pos );
  assert( pos->v->id < (int) hAdjInc.size( ) );
  deleteFromAdjList( hAdjInc[ pos->v->id ], pos );
  updateHDegree( pos );

  assert( neg->u->id < (int) hAdj.size( ) );
  deleteFromAdjList( hAdj   [ neg->u->id ], neg );
  assert( neg->v->id < (int) hAdjInc.size( ) );
  deleteFromAdjList( hAdjInc[ neg->v->id ], neg );
  updateHDegree( neg );
  assert ( find( hEdges.begin( ), hEdges.end( ), pos ) == hEdges.end( ) );
  assert ( find( hEdges.begin( ), hEdges.end( ), neg ) == hEdges.end( ) );
}

//
// Check for a cycle that contains at least one dashed edge
//
bool OGraph::checkInfeasibleCycle( Enode * c, bool reason )
{
#if OVERB
  cerr << "CHECKING INFEASIBLE CICLE: ";
#endif

  OEdge * e = insertDynamic( c, reason );
  //
  // FIXME: why ? RB
  //
  if ( e == NULL )
    return true;

#if USE_VEC
  unsafe_frontier.clear( );
  safe_frontier  .clear( );
#else
  // Frontier of the search
  list< OVertex * > frontier;
#endif

  assert( changed_vertices.empty( ) );

  conflict_edges.resize( Vcnt );
  //
  // Source vertex u
  // Target vertex v
  //
  OVertex * u = e->u; 
  OVertex * v = e->v;

#define DFS 0
#define UNSAFE_FIRST 1

  // Keep track of unsafe vertexes
#if KEEP_UNSAFE
#else
  initVertexUnsafe( );
#endif
  // Keep track of vertexes in queue, to prevent
  // multiple additions of the same vertex
  initInQueue( );
#if USE_VEC
  if ( !e->solid )
  {
    setVertexUnsafe( v );
    unsafe_frontier.push_back( v );
  }
  else
  {
    safe_frontier.push_back( v );
  }
#else
  // Target vertex is unsafe
  if ( !e->solid )
    setVertexUnsafe( v );     
  // Frontier is just v
  frontier.push_back( v );
#endif
  // Is in queue
  setInQueue( v );

  // Store conflict edge for v
  conflict_edges[ v->id ] = e;

  initSeen( );

#if USE_VEC
  while( !unsafe_frontier.empty( ) || !safe_frontier.empty( ) )
  {
    OVertex * s = NULL;
    if ( !unsafe_frontier.empty( ) )
    {
      s = unsafe_frontier.back( );
      unsafe_frontier.pop_back( );
    }
    else
    {
      s = safe_frontier.back( );
      safe_frontier.pop_back( );
    }
#else
  while( !frontier.empty( ) )
  {
    OVertex * s = frontier.back( );
    frontier.pop_back( );
#endif
    assert( isInQueue( s ) );
    unsetInQueue( s ); 
    setSeen( s );
    //
    // For all outgoing edges
    //
    AdjList & adjList = dAdj[ s->id ];
    for ( AdjList::iterator it = adjList.begin( )
	; it != adjList.end( )
	; ++it )
    {
      OEdge * k = *it;
      OVertex * t = k->v;
      // If a vertex is already unsafe there's nothing to do more
      if ( isVertexUnsafe( t ) )
	continue;
      // Decide whether unsafeness should be propagated to t
      // either the source is unsafe, of the edge we consider is dashed
      if ( isVertexUnsafe( s ) || !k->solid )
	setVertexUnsafe( t );
      // Check conflict condition
      if ( t == u && isVertexUnsafe( t ) )
      {
	// Save edge that propagated k
	conflict_edges[ t->id ] = k;
	infeasibleVertex = u; 
	doneSeen( );
#if KEEP_UNSAFE
	// Reinitialize unsafe vertexes
	doneVertexUnsafe( );
	initVertexUnsafe( );
#else
	doneVertexUnsafe( );
#endif
	doneInQueue( );
#if OVERB
	cerr << "  CONFLICT" << endl;
#endif
	return false;
      }
      //
      // Add t to the frontier if not there already
      // Also we don't add safe vertexes already seen 
      // (to avoid repetition of 0-cycles)
      //
      if ( !isInQueue( t )
	&& (isVertexUnsafe( t ) || !isSeen( t )) )
      {
#if USE_VEC
        if ( isVertexUnsafe( t ) )
	  unsafe_frontier.push_back( t );
	else
	  safe_frontier.push_back( t );
#else
#if UNSAFE_FIRST
        if ( isVertexUnsafe( t ) )
	  frontier.push_back( t );
	else
	  frontier.push_front( t );
#elif DFS
	frontier.push_back( t );
#else
	frontier.push_front( t );
#endif
#endif
	setInQueue( t );
	// Save edge that propagated k
	conflict_edges[ t->id ] = k;
      }
    }
  }

  doneSeen( );
#if KEEP_UNSAFE
#else
  doneVertexUnsafe( );
#endif
  doneInQueue( );
  conflict_edges.clear( );

#if OVERB
  cerr << "  SAT" << endl;
#endif

  return true;
}

//
// Find edges with the larger weight than the shortest path between
// the edge endpoints
//
void OGraph::findHeavyEdges( Enode * c )
{
  assert( c->hasPolarity( ) );

  OComplEdges edges = getOEdge( c );
  OEdge *e = c->getPolarity ( ) == l_True ? edges.pos : edges.neg;
  //
  // Check if there is a parallel edge of smaller weight
  // if yes: return
  // RB: why ?
  //
  if ( e->solid && isParallelAndHeavy( e ) )
    return;
  // 
  // Compute relevancy forward from x
  //
  initDxRel( );
  total_in_deg_dx_rel = 0;
  dx_relevant_vertices.clear( );
  initVertexUnsafeDx( );
  findSSSP( e, O_sssp_forward );
  // 
  // Compute relevancy backward from y
  //
  initDyRel( );
  total_out_deg_dy_rel = 0;
  dy_relevant_vertices.clear( );
  initVertexUnsafeDy( );
  findSSSP( e, O_sssp_backward );
  //
  // Discover deductions
  //
  iterateInactive( );
  //
  // Clear the shortest path tree
  //
  doneVertexUnsafeDx( );
  doneVertexUnsafeDy( );
  doneDxRel( ); 
  doneDyRel( );
}

void OGraph::iterateInactive( )
{
  /*
#if OVERB
  cerr << "ITERATE INACTIVE" << endl;
  cerr << "  DX RELEVANT VERTEXES: " << endl;
  for ( vector< OVertex * >::iterator it = dx_relevant_vertices.begin( )
      ; it != dx_relevant_vertices.end( )
      ; ++ it )
    cerr << "    " << (*it)->e << " | unsafe: " << isVertexUnsafeDx( *it ) << endl;
  cerr << "  DY RELEVANT VERTEXES: " << endl;
  for ( vector< OVertex * >::iterator it = dy_relevant_vertices.begin( )
      ; it != dy_relevant_vertices.end( )
      ; ++ it )
    cerr << "    " << (*it)->e << " | unsafe: " << isVertexUnsafeDy( *it ) << endl;
#endif
*/

  if ( total_out_deg_dy_rel < total_in_deg_dx_rel )
  {
#if FAST_IR
    for ( vector< OVertex * >::iterator it = dy_relevant_vertices.begin( )
#else
    for ( set< OVertex * >::iterator it = dy_relevant_vertices.begin( )
#endif
	; it != dy_relevant_vertices.end( )
	; ++ it )
    {
#if OVERB
      cerr << "  Considering dy relevant vertex: " << (*it)->e << endl;
#endif

      OVertex * u = *it;
      assert( isDyRelValid( u ) );

#if FAST_IR
      if ( !u->dy_relevant ) continue;
#else
      assert( u->dy_relevant );
#endif

      assert( (unsigned)(*it)->id < hAdj.size( ) );
      AdjList & adj_list = hAdj[ (*it)->id ];
      for ( AdjList::iterator aIt = adj_list.begin( )
	  ; aIt != adj_list.end( )
	  ; ++ aIt )
      {
	OEdge * candidate = *aIt;
	Enode * c = candidate->c;
	OVertex * v = candidate->v;
#if OVERB
	cerr << "  Considering connected vertex: " << v->e << endl;
#endif
	// Already deduced edge
	if ( c->hasPolarity( ) || c->isDeduced( ) )
	  continue;
	// Can't deduce as vertexes unreachable
	if ( !isDxRelValid( v ) 
	  || !v->getRelevancy( O_sssp_forward ) )
	  continue;
#if OVERB
	cerr << "    has valid dx and is relevant" << endl;
#endif
	// If solid edge, deduce
	if ( candidate->solid )
	{
	  heavy_edges.push_back( candidate );
#if OVERB
	  cerr << "  DEDUCED: " << (candidate->id%2?"!":" ") << c << endl;
#endif
	}
	// If dashed edge, deduce only if at least one is unsafe
	else if ( isVertexUnsafeDy( u ) || isVertexUnsafeDx( v ) )
	{
	  heavy_edges.push_back( candidate );
#if OVERB
	  cerr << "  DEDUCED: " << (candidate->id%2?"!":" ") << c << endl;
#endif
	}
      }
    }
  }
  else
  {
#if FAST_IR
    for ( vector< OVertex * >::iterator it = dx_relevant_vertices.begin( )
#else
    for ( set< OVertex * >::iterator it = dx_relevant_vertices.begin( )
#endif
	; it != dx_relevant_vertices.end( )
	; ++ it )
    {
#if OVERB
      // cerr << "  Considering dx relevant vertex: " << (*it)->e << endl;
#endif

      OVertex * u = *it;
      assert( isDxRelValid( u ) );
#if FAST_IR
      if ( !u->dx_relevant ) continue;
#else
      assert( u->dx_relevant );
#endif
      assert( (unsigned)(*it)->id < hAdjInc.size( ) );
      AdjList & adj_list = hAdjInc[ (*it)->id ];
      for ( AdjList::iterator aIt = adj_list.begin( )
	  ; aIt != adj_list.end( )
	  ; ++ aIt )
      {
	OEdge * candidate = *aIt;
#if OVERB
	cerr << "  Candidate edge: " << candidate << endl;
	cerr << "    Corr to     : " << candidate->c << endl;
	cerr << "    id          : " << candidate->id << endl;
#endif
	Enode * c = candidate->c;
	assert( candidate->v == u );
	OVertex * v = candidate->u;
#if OVERB
	cerr << "  Considering connected vertex: " << v->e << endl;
#endif
	// Already deduced edge
	if ( c->hasPolarity( ) || c->isDeduced( ) )
	  continue;
	// Can't deduce as vertexes unreachable
	if ( !isDyRelValid( v ) 
	  || !v->getRelevancy( O_sssp_backward ) )
	  continue;
#if OVERB
	cerr << "    has valid dy and is relevant" << endl;
#endif
	// If solid edge, deduce
	if ( candidate->solid )
	{
	  heavy_edges.push_back( candidate );
#if OVERB
	  cerr << "  DEDUCED: " << (candidate->id%2?"!":" ") << c << endl;
#endif
	}
	// If dashed edge, deduce only if at least one is unsafe
	else if ( isVertexUnsafeDx( u ) || isVertexUnsafeDy( v ) )
	{
	  heavy_edges.push_back( candidate );
#if OVERB
	  cerr << "  DEDUCED: " << (candidate->id%2?"!":" ") << c << endl;
#endif
	}
      }
    }
  }
}

//
// Shortest path computation
// if   direction = O_sssp_forward then forwardSSSP  ("to x")
// else                                 backwardSSSP ("from x")
//
void OGraph::findSSSP( OEdge * e, O_sssp_direction direction )
{
  OVertex * u = (direction == O_sssp_forward ? e->u : e->v );
  OVertex * v = (direction == O_sssp_forward ? e->v : e->u );

#if OVERB
  cerr << "FIND SSSP, dir " << (direction == O_sssp_forward 
                                           ? "forward" 
					   : "backward") 
                            << endl;
  cerr << "USING EDGE     " << e << endl;
  // cerr << "U              " << u->e << endl;
  // cerr << "V              " << v->e << endl;
#endif

  list< OVertex * > frontier;
  initInQueue( );

  // Paths from v are relevant
  v->setRelevancy( direction, true ); 
  // Paths from u are irrelevant
  u->setRelevancy( direction, false );
  if ( direction == O_sssp_forward )
  {
    updateDxRel( v );
    updateDxRel( u );
  }
  else
  {
    updateDyRel( v );
    updateDyRel( u );
  }
  // Add to frontier
  frontier.push_back( u );
  setInQueue( u );
  frontier.push_back( v );
  setInQueue( v );

  if ( !e->solid )
  {
    if ( direction == O_sssp_forward )
      setVertexUnsafeDx( v );
    else
      setVertexUnsafeDy( v );
  }

  while ( !frontier.empty( ) )
  {
    OVertex * s = frontier.back( );
#if OVERB
    // cerr << "  Popping: " << s->e << " rel " << s->getRelevancy( direction ) << endl;
#endif
    frontier.pop_back( );
    assert( isInQueue( s ) );
    unsetInQueue( s ); 
    if ( s->getRelevancy( direction ) )
      insertRelevantVertices( s, direction );
    else
      removeIrrelevantVertices( s, direction );
    //
    // For all outgoing edges
    //
    AdjList & adjList = direction == O_sssp_forward ? dAdj[ s->id ] : dAdjInc[ s->id ];
    for ( AdjList::iterator it = adjList.begin( )
	; it != adjList.end( )
	; ++it )
    {
      OEdge * k = *it;
      // Skip asserted edge
      if ( k == e ) continue;

      assert( direction != O_sssp_forward  || s == k->u ); 
      assert( direction != O_sssp_backward || s == k->v ); 

      OVertex * t = ( direction == O_sssp_forward ? k->v : k->u );

#if OVERB
      cerr << "  Considering edge: " << k << endl;
      cerr << "  Target vertex   : " << t->e << endl;
#endif

      const bool t_seen = direction == O_sssp_forward 
	                ? isDxRelValid( t ) 
		        : isDyRelValid( t );

      assert( direction == O_sssp_forward ? isDxRelValid( s ) : isDyRelValid( s ) );

      const bool t_unsafe = direction == O_sssp_forward ? isVertexUnsafeDx( t ) : isVertexUnsafeDy( t );
      const bool t_relev  = t->getRelevancy( direction );
      const bool s_unsafe = direction == O_sssp_forward ? isVertexUnsafeDx( s ) : isVertexUnsafeDy( s );
      const bool s_relev  = s->getRelevancy( direction );

      bool add_to_queue = false;
      //
      // OK, we need updating the target vertex of this 
      // edge. We have many cases to consider, which are
      // related to unsafeness and irrelevancy. 
      // Unsafeness propagates onver safeness, and 
      // irrelevancy propagates over relevancy. Also
      // we have to consider the solidness of the edge.
      //
      // Solid edge: s ----> t
      //
      // if t is unsafe irrelevant, do nothing
      // if t is unsafe relevant
      //   if s is unsafe irrelevant, t becomes unsafe irrelevant
      //   if s is unsafe relevant, do nothing
      //   if s is safe, do nothing
      // if t is safe irrelevant
      //   if s is unsafe irrelevant, t becomes unsafe irrelevant
      //   if s is unsafe relevant, t becomes unsafe relevant
      //   if s is safe irrelevant, do nothing
      //   if s is safe relevant, do nothing 
      // if t is safe relevant
      //   if s is unsafe irrelevant, t becomes unsafe irrelevant
      //   if s is unsafe relevant, t becomes unsafe relevant
      //   if s is safe irrelevant, t becomes safe irrelevant
      //   if s is safe relevant, do nothing 
      //
      if ( k->solid )
      {
	if ( !t_seen )
	{
	  if ( direction == O_sssp_forward )
	    updateDxRel( t );
	  else
	    updateDyRel( t );
	  // Same relevancy as s 
	  t->setRelevancy( direction, s_relev );
	  // Same safeness as s
	  if ( s_unsafe && direction == O_sssp_forward )
	    setVertexUnsafeDx( t );
	  if ( s_unsafe && direction == O_sssp_backward )
	    setVertexUnsafeDy( t );

	  add_to_queue = true;
	}
	else
	{
	  if ( t_unsafe )
	  {
	    if ( !t_relev )
	      ; // Do nothing
	    // t is irrelevant
	    else
	    { 
	      if ( s_unsafe && !s_relev )
	      {
		t->setRelevancy( direction, false );
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		add_to_queue = true;
	      }
	    }
	  }
	  else
	  {
	    // t is safe irrelevant
	    if ( !t_relev )
	    { 
	      if ( s_unsafe && !s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		add_to_queue = true;
	      }
	      else if ( s_unsafe && s_relev )
	      {
		// t becomes unsafe 
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes relevant
		t->setRelevancy( direction, true );
		add_to_queue = true;
	      }
	    }
	    // t is safe relevant
	    else
	    {
	      if ( s_unsafe && !s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes irrelevant
		t->setRelevancy( direction, false );
		add_to_queue = true;
	      }
	      else if ( s_unsafe && s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes relevant
		t->setRelevancy( direction, true );
		add_to_queue = true;
	      }
	      else if ( !s_unsafe && !s_relev )
	      {
		// t becomes irrelevant
		t->setRelevancy( direction, false );
		add_to_queue = true;
	      }
	    }
	  }
	}
      }
      //
      // Dashed edge: s - - > t
      //
      // if t is unsafe irrelevant, do nothing
      // if t is unsafe relevant
      //   if s is unsafe irrelevant, t becomes unsafe irrelevant
      //   if s is unsafe relevant, do nothing
      //   if s is safe, do nothing
      // if t is safe irrelevant
      //   if s is unsafe irrelevant, t becomes unsafe irrelevant
      //   if s is unsafe relevant, t becomes unsafe relevant
      //   if s is safe irrelevant, t becomes unsafe irrelevant
      //   if s is safe relevant, t becomes unsafe relevant
      // if t is safe relevant
      //   if s is unsafe irrelevant, t becomes unsafe irrelevant
      //   if s is unsafe relevant, t becomes unsafe relevant
      //   if s is safe irrelevant, t becomes unsafe irrelevant
      //   if s is safe relevant, t becomes unsafe relevant
      //
      else
      {
	if ( !t_seen )
	{
	  if ( direction == O_sssp_forward )
	    updateDxRel( t );
	  else
	    updateDyRel( t );
	  // Same relevancy as s 
	  t->setRelevancy( direction, s_relev );
	  // Set unsafe
	  if ( direction == O_sssp_forward )
	    setVertexUnsafeDx( t );
	  if ( direction == O_sssp_backward )
	    setVertexUnsafeDy( t );

	  add_to_queue = true;
	}
	else
	{
	  if ( t_unsafe )
	  {
	    if ( !t_relev )
	      ; // Do nothing
	    // t is irrelevant
	    else
	    { 
	      if ( s_unsafe && !s_relev )
	      {
		t->setRelevancy( direction, false );
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		add_to_queue = true;
	      }
	    }
	  }
	  else
	  {
	    // t is safe irrelevant
	    if ( !t_relev )
	    { 
	      if ( s_unsafe && !s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		add_to_queue = true;
	      }
	      else if ( s_unsafe && s_relev )
	      {
		// t becomes unsafe 
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes relevant
		t->setRelevancy( direction, true );
		add_to_queue = true;
	      }
	      else if ( !s_unsafe && !s_relev )
	      {
		// t becomes unsafe 
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes relevant
		t->setRelevancy( direction, false );
		add_to_queue = true;
	      }
	      else if ( !s_unsafe && s_relev )
	      {
		// t becomes unsafe 
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes relevant
		t->setRelevancy( direction, true );
		add_to_queue = true;
	      }
	    }
	    // t is safe relevant
	    else
	    {
	      if ( s_unsafe && !s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes irrelevant
		t->setRelevancy( direction, false );
		add_to_queue = true;
	      }
	      else if ( s_unsafe && s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes relevant
		t->setRelevancy( direction, true );
		add_to_queue = true;
	      }
	      else if ( !s_unsafe && !s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		// t becomes irrelevant
		t->setRelevancy( direction, false );
		add_to_queue = true;
	      }
	      else if ( !s_unsafe && s_relev )
	      {
		// t becomes unsafe
		if ( direction == O_sssp_forward )
		  setVertexUnsafeDx( t );
		else
		  setVertexUnsafeDy( t );
		add_to_queue = true;
	      }
	    }
	  }
	}
      }

      // Add it to the frontier if not there already
      if ( add_to_queue && !isInQueue( t ) )
      {
#if UNSAFE_FIRST
        if ( direction == O_sssp_forward && isVertexUnsafeDx( t ) )
	  frontier.push_back( t );
	else if ( direction == O_sssp_backward && isVertexUnsafeDy( t ) )
	  frontier.push_back( t );
	else
	  frontier.push_front( t );
#elif DFS
	frontier.push_back( t );
	assert( frontier.empty( ) 
	     || isDxRelValid( frontier.back( ) )
	     || isDyRelValid( frontier.back( ) ) );
#else
	frontier.push_front( t );
	assert( frontier.empty( ) 
	     || isDxRelValid( frontier.front( ) )
	     || isDyRelValid( frontier.front( ) ) );
#endif
	setInQueue( t );
      }
    }
  }

  doneInQueue( );
}

//
// Print adjacency list
//
void OGraph::printAdj(vector<AdjList> & adj)
{
   vector<AdjList>::iterator it;
  int i = 0;
  for(it = adj.begin(); it != adj.end(); ++it, ++i)
  {
    cerr << "Vertex " << i << " ====> ";
    printAdjList(*it);
    cerr << " " << endl;
  }
}

void OGraph::printAdjList(AdjList & adjList)
{
   AdjList::iterator it;
  for (it = adjList.begin(); it != adjList.end(); ++it)
    cerr << *it << "  ";
}

void OGraph::printDynGraphAsDotty( const char * filename, OEdge *e )
{
  ofstream out( filename );
  out << "DiGraph dump {" << endl;

  for (  vector< OVertex * >::iterator it = vertices.begin( )
      ; it != vertices.end( )
      ; ++ it )
  {
    AdjList & adjList = dAdj[(*it)->id];
     AdjList::iterator jt;
    for (jt = adjList.begin(); jt != adjList.end(); ++jt)
    {
      if ( (*jt) == e )
	printPlainEdge( out, *jt, "[color=red];" );
      else
	printPlainEdge( out, *jt );

    }
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}

/*
void OGraph::printSSSPAsDotty( const char * filename, OVertex * u , O_sssp_direction direction )
{
  ofstream out( filename );
  out << "DiGraph dump {" << endl;
  out << "\"" << u->e << " | " << u->getDist( direction ) << "\"" << " [color=red];" << endl;

  for (  vector< OVertex * >::iterator it = vertices.begin( )
      ; it != vertices.end( )
      ; ++ it )
  {
    AdjList & adjList = dAdj[(*it)->id];
     AdjList::iterator jt;
    for (jt = adjList.begin(); jt != adjList.end(); ++jt)
      printSSSPEdge( out, *jt, direction );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}
*/

void OGraph::printInactiveAsDotty( const char * filename )
{
  ofstream out ( filename );
  out << "DiGraph dump { " << endl;
   vector< OEdge * >::iterator it;
  for ( it = hEdges.begin( ); it != hEdges.end( ); ++ it)
  {
    const bool u_is_relevant = isDyRelValid( (*it)->u ) && (*it)->u->dy_relevant;
    const bool v_is_relevant = isDxRelValid( (*it)->v ) && (*it)->v->dx_relevant;
    string attrib = (u_is_relevant && v_is_relevant) ? " [color=red]; " : " ;";

    printDistEdge( out, *it, attrib );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;

}

void OGraph::printDeducedAsDotty( const char * filename )
{
  ofstream out( filename );
  out << "DiGraph dump {" << endl;

  for (  vector< OEdge * >::iterator it = heavy_edges.begin( )
      ; it != heavy_edges.end( ); ++ it)
  {
    string attrib = " [color=green]; ";
    printDistEdge( out, *it, attrib );
  }

  for (  vector< OVertex * >::iterator it = vertices.begin( )
      ; it != vertices.end( )
      ; ++ it )
  {
    AdjList & adjList = dAdj[(*it)->id];
     AdjList::iterator jt;
    for (jt = adjList.begin(); jt != adjList.end(); ++jt)
      printDistEdge( out, *jt );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}

/*
void OGraph::printShortestPath( OEdge * e, const char * filename )
{
  assert( e );
  OPath & shortest_path = getShortestPath( e );
  ofstream out( filename );
  out << "DiGraph sp {" << endl;

  printDistEdge( out, e, "[color=red];" );

  for ( OPath::iterator it = shortest_path.begin( ); it != shortest_path.end( ); ++ it )
  {
    printDistEdge( out, *it );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}
*/

void OGraph::printOPath( OPath path, const char * filename )
{
  //assert( path );
  ofstream out( filename );

  out << "DiGraph sp {" << endl;

  for ( OPath::iterator it = path.begin( ); it != path.end( ); ++ it )
  {
    printDistEdge( out, *it );
  }

  out << "}" << endl;
  out.close( );

  cerr << "[ Wrote: " << filename << " ]" << endl;
}
