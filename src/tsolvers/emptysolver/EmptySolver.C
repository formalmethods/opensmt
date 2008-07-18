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

//
// This is an empty solver to be used as a template for the 
// development of ordinary theory solvers. Set EMPTY_SOLVER_OUTPUT to 1 
// (and recompile) to get a verbose printout for each action at the 
// time they are called
//

#define EMPTY_SOLVER_OUTPUT 1

#include "EmptySolver.h"

//
// The solver is informed of the existence of
// atom e. It might be useful for initializing
// the solver's data structures. This function is 
// called before the actual solving starts.
// 
void EmptySolver::inform( Enode * e )  
{ 
  assert( e );
  assert( belongsToT( e ) );
}

// 
// Asserts a literal into the solver. If by chance
// you are able to discover inconsistency you may
// return false. The real consistency state will
// be checked with "check"
//
bool EmptySolver::assertLit ( Enode * e )
{
  assert( e );
  assert( belongsToT( e ) );
  return true;
}

//
// Saves a backtrack point
// You are supposed to keep track of the
// operations, for instance in a vector
// called "undo_stack_term", as happens
// in EgraphSolver
//
void EmptySolver::pushBacktrackPoint ( )
{
  // backtrack_points( undo_stack_term.size( );
}

//
// Restore a previous state. You can now retrieve
// the size of the stack when you pushed the last
// backtrack point. You have to implement the 
// necessary backtrack operations 
// (see for instance backtrackToStackSize( )
// in EgraphSolver)
// Also make sure you clean the deductions you
// did not communicate
//
void EmptySolver::popBacktrackPoint ( )
{
  // assert( backtrack_points.size( ) > 0 );
  // size_t undo_stack_new_size = backtrack_points.back( );
  //
  // Do Something to restore the solver state
  //
  // backtrack_points.pop_back( );
}

void EmptySolver::popUntilDeduced ( Enode * e )
{
  // Make sure this solver deduced e
  assert( e->getDedIndex( ) == id );
  //
  // Do something to restore that state that deduced e
  //
}

//
// With this function you can provide a deduction.
// NULL means no deductions. Providing deductions
// it's optional.
//
Enode * EmptySolver::getDeduction ( )
{
  return NULL;
}

vector< Enode * > & EmptySolver::getConflict ( )
{
  return explanation;
}

//
// This function is called to retrieve an
// explanation of a deduction. You can assume
// that the solver has already been backtracked
// to the state that caused this deduction to
// happen: in that state clearly (not e) does not
// hold. An easy way to get an explanation is to 
// temporairly push (not e) and return the explanation
//
vector< Enode * > & EmptySolver::getReason ( Enode * e )
{
  assert( e );
  return explanation;
}

//
// Check for consistency. If flag is
// set make sure you run a complete check
//
bool EmptySolver::check( bool complete )    
{ 
  // Here check for consistency
  return true;
}

//
// Return true if the enode belongs
// to this theory. You should examine
// the structure of the node to see
// if it matches the theory operators
//
bool EmptySolver::belongsToT( Enode * e )
{
  assert( e );
  return true;
}
