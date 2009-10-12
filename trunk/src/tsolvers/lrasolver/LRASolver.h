/*********************************************************************
 Author: Aliaksei Tsitovich <aliaksei.tsitovich@lu.unisi.ch>
 Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

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

#ifndef LRASOLVER_H
#define LRASOLVER_H

#include "TSolver.h"
#include "LAVar.h"
#include "LARow.h"

//
// Class to solve Linear Arithmetic theories
//
class LRASolver: public OrdinaryTSolver
{
private:

  // Structure to keep backtracking history elements
  struct LAVarHistory
  {
    Enode * e;
    LAVar * v;
    unsigned bound;
    bool bound_type;
  };

  // Possible internal states of the solver
  typedef enum
  {
    INIT, INCREMENT, SAT, UNSAT, ERROR
  } LRASolverStatus;

  typedef vector<LAVar *> VectorLAVar;

public:

  LRASolver( const int i, const char * n, SMTConfig & c, Egraph & e, vector<Enode *> & x, vector<Enode *> & d, vector<Enode *> & s ) :
    OrdinaryTSolver( i, n, c, e, x, d, s )           // constructor
  {
    status = INIT;
  }
  ~LRASolver( );                                     // destructor

  lbool inform            ( Enode * );               // Inform LRA about the existence of this constraint
  bool  check             ( bool );                  // Checks the satisfiability of current constraints
  bool  assertLit         ( Enode *, bool = false ); // Push the constraint into Solver
  void  pushBacktrackPoint( );                       // Push a backtrack point
  void  popBacktrackPoint ( );                       // Backtrack to last saved point
  bool  belongsToT        ( Enode * );               // Checks if Atom belongs to this theory
  void  computeModel      ( );                       // Computes the model into enodes

private:
  void doGaussianElimination( );                          // Performs Gaussian elimination of all redundant terms in the Tableau
  void update( LAVar *, const Delta & );                  // Updates the bounds after constraint pushing
  void pivotAndUpdate( LAVar *, LAVar *, const Delta &);  // Updates the tableau after constraint pushing
  void getConflictingBounds( LAVar *, vector<Enode *> & );// Returns the bounds conflicting with the actual model
  void refineBounds( );                                   // Compute the bounds for touched polynomials and deduces new bounds from it
  inline bool getStatus( );                               // Read the status of the solver in lbool
  inline bool setStatus( LRASolverStatus );               // Sets and return status of the solver
  void initSolver( );                                     // Initializes the solver
  void print( ostream & out );                            // Prints terms, current bounds and the tableau

  LRASolverStatus status;                  // Internal status of the solver (different from bool)
  vector<LAVar *> enode_lavar;             // Maps original constraints to solver's terms and bounds
  vector<LAVar *> columns;                 // Maps terms' ID to LAVar pointers
  vector<LAVar *> rows;                    // Maps terms' ID to LAVar pointers, used to store basic columns
  vector<LAVar *> slack_vars;              // Collect slack variables (useful for removal)
  vector<Real *> numbers_pool;             // Collect numbers (useful for removal)
  vector<LAVarHistory> pushed_constraints; // Keeps history of constraints
  set<LAVar *> touched_rows;               // Keeps the list of modified rows

  // Two reloaded output operators
  inline friend ostream & operator <<( ostream & out, LRASolver & solver )
  {
    solver.print( out );
    return out;
  }

  inline friend ostream & operator <<( ostream & out, LRASolver * solver )
  {
    solver->print( out );
    return out;
  }

};

#endif
