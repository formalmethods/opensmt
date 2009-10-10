/*********************************************************************
Author: Edgar Pek <edgar.pek@lu.unisi.ch>
      , Roberto Bruttomesso <roberto.bruttomesso@unisi.ch> 

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

// Graph functions 
#ifndef OGRAPH_H
#define OGRAPH_H

#include "Enode.h"
#include "Egraph.h"
#include "SMTConfig.h"

#define OVERB 0
#define USE_VEC 1
#define FAST_IR 1

enum O_sssp_direction { O_sssp_forward, O_sssp_backward };

// FIXME: 
// . constructor first
// . use reference when passing Numbers 
// . input parameters in constructor should have different names than class attributes
struct OVertex 
{
  OVertex ( Enode * e_
          , int     id_ )
    : e             ( e_ )
    , id            ( id_ )
    , dx            ( -1 )
    , dy            ( -1 )
    , dx_relevant   ( false )
    , dy_relevant   ( false )
    , dyn_outdegree ( 0 )
    , dyn_indegree  ( 0 )
    , h_outdegree   ( 0 ) 
    , h_indegree    ( 0 )
  { }

  OVertex ( OVertex * v ) 
    : e		   ( v->e )
    , id	   ( v->id)
    , dx	   ( v->dx )
    , dy           ( v->dy )
    , dx_relevant  ( v->dx_relevant )
    , dy_relevant  ( v->dy_relevant )
    , dyn_outdegree( v->dyn_outdegree )
    , dyn_indegree ( v->dyn_indegree ) 
    , h_outdegree  ( v->h_outdegree )
    , h_indegree   ( v->h_indegree ) 
  { }

  inline bool getRelevancy( O_sssp_direction dir           ) { return dir == O_sssp_forward ? dx_relevant : dy_relevant; }
  inline void setRelevancy( O_sssp_direction dir, bool val ) { if   ( dir == O_sssp_forward ) dx_relevant = val; else dy_relevant = val; }

  Enode *     e;
  const int   id;
  int	      dx;
  int	      dy;
  bool        dx_relevant;
  bool        dy_relevant;
  size_t      dyn_outdegree;
  size_t      dyn_indegree;
  size_t      h_outdegree;
  size_t      h_indegree;
};

struct OEdge 
{
  OEdge( Enode    * c_
	, int       id_
        , OVertex * u_
	, OVertex * v_
	, bool      solid_ )
      : c     ( c_ )
      , id    ( id_ )
      , u     ( u_ )
      , v     ( v_ )
      , solid ( solid_ ) 
      , rwt   ( -1 )
      , r     ( NULL )
  { }

  inline void printOEdge ( ostream & os ) 
  { 
    if ( u->e == NULL )
      os << "ZERO";
    else
      os << u->e;
    os << (solid ? " ----> " : " - - > ");
    if ( v->e == NULL )
      os << "ZERO";
    else
      os << v->e;
    os << " " << id;
  }
  inline friend ostream & operator<< ( ostream & os, OEdge * e ) { assert( e ); e->printOEdge( os ); return os; }

  Enode *    c;      // reference to the original Enode
  const int  id;     // id of the edge
  OVertex *  u;      // the source vertex
  OVertex *  v;      // the destination vertex
  const bool solid;  // original cost of the edges
  int        rwt;    // reduced  cost of the edge (nonnegative edge weight)
  OEdge *    r;      // the edge that caused deduction TODO: check if needed!
};

struct OComplEdges
{
  OComplEdges ( OEdge * pos_
	      , OEdge * neg_ ) 
	     : pos ( pos_ )
	     , neg ( neg_ )  
  { }

  OEdge * pos;
  OEdge * neg;
};

struct OTwoVertices
{
  OTwoVertices ( OVertex * u_
	       , OVertex * v_ )
	       : u ( u_ )
	       , v ( v_ )
  { }

  struct vertexIdGreaterThan
  {
    inline bool operator ( ) ( const OTwoVertices & x, const OTwoVertices & y)
    {
      return x.u->id > y.u->id;
    }

  };

  OVertex * u;
  OVertex * v;
};

namespace __gnu_cxx
{
  // Hash function for OEdge *
  template< typename DLEdge >
  struct SizeTOEdge
  {
    size_t operator ( ) ( const OEdge & e ) const 
    {
      return (size_t) e;
    }
  };
}

class OGraph
{
public:

    OGraph( SMTConfig & config_, Egraph & egraph_ ) 
      : Vcnt		        ( 0 ) 
      , Ecnt		        ( 0 )
      , active_dx_rel           ( false )
      , dx_rel_count            ( 0 )
      , active_dy_rel           ( false )
      , dy_rel_count            ( 0 )				  
      , active_in_queue         ( false )
      , in_queue_count          ( 0 )
      , active_seen             ( false )
      , seen_count              ( 0 )
      , active_vertex_unsafe    ( false )
      , vertex_unsafe_count     ( 0 )
      , active_vertex_unsafe_dx ( false )
      , vertex_unsafe_count_dx  ( 0 )
      , active_vertex_unsafe_dy ( false )
      , vertex_unsafe_count_dy  ( 0 )
      , active_dfs_finished     ( false )
      , dfs_finished_count      ( 0 )
      , after_backtrack         ( false )
      , config                  ( config_ )
      , egraph                  ( egraph_ )
  { } 

   ~OGraph( );

  typedef vector< OEdge * > AdjList;

  typedef __gnu_cxx::SizeTOEdge< const OEdge * > HashOEdge;
  typedef vector< OEdge * >            OPath;

  typedef map< Enode *, OVertex * >    Enode2Vertex;
  typedef map< Enode *, OComplEdges >  Enode2Edge;

  inline unsigned getVcnt ( ) const { return Vcnt; }
  inline unsigned getEcnt ( ) const { return Ecnt; }

  inline Enode2Edge &        getEdgeMap          ( ) { return edgeMap; }
  inline vector< AdjList > & getDAdj             ( ) { return dAdj; }
  inline vector< OEdge * > & getConflictEdges    ( ) { return conflict_edges; }
  inline OVertex *           getInfeasibleVertex ( ) { return infeasibleVertex; }
  inline double              getynGraphDensity   ( ) { return (double) dEdges.size( ) / (double) Vcnt; }

  inline void deleteFromAdjList( AdjList & adj_list, OEdge * e)
  { 
    assert( e ); 
    AdjList::iterator it = find( adj_list.begin( ), adj_list.end( ), e );
    if ( it != adj_list.end( ))
      adj_list.erase( it );
  }

  void    insertStatic  ( Enode * );
  OEdge * insertDynamic ( Enode *, bool reason );
  void	  insertImplied ( Enode * );
  void	  insertInactive( Enode * );

  void	  deleteActive  ( Enode * );
  void	  deleteInactive( Enode * );

  inline OEdge * getOppositePolarityEdge( Enode * c ) { assert( c->hasPolarity( ) );  assert( edgeMap.find( c ) != edgeMap.end( ) ); OComplEdges edges = edgeMap.find( c )->second; OEdge * e = c->getPolarity( ) == l_True ? edges.neg : edges.pos;  assert( e ); return e;  }
  inline OEdge * getEdgeWithPolarity    ( Enode * c ) { assert( c->hasPolarity( ) );  assert( edgeMap.find( c ) != edgeMap.end( ) ); OComplEdges edges = edgeMap.find( c )->second; OEdge * e = c->getPolarity( ) == l_True ? edges.pos : edges.neg;  assert( e ); return e;  }

  inline void updateDynDegree( OEdge * e )
  {
    e->u->dyn_outdegree = dAdj[    e->u->id ].size( );
    e->v->dyn_indegree  = dAdjInc[ e->v->id ].size( );
  }

  inline void updateHDegree( OEdge * e )
  {
    e->u->h_outdegree = hAdj[    e->u->id ].size( );
    e->v->h_indegree  = hAdjInc[ e->v->id ].size( );
  }

  bool        checkInfeasibleCycle ( Enode *, bool );
  void	      findHeavyEdges       ( Enode * );
  void	      iterateInactive      ( );

  inline bool isParallelAndHeavy( OEdge * e )
  {
    // Check if there is a parallel edge of smaller weight - if yes: return
    AdjList & adj_list_x = dAdj[ e->u->id ];
    for ( AdjList::iterator it = adj_list_x.begin( ); it != adj_list_x.end( ); ++ it )
      if ( ( (*it)->v->id == e->v->id ) && !(*it)->solid )
	return true;
    return false;
  }

  void        printAdj             ( vector<AdjList> & );
  void        printAdjList         ( AdjList & );
  
  inline void printStatAdj         ( ) { printAdj(sAdj); }
  inline void printDynAdj          ( ) { printAdj(dAdj); }

  vector< OVertex * > vertices;
  vector< OEdge * >   heavy_edges;                 // TODO: deal with it when backtracking!

  vector< Enode *  >  undo_stack_inactive_enodes;  //  stack of inactive edges
  vector< OEdge * >   undo_stack_deduced_edges;    //  stack of deduced edges

  void printShortestPath ( OEdge *, const char * );
  void printOPath       ( OPath, const char * );

private:

  inline OVertex * getOVertex( Enode * x )   
  {	
    if ( vertexMap.find( x ) == vertexMap.end( ) )
    {
      OVertex * v = new OVertex( x, vertexMap.size( ) );
      vertexMap.insert( pair< Enode *, OVertex *> (x, v) );	
      vertices.resize( vertexMap.size( ) ); vertices[v->id] = v;
      return v;
    }

    return vertexMap.find( x )->second;
  }

  OComplEdges getOEdge( Enode * );
  //
  // Check if dx-relevancy initialized
  //
  inline void initDxRel    ( )             { assert ( !active_dx_rel ); active_dx_rel = true; dx_rels.resize( Vcnt, dx_rel_count ); ++ dx_rel_count; }
  inline void updateDxRel  ( OVertex * v ) { assert (  active_dx_rel ); assert( v->id < (int)dx_rels.size( ) ); dx_rels[ v->id ] = dx_rel_count; }
  inline bool isDxRelValid ( OVertex * v ) { assert (  active_dx_rel ); assert( v->id < (int)dx_rels.size( ) ); return dx_rels[ v->id ] == dx_rel_count; }
  inline void doneDxRel    ( )             { assert (  active_dx_rel ); active_dx_rel = false; }
  //
  // Check if dy-relevancy initialized
  //
  inline void initDyRel    ( )             { assert ( !active_dy_rel ); active_dy_rel = true; dy_rels.resize( Vcnt, dy_rel_count ); ++ dy_rel_count; }
  inline void updateDyRel  ( OVertex * v ) { assert (  active_dy_rel ); assert( v->id < (int)dy_rels.size( ) ); dy_rels[ v->id ] = dy_rel_count; }
  inline bool isDyRelValid ( OVertex * v ) { assert (  active_dy_rel ); assert( v->id < (int)dy_rels.size( ) ); return dy_rels[ v->id ] == dy_rel_count; }
  inline void doneDyRel    ( )             { assert (  active_dy_rel ); active_dy_rel = false; }
  //
  // Fast safe/unsafe check for SSSP
  //
  inline void initVertexUnsafeDx ( )             { assert ( !active_vertex_unsafe_dx ); active_vertex_unsafe_dx = true; vertex_unsafe_dx.resize( Vcnt, vertex_unsafe_count_dx ); ++ vertex_unsafe_count_dx; }
  inline void setVertexUnsafeDx  ( OVertex * v ) { assert (  active_vertex_unsafe_dx ); assert( v->id < (int) vertex_unsafe_dx.size( ) ); vertex_unsafe_dx[ v->id ] = vertex_unsafe_count_dx; }
  inline bool isVertexUnsafeDx   ( OVertex * v ) { assert (  active_vertex_unsafe_dx ); assert( v->id < (int) vertex_unsafe_dx.size( ) ); return vertex_unsafe_dx[ v->id ] == vertex_unsafe_count_dx; }
  inline void doneVertexUnsafeDx ( )	         { assert (  active_vertex_unsafe_dx ); active_vertex_unsafe_dx = false; }
  //
  // Fast safe/unsafe check for SSSP
  //
  inline void initVertexUnsafeDy ( )             { assert ( !active_vertex_unsafe_dy ); active_vertex_unsafe_dy = true; vertex_unsafe_dy.resize( Vcnt, vertex_unsafe_count_dy ); ++ vertex_unsafe_count_dy; }
  inline void setVertexUnsafeDy  ( OVertex * v ) { assert (  active_vertex_unsafe_dy ); assert( v->id < (int) vertex_unsafe_dy.size( ) ); vertex_unsafe_dy[ v->id ] = vertex_unsafe_count_dy; }
  inline bool isVertexUnsafeDy   ( OVertex * v ) { assert (  active_vertex_unsafe_dy ); assert( v->id < (int) vertex_unsafe_dy.size( ) ); return vertex_unsafe_dy[ v->id ] == vertex_unsafe_count_dy; }
  inline void doneVertexUnsafeDy ( )	         { assert (  active_vertex_unsafe_dy ); active_vertex_unsafe_dy = false; }
  //
  // Fast is-in-queue check. Cannot be nested.
  //
  inline void initInQueue ( )             { assert ( !active_in_queue ); active_in_queue = true; in_queue.resize( Vcnt, in_queue_count ); ++ in_queue_count; }
  inline void setInQueue  ( OVertex * v ) { assert (  active_in_queue ); assert( v->id < (int) in_queue.size( ) ); in_queue[ v->id ] = in_queue_count; }
  inline void unsetInQueue( OVertex * v ) { assert (  active_in_queue ); assert( v->id < (int) in_queue.size( ) ); in_queue[ v->id ] = in_queue_count-1; }
  inline bool isInQueue   ( OVertex * v ) { assert (  active_in_queue ); assert( v->id < (int) in_queue.size( ) ); return in_queue[ v->id ] == in_queue_count; }
  inline void doneInQueue ( )	          { assert (  active_in_queue ); active_in_queue = false; }
  //
  // Fast is-in-queue check. Cannot be nested.
  //
  inline void initSeen ( )             { assert ( !active_seen ); active_seen = true; seen.resize( Vcnt, seen_count ); ++ seen_count; }
  inline void setSeen  ( OVertex * v ) { assert (  active_seen ); assert( v->id < (int) seen.size( ) ); seen[ v->id ] = seen_count; }
  inline bool isSeen   ( OVertex * v ) { assert (  active_seen ); assert( v->id < (int) seen.size( ) ); return seen[ v->id ] == seen_count; }
  inline void doneSeen ( )	       { assert (  active_seen ); active_seen = false; }
  //
  // Fast safe/unsafe check
  //
  inline void initVertexUnsafe ( )             { assert ( !active_vertex_unsafe ); active_vertex_unsafe = true; vertex_unsafe.resize( Vcnt, vertex_unsafe_count ); ++ vertex_unsafe_count; }
  inline void setVertexUnsafe  ( OVertex * v ) { assert (  active_vertex_unsafe ); assert( v->id < (int) vertex_unsafe.size( ) ); vertex_unsafe[ v->id ] = vertex_unsafe_count; }
  inline bool isVertexUnsafe   ( OVertex * v ) { assert (  active_vertex_unsafe ); assert( v->id < (int) vertex_unsafe.size( ) ); return vertex_unsafe[ v->id ] == vertex_unsafe_count; }
  inline void doneVertexUnsafe ( )	       { assert (  active_vertex_unsafe ); active_vertex_unsafe = false; }
  //
  // Dotty pretty print
  //
  void printDynGraphAsDotty( const char *, OEdge *e = NULL );
  void printSSSPAsDotty    ( const char * , OVertex * , O_sssp_direction );

  inline void printDistEdge( ofstream & out, OEdge * e, string attrib = ";" )
  {
    assert( out );
    assert( e   );
    out << "\"" << e->u->e << " | " << e->u->dy << "\"" 
      << " -> " 
      << "\"" << e->v->e << " | " << e->v->dx << "\"" 
      << " [label=" << (e->solid ? "solid" : "dashed" ) << "] " << attrib << endl;
  }

  inline void printPlainEdge( ofstream & out, OEdge * e, string attrib = ";" )
  {
    assert( out );
    assert( e   );
    out << "\"" << e->u->e << " | " << e->u->id << "\"" 
        << " -> " 
        << "\"" << e->v->e << " | " << e->v->id <<  "\"" 
        << " [label=" << (e->solid ? "solid" : "dashed" ) << "] " << attrib << endl;
  }

  void printInactiveAsDotty ( const char * );
  void printDeducedAsDotty  (  const char * );

  void printAdjMatrix       ( const char * );
  void printAPSPMatrix      ( const char * );
  //
  //  SSSP computation
  //
  void findSSSP             ( OEdge *, O_sssp_direction );

  inline void insertRelevantVertices( OVertex * v, O_sssp_direction dir )
  { 
    if( dir == O_sssp_forward )
    {
      assert( isDxRelValid( v ) );
      assert( v->dx_relevant );
#if FAST_IR
      dx_relevant_vertices.push_back( v );
#else
      dx_relevant_vertices.insert( v );
#endif
      total_in_deg_dx_rel += hAdjInc[ v->id ].size( );
    }
    else
    {
      assert( isDyRelValid( v ) );
      assert( v->dy_relevant );
#if FAST_IR
      dy_relevant_vertices.push_back( v );  
#else
      dy_relevant_vertices.insert( v );  
#endif
      total_out_deg_dy_rel += hAdj[ v->id ].size( );
    }
  }

#if FAST_IR
  inline void removeIrrelevantVertices( OVertex * v, O_sssp_direction dir )
  {
    if( dir == O_sssp_forward )
    {
      assert( isDxRelValid( v ) );
      if( !v->dx_relevant )
	total_in_deg_dx_rel -= hAdjInc[ v->id ].size( );
    }
    else
    {
      assert( isDyRelValid( v ) );
      if( !v->dy_relevant )
	total_out_deg_dy_rel -= hAdj[ v->id ].size( );
    }
  }
#else
  inline void removeIrrelevantVertices( OVertex * v, O_sssp_direction dir )
  {
    if( dir == O_sssp_forward )
    {
      if ( dx_relevant_vertices.erase( v ) != 0 )
      {
	assert( isDxRelValid( v ) );
	assert( !v->dx_relevant );
	total_in_deg_dx_rel -= hAdjInc[ v->id ].size( );
      }
    }
    else
    {
      if ( dy_relevant_vertices.erase( v ) != 0 )  
      {
	assert( isDyRelValid( v ) );
	assert( !v->dy_relevant );
	total_out_deg_dy_rel -= hAdj[ v->id ].size( );
      }
    }
  }
#endif

  //
  // Find shortest path for an edge
  //
  bool findShortestPath( OEdge * );

#if USE_VEC
  vector< OVertex * > unsafe_frontier;
  vector< OVertex * > safe_frontier;
#endif
  
  unsigned Vcnt;
  unsigned Ecnt;
	
  bool          active_dx_rel;	            // To prevent nested usage
  vector< int > dx_rels;                    // Fast check if dx is relevant
  int           dx_rel_count;               // DeltaX relevancy token
                                            
  bool          active_dy_rel;              // To prevent nested usage
  vector< int > dy_rels;                    // Fast check if dy is relevant
  int           dy_rel_count;               // DeltaY relevancy token
                                            
  bool		active_in_queue;            // To prevent nested usage
  vector< int > in_queue;	            // Fast check if a vertex is in_queue during dfs
  int		in_queue_count;             // fast is-in-queue check

  bool		active_seen;                // To prevent nested usage
  vector< int > seen;	                    // Fast check if a vertex has been seen already
  int		seen_count;                 // fast already visited check
                                            
  bool		active_vertex_unsafe;       // To prevent nested usage
  vector< int > vertex_unsafe;	            // Fast check if a vertex is unsafe
  int		vertex_unsafe_count;        // vertex unsafe token
                                            
  bool		active_vertex_unsafe_dx;    // To prevent nested usage
  vector< int > vertex_unsafe_dx;	    // Fast check if a vertex is unsafe
  int		vertex_unsafe_count_dx;     // vertex unsafe token
                                            
  bool		active_vertex_unsafe_dy;    // To prevent nested usage
  vector< int > vertex_unsafe_dy;	    // Fast check if a vertex is unsafe
  int		vertex_unsafe_count_dy;     // vertex unsafe token
                                            
  bool		active_dfs_finished;        // To prevent nested usage
  vector< int > dfs_finished;	            // Fast check if processing of vertex is finished
  int		dfs_finished_count;         // dfs finished count
                                            
  Enode2Vertex      vertexMap;              
  Enode2Edge        edgeMap;                
  vector< AdjList > sAdj;	            // adjacency list - static constraint graph
  vector< OEdge * > sEdges;                 // edges - static constraint graph
                                            
  vector< AdjList > dAdj;                   // adjacency list - dynamic constraint graph
  vector< AdjList > dAdjInc;                // incoming edges adjacency list  - dynamic constraint graph
  vector< OEdge * > dEdges;                 // edges - dynamic constraint graph
                                            
  vector< AdjList > iAdj;                   // adjacency list - implied graph
  vector< OEdge * > iEdges;                 // edges - implied graph
                                            
  vector< AdjList > hAdj;                   
  vector< AdjList > hAdjInc;                
  vector< OEdge * > hEdges;	            // the vector of inactive edges :  d - s
                                            
  vector< OEdge * > conflict_edges;         // used to explain a conflict
  OVertex *         infeasibleVertex;       // vertex from which the negative cycle starts (RB, not sure)

  vector< OVertex * > vertex_heap;
  vector< OVertex * > changed_vertices;
  // 
  // Data structures used in SSSP computations
  //
#if FAST_IR
  vector< OVertex * > dx_relevant_vertices; // Set of relevant vertices from x
  vector< OVertex * > dy_relevant_vertices; // Set of relevant vertices from y
#else
  set< OVertex * > dx_relevant_vertices; // Set of relevant vertices from x
  set< OVertex * > dy_relevant_vertices; // Set of relevant vertices from y
#endif

  unsigned total_in_deg_dx_rel;
  unsigned total_out_deg_dy_rel;

  bool	   after_backtrack;

  SMTConfig & config;
  Egraph &    egraph;
};

#endif
