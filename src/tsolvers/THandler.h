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

#ifndef THANDLER_H
#define THANDLER_H

#include "SMTSolver.h"
#include "SMTConfig.h"
#include "Egraph.h"
#include "TSolver.h"

class SMTSolver; // Forward declaration

class THandler
{
public:

  THandler ( Egraph &      e
           , SMTConfig &   c
           , SMTSolver &   s
	   , vec< Lit > &  t
	   , vec< int > &  l
	   , vec< char > & a )
    : core_solver        ( e )
    , config             ( c )
    , solver             ( s )
    , trail              ( t )
    , level              ( l )
    , assigns            ( a )
    , checked_trail_size ( 0 )
  { }
  
  virtual ~THandler ( ) { }

  void    getConflict  ( vec< Lit > &, int & ); // Returns theory conflict in terms of literals
  Lit     getDeduction ( );			// Returns a literal that is implied by the current state
  void    getReason    ( Lit, vec< Lit > & );   // Returns the explanation for a deduced literal
                                         
  Var     enodeToVar   ( Enode * );             // Converts enode into boolean variable. Create a new variable if needed
  Lit     enodeToLit   ( Enode * );             // Converts enode into boolean literal. Create a new variable if needed
  Enode * varToEnode   ( Var );                 // Return the enode corresponding to a variable

  bool    assertLits   ( );                     // Give to the TSolvers the newly added literals on the trail
  bool    check        ( bool );                // Check trail in the theories
  void    backtrack    ( int );                 // Remove literals up to a certain decision level

private:                                 

  void    backtrackToStackSize ( size_t );      // Backtrack until stack size 

#ifdef EXTERNAL_TOOL
  void verifyCallWithExternalTool        ( bool, size_t );
  void verifyExplanationWithExternalTool ( vector< Enode * > &, Enode * = NULL );
  void declareStuff                      ( ostream &, set< int > &, Enode * );
#endif

  bool  isOnTrail     ( Lit );
                                         
  vector< Var >       enode_id_to_var;          // Conversion EnodeID --> Var
  vector< Enode * >   var_to_enode;             // Conversion Var --> EnodeID
                                               
  Egraph &            core_solver;              // Pointer to Egraph that works as core solver
  SMTConfig &         config;                   // Reference to configuration
  SMTSolver &         solver;                   // Reference to SMT Solver
  vec< Lit > &        trail;                    // Reference to SMT Solver trail
  vec< int > &        level;                    // Reference to SMT Solver level
  vec< char > &       assigns;                  // Reference to SMT Solver assigns
  vector< Enode * >   stack;                    // Stack of theory atoms
  int                 checked_trail_size;       // Store last size of the trail checked by the solvers
  vector< size_t >    level_to_stack_size;      // Hold the correspondence decision level -> stack.size( )
};

#endif
