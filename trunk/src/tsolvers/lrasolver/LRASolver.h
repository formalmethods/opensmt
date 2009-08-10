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

//
// Class to solve Linear Arithmetic theories
//
class LRASolver: public OrdinaryTSolver
{

  struct LAVarHistory
  {
    Enode * e;
    LAVar * v;
    unsigned bound;
    bool boundType;
  };

  typedef enum
  {
    INIT, SAT, UNSAT, ERROR
  } LRAsolverStatus;

  typedef map<int, LAVar *> MapEnodeIDToLAVar;
  typedef vector<LAVar *> VectorLAVar;

public:

  // constructor as you may guess

  LRASolver( const int i, const char * n, SMTConfig & c, Egraph & e, vector<Enode *> & x, vector<Enode *> & d, vector<Enode *> & s ) :
    OrdinaryTSolver( i, n, c, e, x, d, s )
  {
    status = INIT;
  }

  ~LRASolver( )
  {

    // Remove slack variables
    while( !slack_vars.empty( ) )
    {
      LAVar * s = slack_vars.back( );
      slack_vars.pop_back( );
      assert( s );
      delete s;
    }
    // Remove numbers
    while( !numbers_pool.empty( ) )
    {
      assert( numbers_pool.back( ) );
      delete numbers_pool.back( );
      numbers_pool.pop_back( );
    }
  }

  bool belongsToT( Enode * ); // Checks if Atom belongs to this theory

  lbool inform( Enode * ); // Inform LRA about the existence of this constraint
  bool check( bool ); // Checks the satisfiability of current constraints
  bool assertLit( Enode *, bool = false ); // Push the constraint into Solver
  void pushBacktrackPoint( ); // Push a backtrack point
  void popBacktrackPoint( ); // Backtrack to last saved point

  void print( ostream & out ); // Prints terms, current bounds and the tableau

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

private:

  void doGaussianElimination( ); // Performs Gaussian elimination of all redundant terms in the Tableau
  void update( LAVar *, const Delta & ); // Updates the bounds after constraint pushing
  void pivotAndUpdate( LAVar *, LAVar *, const Delta & v ); // Updates the tableau after constraint pushing
  void getConflictingBounds( LAVar *, vector<Enode *> & dst ); // Returns the bounds conflicting with the actual model
  void refineBounds( ); // Compute the bounds for touched polynomials and deduces new bounds from it
  inline bool getStatus( ); // Read the status of the solver in lbool
  inline bool setStatus( LRAsolverStatus ); // Sets and return status of the solver

  void initSolver( ); // Initializes the solver

  LRAsolverStatus status; // Internal status of the solver (different from bool)
  vector<LAVar *> enode_lavar; // Maps original constraints to solver's terms and bounds
  vector<LAVar *> columns; // Maps terms' ID to LAVar pointers
  vector<LAVar *> rows; // Maps terms' ID to LAVar pointers, used to store basic columns
  vector<LAVar *> slack_vars; // Collect slack variables (useful for removal)
  vector<Real *> numbers_pool; // Collect numbers (useful for removal)
  vector<LAVarHistory> pushed_constraints; // Keeps history of constraints
  set<LAVar *> touched_rows; // Keeps history of constraints

};

#endif
