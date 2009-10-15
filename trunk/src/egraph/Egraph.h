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

#ifndef EGRAPH_H
#define EGRAPH_H

#include "Enode.h"
#include "TSolver.h"
#include "SigTab.h"
#include "SplayTree.h"

class Egraph : public CoreTSolver
{
public:

  Egraph( SMTConfig & c )
      : CoreTSolver       ( 0, "EUF Solver", c )
      , enil              ( new Enode )
      , active_dup1       ( false )
      , active_dup2       ( false )
      , dup_count1        ( 0 )
      , dup_count2        ( 0 )
      , active_dup_map    ( false )
      , active_dup_map2   ( false )
      , dup_map_count     ( 0 )
      , dup_map_count2    ( 0 )
      , theoryInitialized ( false )
      , time_stamp        ( 0 )
      , enable_undo       ( false )
      , use_gmp		  ( false )
  {
    //
    // Initialize nil key for splay tree
    //
    Enode * nilKey = const_cast< Enode * >( enil );
    store.setNil( nilKey );
    id_to_enode.push_back( const_cast< Enode * >( enil ) );
    //
    // Initialize Egraph-Based Storage for terms
    //
    initializeStore( );
  }

  ~Egraph( )
  {
    backtrackToStackSize( 0 );
#ifdef STATISTICS
    if ( config.gconfig.print_stats && tsolvers_stats.size( ) > 0 )
    {
      config.getStatsStream( ) << "# -------------------------" << endl;
      config.getStatsStream( ) << "# STATISTICS FOR EUF SOLVER" << endl;
      config.getStatsStream( ) << "# -------------------------" << endl;
      tsolvers_stats[ 0 ]->printStatistics( config.getStatsStream( ) );
      delete tsolvers_stats[ 0 ];
    }
#endif
    //
    // Delete Ordinary Theory Solvers
    //
#ifdef STATISTICS
    assert( tsolvers.size( ) == tsolvers_stats.size( ) );
#endif
    for ( unsigned i = 1 ; config.gconfig.print_stats && i < tsolvers.size( ) ; i ++ )
    {
#ifdef STATISTICS
      config.getStatsStream( ) << "# -------------------------" << endl;
      config.getStatsStream( ) << "# STATISTICS FOR " << tsolvers[ i ]->getName( ) << endl;
      config.getStatsStream( ) << "# -------------------------" << endl;
      assert( tsolvers_stats[ i ] );
      tsolvers_stats[ i ]->printStatistics( config.getStatsStream( ) );
      delete tsolvers_stats[ i ];
#endif
      assert( tsolvers[ i ] );
      delete tsolvers[ i ];
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

  void     newSort             ( const char * );                                                 // Inserts a new uninterpreted sort
  unsigned getSort             ( const char * );                                                 // From name to sort id
  Enode *  newSymbol           ( const char *, vector< unsigned > & );                           // Creates a new symbol
  Enode *  newSymbol           ( const char *, const unsigned );                                 // Creates a new symbol
  Enode *  cons                ( list< Enode * > & );                                            // Shortcut, but not efficient
  Enode *  cons                ( Enode *, Enode * );                                             // Create Lists/Terms
  Enode *  cons                ( Enode * e ) { return cons( e, const_cast< Enode * >(enil) ); }  // Shortcut for singleton
  void     undoCons            ( Enode * );							 // Undoes a cons
  //
  // Specialized functions 
  // 
  inline Enode * mkLt          ( Enode * args ) { return mkNot( cons( mkLeq( swapList( args ) ) ) ); }  
  inline Enode * mkGeq         ( Enode * args ) { return              mkLeq( swapList( args ) ); }  
  inline Enode * mkGt          ( Enode * args ) { return mkNot( cons( mkLeq(           args ) ) ); } 

  inline Enode * mkTrue        ( )              { return etrue; }  
  inline Enode * mkFalse       ( )              { return efalse; } 
  
  inline Enode * mkBvslt       ( Enode * args, bool simp = true ) { return simp ? mkNot( cons( mkBvsle( swapList( args ) ) ) ) : cons( id_to_enode[ ENODE_ID_BVSLT ], args ); }
  inline Enode * mkBvsge       ( Enode * args )                   { return              mkBvsle( swapList( args ) ); }
  inline Enode * mkBvsgt       ( Enode * args )                   { return mkNot( cons( mkBvsle( args ) ) ); }

  inline Enode * mkBvult       ( Enode * args, bool simp = true ) { return simp ? mkNot( cons( mkBvule( swapList( args ) ) ) ) : cons( id_to_enode[ ENODE_ID_BVULT ], args ); }
  inline Enode * mkBvuge       ( Enode * args )                   { return              mkBvule( swapList( args ) ); }
  inline Enode * mkBvugt       ( Enode * args )                   { return mkNot( cons( mkBvule( args ) ) ); }

  inline Enode * mkBvurem      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVUREM ], args ); }
  inline Enode * mkBvudiv      ( Enode * args ) { return cons( id_to_enode[ ENODE_ID_BVUDIV ], args ); }

  //
  // Implemented in EgraphStore.C
  //
  Enode * mkPlus             ( Enode * );
  Enode * mkMinus            ( Enode * );
  Enode * mkTimes            ( Enode * );
  Enode * mkDiv              ( Enode * );
  
  Enode * mkUminus           ( Enode * );

  Enode * mkBvand            ( Enode * );
  Enode * mkBvor             ( Enode * );
  Enode * mkBvnot            ( Enode * );
  Enode * mkBvxor            ( Enode * );

  Enode * mkConcat           ( Enode * );
  Enode * mkCbe              ( Enode * );
  Enode * mkDistinct         ( Enode * );
  Enode * mkBvlshr           ( Enode * );
  Enode * mkBvashr           ( Enode * );
  Enode * mkBvshl            ( Enode * );
  Enode * mkBvneg            ( Enode * );
  Enode * mkBvmul            ( Enode * );
  Enode * mkBvadd            ( Enode * );
  Enode * mkBvsub            ( Enode * );
  Enode * mkBvsdiv           ( Enode * );
  Enode * mkBvsrem           ( Enode * );

  Enode * mkSelect	     ( Enode *, Enode * );	
  Enode * mkStore	     ( Enode *, Enode *, Enode * );

  Enode * mkNot              ( Enode * );
  Enode * mkAnd              ( Enode * );
  Enode * mkIff              ( Enode * );
  Enode * mkOr               ( Enode * );

  Enode * mkIte              ( Enode *, Enode *, Enode * );
  Enode * mkIfthenelse       ( Enode *, Enode *, Enode * );
  Enode * mkEq               ( Enode * );
  Enode * mkNeq              ( Enode * );
  Enode * mkLeq              ( Enode * );
  Enode * mkBvsle            ( Enode * );
  Enode * mkBvule            ( Enode * );
  Enode * mkZeroExtend       ( int, Enode * );
  Enode * mkSignExtend       ( int, Enode * );
  Enode * mkRotateLeft       ( int, Enode * );
  Enode * mkRotateRight      ( int, Enode * );
  Enode * mkExtract          ( int, int, Enode * );
  Enode * mkImplies          ( Enode * );
  Enode * mkXor              ( Enode * );
  Enode * mkBvnum            ( char * );
  Enode * allocTrue          ( );
  Enode * allocFalse         ( );
                             
  Enode * mkVar              ( const char *, bool = false );
  Enode * mkNum              ( const char * );
  Enode * mkNum              ( const char *, const char * );
  Enode * mkNum              ( const Real & );
  Enode * mkUf               ( const char *, Enode * );
  Enode * mkUp               ( const char *, Enode * );

  void    mkDefine           ( const char *, Enode * );
  Enode * getDefine          ( const char * );

  Enode * mkWord1cast        ( Enode * );
  Enode * mkBoolcast         ( Enode * );

  Enode * makeNumberFromGmp  ( mpz_class &, const int );

  Enode * getFormula         ( );
  void    setDistinctEnodes  ( vector< Enode * > & );

  void    printEnodeList     ( ostream & );

  inline void setTopEnode    ( Enode * e ) { assert( e ); top = e; /*inc_fan_in( e );*/ }

  void addAssumption         ( Enode * );

  inline size_t nofEnodes    ( ) { return id_to_enode.size( ); }

  inline Enode * indexToDist ( unsigned index ) const
  { 
    assert( index < index_to_dist.size( ) ); 
    return index_to_dist[ index ]; 
  }

  Enode * copyEnodeEtypeTermWithCache   ( Enode *, bool = false );
  Enode * copyEnodeEtypeListWithCache   ( Enode *, bool = false );

  inline void         setRescale        ( Real & r ) { rescale_factor = r; rescale_factor_l = atol( r.get_str( ).c_str( ) ); }
  inline const Real & getRescale        ( Real & p ) { (void)p; return rescale_factor; }
  inline const long & getRescale        ( long & p ) { (void)p; return rescale_factor_l; }

  inline bool hasItes                   ( ) { return has_ites; }

#ifdef STATISTICS
  void        printMemStats             ( ostream & );
#endif
  //
  // Fast duplicates checking. Cannot be nested !
  //
  inline void initDup1  ( )           { assert( !active_dup1 ); active_dup1 = true; duplicates1.resize( id_to_enode.size( ), dup_count1 ); dup_count1 ++; }
  inline void storeDup1 ( Enode * e ) { assert(  active_dup1 ); assert( e->getId( ) < (enodeid_t)duplicates1.size( ) ); duplicates1[ e->getId( ) ] = dup_count1; }
  inline bool isDup1    ( Enode * e ) { assert(  active_dup1 ); assert( e->getId( ) < (enodeid_t)duplicates1.size( ) ); return duplicates1[ e->getId( ) ] == dup_count1; }
  inline void doneDup1  ( )           { assert(  active_dup1 ); active_dup1 = false; }
  //
  // Fast duplicates checking. Cannot be nested !
  //
  inline void initDup2  ( )           { assert( !active_dup2 ); active_dup2 = true; duplicates2.resize( id_to_enode.size( ), dup_count2 ); dup_count2 ++; }
  inline void storeDup2 ( Enode * e ) { assert(  active_dup2 ); assert( e->getId( ) < (enodeid_t)duplicates2.size( ) ); duplicates2[ e->getId( ) ] = dup_count2; }
  inline bool isDup2    ( Enode * e ) { assert(  active_dup2 ); assert( e->getId( ) < (enodeid_t)duplicates2.size( ) ); return duplicates2[ e->getId( ) ] == dup_count2; }
  inline void doneDup2  ( )           { assert(  active_dup2 ); active_dup2 = false; }
  //
  // Fast duplicates checking. Cannot be nested !
  //
  void    initDupMap  ( );
  void    storeDupMap ( Enode *, Enode * );
  Enode * valDupMap   ( Enode * );
  void    doneDupMap  ( );

  void    initDupMap2 ( );
  void    storeDupMap2( Enode *, Enode * );
  Enode * valDupMap2  ( Enode * );
  void    doneDupMap2 ( );

  void    computePolarities ( Enode * );


#ifndef SMTCOMP
  void dumpHeaderToFile  ( ofstream & );
  void dumpFormulaToFile ( ofstream &, Enode * );
  void dumpToFile        ( const char *, Enode * );
#endif

  //===========================================================================
  // Public APIs for Egraph Core Solver

  void		      initializeTheorySolvers ( SimpSMTSolver * );       // Attaches ordinary theory solvers
  lbool               inform                  ( Enode * );               // Inform the solver about the existence of a theory atom
  bool                assertLit               ( Enode *, bool = false ); // Assert a theory literal
  void                pushBacktrackPoint      ( );                       // Push a backtrack point
  void                popBacktrackPoint       ( );                       // Backtrack to last saved point
  Enode *             getDeduction            ( );                       // Return an implied node based on the current state
  Enode *             getSuggestion           ( );                       // Return a suggested literal based on the current state
  vector< Enode * > & getConflict             ( bool = false );          // Get explanation
  bool                check                   ( bool );		         // Check satisfiability
  void                initializeCong          ( Enode * );               // Initialize congruence structures for a node
  void                printModel              ( ostream & );             // Computes and print the model
  inline void         setUseGmp               ( ) { use_gmp = true; }
  inline bool         getUseGmp               ( ) { return use_gmp; }
  void                splitOnDemand           ( vector< Enode * > &, int );

private:
  
  //===========================================================================
  // Private Routines for enode construction/destruction

  //
  // Defines the set of operations that can be performed and that should be undone
  //
  typedef enum { ASSERT, SYMB, NUMB, CONS, MERGE, DISEQ, DIST, SPLIT, CBETSTORE, CBENEQSTORE, REASON } oper_t;
  //
  // Handy function to swap two arguments of a list
  //
  inline Enode * swapList ( Enode * args ) 
  { 
    assert( args );
    assert( args->isList( ) );
    assert( args->getArity( ) == 2 );
    return cons( args->getCdr( )->getCar( ), cons( args->getCar( ) ) );
  }

  void    initializeStore ( );                                  // Initializes store
  //                                                            
  // Related to term creation                                   
  //                                                            
  Enode * insertNumber ( Enode * );                             // Inserts a number
  void    removeNumber ( Enode * );                             // Remove a number
  void    insertSymbol ( Enode * );                             // Inserts a symbol
  void    removeSymbol ( Enode * );                             // Remove a symbol
  Enode * lookupSymbol ( const char * name );                   // Retrieve a symbol
  void    insertDefine ( const char *, Enode * );               // Insert a define
  Enode * lookupDefine ( const char * );                        // Retrieve a define
  Enode * insertStore  ( const enodeid_t, Enode *, Enode * );   // Insert node into the global store
  //                                                                
  // Related to congruence closure                                  
  //                                                                
  Enode * insertSigTab ( const enodeid_t, Enode *, Enode * );   // For undoable cons only
  Enode * insertSigTab ( Enode * );                             // For for terms that go in the congruence
  Enode * lookupSigTab ( Enode * );                             // Retrieve Enode
  void    removeSigTab ( Enode * );                             // Remove Enode from sig_tab
                                                                
  bool               active_dup1;                               // To prevent nested usage
  bool               active_dup2;                               // To prevent nested usage
  vector< int >      duplicates1;                               // Fast duplicate checking
  vector< int >      duplicates2;                               // Fast duplicate checking
  int                dup_count1;                                // Current dup token
  int                dup_count2;                                // Current dup token
  bool               active_dup_map;                            // To prevent nested usage
  bool               active_dup_map2;                           // To prevent nested usage
  vector< Enode * >  dup_map;                                   // Fast duplicate checking
  vector< int >      dup_set;                                   // Fast duplicate checking
  vector< Enode * >  dup_map2;                                  // Fast duplicate checking
  vector< int >      dup_set2;                                  // Fast duplicate checking
  int                dup_map_count;                             // Current dup token
  int                dup_map_count2;                            // Current dup token
  MapNameUint        name_to_extrasort;                         // Store for extrasorts
  map< int, string > extrasort_to_name;                         // Store for extrasorts
  MapNameEnode       name_to_number;                            // Store for numbers
  MapNameEnode       name_to_symbol;                            // Store for symbols
  MapNameEnode       name_to_define;                            // Store for defines
  SplayTree< Enode *, Enode::idLessThan > store;                
                                                                
  SigTab             sig_tab;                                   // (Supposely) Efficient Signature table for congruence closure
  vector< Enode * >  id_to_enode;                               // Table ENODE_ID --> ENODE
  vector< int >      id_to_belong_mask;                         // Table ENODE_ID --> ENODE
  vector< int >      id_to_fan_in;                              // Table ENODE_ID --> fan in
  vector< Enode * >  index_to_dist;                             // Table distinction index --> enode
  list< Enode * >    assumptions;                               // List of assumptions
  vector< Enode * >  cache;                                     // Cache simplifications
  Enode *            top;                                       // Top node of the formula
  MapPairEnode       ext_store;                                 // For fast extraction handling
  vector< Enode * >  se_store;                                  // For fast sign extension
  vector< int >      id_to_inc_edges;                           // Keeps track of how many edges enter an enode
  bool               has_ites;                                  // True if there is at least one ite
  set< Enode * >     variables;                                 // List of variables
  
  //===========================================================================
  // Private Routines for Core Theory Solver

  bool    assertLit_      ( Enode * );                          // Assert a theory literal
  //                                                            
  // Asserting literals                                         
  //                                                            
  bool    assertEq        ( Enode *, Enode *, Reason * );       // Asserts an equality
  bool    assertNEq       ( Enode *, Enode *, Reason * );       // Asserts a negated equality
  bool    assertDist      ( Enode * );                          // Asserts a distinction
  //                                                            
  // Backtracking                                               
  //                                                            
  void backtrackToStackSize ( size_t size );                    // Backtrack to a certain operation
  //                                                            
  // Congruence closure main routines                           
  //                                                            
  bool    unmergable      ( Enode *, Enode *, Enode ** );       // Can two nodes be merged ?
  void    merge           ( Enode *, Enode * );                 // Merge two nodes
  void	  deduce          ( Enode *, Enode * );                 // Deduce from merging of two nodes
  void    undoMerge       ( Enode * );                          // Undoes a merge
  void    undoDisequality ( Enode * );                          // Undoes a disequality
  void    undoDistinction ( Enode * );                          // Undoes a distinction
  //
  // Explanation routines and data
  //
  void     expExplain           ( );                            // Main routine for explanation
  void     expExplain           ( Enode *, Enode * );           // Enqueue equality and explain
  void     expExplainAlongPath  ( Enode *, Enode * );           // Store explanation in explanation
  void     expEnqueueArguments  ( Enode *, Enode * );           // Enqueue arguments to be explained
  void     expStoreExplanation  ( Enode *, Enode *, Reason * ); // Store the explanation for the merge
  void     expReRootOn          ( Enode * );                    // Reroot the proof tree on x
  void     expUnion             ( Enode *, Enode * );           // Union of x and y in the explanation
  Enode *  expFind              ( Enode * );                    // Find for the eq classes of the explanation
  Enode *  expHighestNode       ( Enode * );                    // Returns the node of the eq class of x that is closes to the root of the explanation tree
  Enode *  expNCA               ( Enode *, Enode * );           // Return the nearest common ancestor of x and y
  void     expRemoveExplanation ( );                            // Undoes the effect of expStoreExplanation
  void     expCleanup           ( );                            // Undoes the effect of expExplain
  void     expPushNewReason     ( Reason * );                   // Allocates a new reason
  void     expDeleteLastReason  ( );                            // Delete a previously allocated reason

  inline const char * logicStr ( logic_t l )
  {
         if ( l == EMPTY )    return "EMPTY";
    else if ( l == QF_UF )    return "QF_UF";
    else if ( l == QF_BV )    return "QF_BV";
    else if ( l == QF_RDL )   return "QF_RDL";
    else if ( l == QF_IDL )   return "QF_IDL";
    else if ( l == QF_LRA )   return "QF_LRA";
    else if ( l == QF_LIA )   return "QF_LIA";
    else if ( l == QF_UFRDL ) return "QF_UFRDL";
    else if ( l == QF_UFIDL ) return "QF_UFIDL";
    else if ( l == QF_UFLRA ) return "QF_UFLRA";
    else if ( l == QF_UFLIA ) return "QF_UFLIA";
    else if ( l == QF_UFBV )  return "QF_UFBV";
    else if ( l == QF_AX )    return "QF_AX";
    return "";
  }

  bool                        theoryInitialized;                // True if theory solvers are initialized
  bool                        state;                            // the hell is this ?
  Set( enodeid_t )            initialized;                      // Keep track of initialized nodes
  Map( enodeid_t, lbool )     informed;                         // Keep track of informed nodes
  vector< Enode * >           pending;                          // Pending merges
  vector< Enode * >           undo_stack_term;                  // Keeps track of terms involved in operations
  vector< oper_t >            undo_stack_oper;                  // Keeps track of operations
  vector< Enode * >           explanation;                      // Stores explanation
  vector< Enode * >           exp_pending;                      // Pending explanations
  vector< Enode * >           exp_undo_stack;                   // Keep track of exp_parent merges
  vector< Enode * >           exp_cleanup;                      // List of nodes to be restored
  int                         time_stamp;                       // Need for finding NCA
  int                         conf_index;                       // Index of theory solver that caused conflict
  vector< Reason * >          reasons;
  Real                        rescale_factor;                   // Rescale factor for DL
  long                        rescale_factor_l;                 // Rescale factor for DL
  bool                        enable_undo;                      // Enable undoable operations
  bool                        use_gmp;                          // Do we have to use gmp?

  //===========================================================================
  // Private Routines for Extraction Concatenation Interpretation
  
  //
  // Asserting Bv literals
  //
  bool    assertBv              ( Enode *, Enode *, Enode *, bool ); 
  //
  // CBE Computation
  //
  void    cbeDetectNewSlices    ( Enode *, Enode * );
  void    cbeCompute            ( Enode *, Enode *, list< Enode * > &, list< Enode * > & );
  void    cbeRetrieve           ( Enode *, const int, const int, bool, OrderedSet( int ) &, list< Enode * > & );
  Enode * cbe                   ( Enode * );
  void    cbeUpdate             ( Enode *, const int, const int );
  bool    cbeIsExternal         ( Enode *, const int, const int );
  void    cbeUpdateSplit        ( Enode *, const int, const int );
  bool    cbeEquateSlices       ( );
  void    cbeExplainCb          ( Enode * );
  void    cbeExplainConstant    ( Enode * );
  void    cbeExplainConstantRec ( const int, const int, Enode * );
  void    cbeExplainSlice       ( const int, const int, Enode * );
  void    cbeExplainSliceRec    ( const int, const int, Enode *, Enode *, Enode * );
  void    cbeUndoStore          ( Enode * );
  void    cbeAddToCb            ( Enode * );
  void    cbeRemoveFromCb       ( Enode * );
  bool    cbeIsInCb             ( Enode * );
  Enode * cbeGetSlice           ( const int, const int, Enode * );

  vector< bool >                terms_stored;                   // Is term stored in terms_to_update ?
  vector< int >                 terms_in_cb;                    // True if the term is part of a cb
  vector< Enode * >             neq_stored;                     // List of neqs to check

  //===========================================================================
  // Debugging routines

  void printExplanation          ( ostream & );
  void printExplanationTree      ( ostream &, Enode * );
  void printExplanationTreeDotty ( ostream &, Enode * );
  void printDistinctionList      ( ostream &, Enode * );
  void printCbeStructure         ( );
  void printCbeStructure         ( ostream &, Enode *, Set( int ) & );
#if PEDANTIC_DEBUG
  bool checkExplanationTree      ( Enode * );
  bool checkExplanation          ( );
  bool checkReachable            ( Enode *, Enode * );
#endif

#ifdef STATISTICS
  void printStatistics ( ofstream & );
#endif
};

inline void Egraph::initDupMap( )
{ 
  assert( !active_dup_map ); 
  active_dup_map = true; 
  dup_map.resize( id_to_enode.size( ), NULL ); 
  dup_set.resize( id_to_enode.size( ), dup_map_count ); 
  dup_map_count ++; 
}

inline void Egraph::initDupMap2( )
{ 
  assert( !active_dup_map2 ); 
  active_dup_map2 = true; 
  dup_map2.resize( id_to_enode.size( ), NULL ); 
  dup_set2.resize( id_to_enode.size( ), dup_map_count2 ); 
  dup_map_count2 ++; 
}

inline void Egraph::storeDupMap( Enode * k, Enode * e ) 
{ 
  assert(  active_dup_map ); 
  dup_map.resize( id_to_enode.size( ), NULL ); 
  dup_set.resize( id_to_enode.size( ), dup_map_count - 1 ); 
  assert( k->getId( ) < (enodeid_t)dup_set.size( ) ); 
  dup_set[ k->getId( ) ] = dup_map_count; 
  dup_map[ k->getId( ) ] = e; 
}

inline void Egraph::storeDupMap2( Enode * k, Enode * e ) 
{ 
  assert(  active_dup_map2 ); 
  dup_map2.resize( id_to_enode.size( ), NULL ); 
  dup_set2.resize( id_to_enode.size( ), dup_map_count2 - 1 ); 
  assert( k->getId( ) < (enodeid_t)dup_set2.size( ) ); 
  dup_set2[ k->getId( ) ] = dup_map_count2; 
  dup_map2[ k->getId( ) ] = e; 
}

inline Enode * Egraph::valDupMap( Enode * k )
{ 
  assert(  active_dup_map ); 
  dup_map.resize( id_to_enode.size( ), NULL ); 
  dup_set.resize( id_to_enode.size( ), dup_map_count - 1 ); 
  assert( k->getId( ) < (enodeid_t)dup_set.size( ) ); 
  if ( dup_set[ k->getId( ) ] == dup_map_count ) 
    return dup_map[ k->getId( ) ]; 
  return NULL; 
}

inline Enode * Egraph::valDupMap2( Enode * k )
{ 
  assert(  active_dup_map2 ); 
  dup_map2.resize( id_to_enode.size( ), NULL ); 
  dup_set2.resize( id_to_enode.size( ), dup_map_count2 - 1 ); 
  assert( k->getId( ) < (enodeid_t)dup_set2.size( ) ); 
  if ( dup_set2[ k->getId( ) ] == dup_map_count2 ) 
    return dup_map2[ k->getId( ) ]; 
  return NULL; 
}

inline void Egraph::doneDupMap( ) 
{ 
  assert(  active_dup_map ); 
  active_dup_map = false; 
}

inline void Egraph::doneDupMap2( ) 
{ 
  assert(  active_dup_map2 ); 
  active_dup_map2 = false; 
}

#endif
