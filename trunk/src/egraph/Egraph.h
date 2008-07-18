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

#ifndef EGRAPH_H
#define EGRAPH_H

#include "Enode.h"
#include "TSolver.h"

class Egraph : public TSolver
{
public:

  Egraph( SMTConfig & c )
      : TSolver           ( -1, "Core Solver", c )
      , enil              ( new Enode )
      , enable_undo       ( false )
      , time_stamp        ( 0 )
      , theoryInitialized ( false )
  {
    //
    // Initialize Egraph-Based Storage for terms
    //
    initializeStore( );
  }

  ~Egraph( )
  {
    //
    // Delete Ordinary Theory Solvers
    //
    while ( !tsolvers.empty( ) )
    {
      delete tsolvers.back( );
      tsolvers.pop_back( );
    } 
    //
    // Delete enodes
    //
    while ( !id_to_enode.empty( ) )
    {
      assert( id_to_enode.back( ) );
      delete id_to_enode.back( );
      id_to_enode.pop_back( );
    }
    // 
    // Delete constants
    //
    delete enil;
  }

  //
  // Predefined constants nil, true, false
  // TODO: turn etrue efalse into constants
  //
  const Enode * const enil;
  Enode * etrue;
  Enode * efalse;

  //===========================================================================
  // Public APIs for enode construction/destruction

  void    newSort              ( const char * );                                   // Inserts a new uninterpreted sort
  int     getSort              ( const char * );                                   // From name to sort id
  Enode * newSymbol            ( const char *, const char, const int );            // Inserts a new symbol
  Enode * cons                 ( Enode *, Enode * );                               // Create Lists/Terms
  Enode * cons                 ( Enode * e ) { return cons( e, (Enode *)enil ); }  // Shortcut for singleton
  Enode * uCons                ( Enode *, Enode * );                               // Create Lists/Terms
  Enode * uCons                ( Enode * e ) { return uCons( e, (Enode *)enil ); } // Shortcut for singleton
  void    undoCons             ( Enode * );                                        // Undoes a Cons
  
  // TODO: use only one operator between mkLeq and mkGeq, etc ...
  inline Enode * mkLeq         ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_LEQ ]   , args ); }
  inline Enode * mkGeq         ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_GEQ ]   , args ); }
  inline Enode * mkLt          ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_LT ]    , args ); }
  inline Enode * mkGt          ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_GT ]    , args ); }
  inline Enode * mkPlus        ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_PLUS ]  , args ); }
  inline Enode * mkMinus       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_MINUS ] , args ); }
  inline Enode * mkTimes       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_TIMES ] , args ); }
  inline Enode * mkDiv         ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_DIV ]   , args ); }
  inline Enode * mkUminus      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_UMINUS ], args ); }
  
  inline Enode * mkTrue        ( )              { return etrue; }  
  inline Enode * mkFalse       ( )              { return efalse; } 
  
  inline Enode * mkBvand       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVAND ] , args ); }
  inline Enode * mkBvor        ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVOR ]  , args ); }
  inline Enode * mkBvadd       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVADD ] , args ); }
  inline Enode * mkBvslt       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSLT ] , args ); }
  inline Enode * mkBvsgt       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSGT ] , args ); }
  inline Enode * mkBvsleq      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSLEQ ], args ); }
  inline Enode * mkBvsgeq      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSGEQ ], args ); }
  inline Enode * mkBvult       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVULT ] , args ); }
  inline Enode * mkBvugt       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVUGT ] , args ); }
  inline Enode * mkBvuleq      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVULEQ ], args ); }
  inline Enode * mkBvugeq      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVUGEQ ], args ); }
  inline Enode * mkBvxor       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVXOR ] , args ); }
  inline Enode * mkBvsub       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSUB ] , args ); }
  inline Enode * mkBvmul       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVMUL ] , args ); }
  inline Enode * mkBvlshr      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVLSHR ], args ); }
  inline Enode * mkBvshl       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSHL ] , args ); }
  inline Enode * mkBvsrem      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSREM ], args ); }
  inline Enode * mkBvurem      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVUREM ], args ); }
  inline Enode * mkBvsdiv      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVSDIV ], args ); }
  inline Enode * mkBvudiv      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVUDIV ], args ); }
  inline Enode * mkBvnot       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVNOT ] , args ); }
  inline Enode * mkBvneg       ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVNEG ] , args ); }

  // Implemented in EgraphStore.C
  Enode * mkConcat           ( Enode * );
  Enode * mkDistinct         ( Enode * );
  Enode * mkNot              ( Enode * );
  Enode * mkAnd              ( Enode * );
  Enode * mkOr               ( Enode * );
  Enode * mkIte              ( Enode *, Enode *, Enode * );
  Enode * mkIfthenelse       ( Enode *, Enode *, Enode * );
  Enode * mkEq               ( Enode * );
  Enode * mkSignExtend       ( int, Enode * );
  Enode * mkZeroExtend       ( int, Enode * );
  Enode * mkExtract          ( int, int, Enode * );
  Enode * mkImplies          ( Enode * );
  Enode * mkIff              ( Enode * );
  Enode * mkXor              ( Enode * );
  Enode * mkBvnum            ( char * );
  Enode * allocTrue          ( );
  Enode * allocFalse         ( );
                             
  Enode * mkVar              ( const char * );
  Enode * mkNum              ( const char * );
  Enode * mkNum              ( const char *, const char * );
  Enode * mkNum              ( const Real & );
  Enode * mkUf               ( const char *, Enode * );
  Enode * mkUp               ( const char *, Enode * );

  void    mkDefine           ( const char *, Enode * );
  Enode * getDefine          ( const char * );

  Enode * getFormula         ( );
  void    setDistinctEnodes  ( vector< Enode * > & );

  void    printEnodeCong     ( ostream & );
  void    printEnodeList     ( ostream & );
#ifdef PEDANTIC_DEBUG
  bool    checkUFInvariants  ( );
#endif

  inline void setTopEnode    ( Enode * e ) { assert( e ); top = e; }
  inline void addAssumption  ( Enode * e ) { assert( e ); assumptions.push_back( e ); }
  inline void enableUndo     ( )           { enable_undo = true; }
  inline void disableUndo    ( )           { enable_undo = false; }

  inline size_t nofEnodes    ( ) { return id_to_enode.size( ); }

  inline Enode * indexToDist ( unsigned index ) const
  { 
    assert( index < index_to_dist.size( ) ); 
    return index_to_dist[ index ]; 
  }

  Enode * copyEnodeEtypeListWithCache   ( Enode *, Map( int, Enode * ) & );
  Enode * copyEnodeEtypeList            ( Enode * );

  void    printStatistics               ( ostream & );

  //===========================================================================
  // Public APIs for Egraph Core Solver

  void		      initializeTheorySolvers ( );         // Attaches ordinary theory solvers
  void                inform                  ( Enode * ); // Inform the solver about the existence of a theory atom
  bool                assertLit               ( Enode * ); // Assert a theory literal
  void                pushBacktrackPoint      ( );         // Push a backtrack point
  void                popBacktrackPoint       ( );         // Backtrack to last saved point
  void                popUntilDeduced         ( Enode * ); // Backtrack to a state that deduced the enode
  Enode *             getDeduction            ( );         // Return an implied node based on the current state
  vector< Enode * > & getConflict             ( );         // Return a reference to the explanation
  vector< Enode * > & getReason               ( Enode * ); // Explain why Lit is deduced. Store result in explanation
  bool                check                   ( bool );    // Check satisfiability

private:
  
  //===========================================================================
  // Private Routines for enode construction/destruction

  // Defines the set of operations that can be performed 
  typedef enum { SYMB, CONS, MERGE, DISEQ, DIST, SPLIT, CBE } oper_t;

  void    initializeStore      ( );                     // Initializes store
  //
  // Related to term creation
  //
  Enode * insertNumber ( Enode * );                     // Inserts a number
  void    insertSymbol ( Enode * );                     // Inserts a symbol
  void    removeSymbol ( Enode * );                     // Remove a symbol
  Enode * lookupSymbol ( const char * name );           // Retrieve a symbol
  void    insertDefine ( const char *, Enode * );       // Insert a define
  Enode * lookupDefine ( const char * );                // Retrieve a define
  Enode * insertStore  ( const int, Enode *, Enode * ); // Insert node into the global store

  MapNameInt   name_to_extrasort;                       // Store for extrasorts
                                                        
  MapNameEnode name_to_number;                          // Store for numbers
  MapNameEnode name_to_symbol;                          // Store for symbols
  MapNameEnode name_to_define;                          // Store for defines
  MapPairEnode store;                                   // Store for terms and lists
  //
  // Related to congruence closure
  //
  Enode * insertSigTab ( const int, Enode *, Enode * ); // For undoable cons only
  Enode * insertSigTab ( Enode * );                     // For for terms that go in the congruence
  Enode * lookupSigTab ( Enode * );                     // Retrieve Enode
  void    removeSigTab ( Enode * );                     // Remove Enode from sig_tab

  MapPairEnode       sig_tab_term;                      // Signature table for terms
  MapPairEnode       sig_tab_list;                      // Signature table for lists
  //
  // Generic attributes
  //
  vector< Enode * >  id_to_enode;                       // Table ENODE_ID --> ENODE
  vector< Enode * >  index_to_dist;                     // Table distinction index --> enode
  vector< Enode * >  assumptions;                       // List of assumptions
  Enode *            top;                               // Top node of the formula
  vector< size_t >   enode_id_to_deduced;               // Enode ID --> stack size when deduced  
  
  //===========================================================================
  // Private Routines for Core Theory Solver

  //
  // Initialization
  //
  void    initializeCong          ( Enode * );       // Initialize Congruence
  //
  // Asserting literals
  //
  bool    assertEq        ( Enode *, Enode *, Enode * );
  bool    assertNEq       ( Enode *, Enode *, Enode * );
  bool    assertDist      ( Enode * );
  bool    assertBvEq      ( Enode *, Enode *, Enode * );
  //
  // Backtracking
  //
  void backtrackToStackSize ( size_t size );                  // Backtrack
  //
  // Congruence closure main routines
  //
  bool    unmergable      ( Enode *, Enode *, Enode ** );
  void    merge           ( Enode *, Enode * );
  void	  deduce          ( Enode *, Enode * ); 
  void    undoMerge       ( Enode * );
  void    undoDisequality ( Enode * );
  void    undoDistinction ( Enode * );
  //
  // Explanation routines
  //
  void    expExplain           ( );                           // Main routine for explanation
  void    expExplain           ( Enode *, Enode * );          // Enqueue equality and explain
  void    expExplainAlongPath  ( Enode *, Enode * );          // Store explanation in explanation
  void    expEnqueueArguments  ( Enode *, Enode * );          // Enqueue arguments to be explained
  void    expStoreExplanation  ( Enode *, Enode *, Enode * ); // Store the explanation for the merge
  void    expReRootOn          ( Enode * );                   // Reroot the proof tree on x
  void    expUnion             ( Enode *, Enode * );          // Union of x and y in the explanation
  Enode * expFind              ( Enode * );                   // Find for the eq classes of the explanation
  Enode * expHighestNode       ( Enode * );                   // Returns the node of the eq class of x that is closes to the root of the explanation tree
  Enode * expNCA               ( Enode *, Enode * );          // Return the nearest common ancestor of x and y
  void    expRemoveExplanation ( );                           // Undoes the effect of expStoreExplanation
  void    expCleanup           ( );                           // Undoes the effect of expExplain
  //
  // For Extraction and Concatenation Interpretation
  //
  void    cbeCompute      ( Enode *, Enode *, vector< Enode * > &, vector< Enode * > & );
  void    cbeRetrieve     ( Enode *, vector< Enode * > & );
  void    cbeRetrieve     ( Enode *, int, Set( int ) & );
  Enode * cbe             ( Enode * );
  void    cbeUpdate       ( Enode *, int, int );
  void    cbeUpdateRec    ( Enode *, int, int, vector< Enode * > & ); 
  //
  // Debugging routines
  //
  void printExplanation          ( ostream & );
  void printExplanationTree      ( ostream &, Enode * );
  void printExplanationTreeDotty ( ostream &, Enode * );
  void printDistinctionList      ( ostream &, Enode * );
#if PEDANTIC_DEBUG
  bool checkInvariants           ( );
  bool checkExplanation          ( );
  bool checkExplanationTree      ( Enode * );
#endif

  vector< OrdinaryTSolver * > tsolvers; // List of ordinary theory solvers
  
  vector< Enode * >  pending;           // Pending merges
  vector< Enode * >  undo_stack_term;   // Keeps track of terms involved in operations
  vector< oper_t >   undo_stack_oper;   // Keeps track of operations
  vector< Enode * >  deductions_pushed; // Keeps the list of pushed deductions
  //
  // For explanations
  //
  vector< Enode * >  explanation;       // Stores explanation
  vector< Enode * >  exp_pending;       // Pending explanations
  vector< Enode * >  exp_undo_stack;    // Keep track of exp_parent merges
  vector< Enode * >  exp_cleanup;       // List of nodes to be restored
  //
  // Misc
  //
  bool               enable_undo;       // Enable undoable operations
  int                time_stamp;        // Need for finding NCA
  bool               theoryInitialized; // True if theory solvers are initialized
  int                conf_index;        // Index of theory solver that caused conflict
  //
  // Initialize cache
  //
  Set( int )         initialized;       // Keep track of initialized nodes
};

#endif
