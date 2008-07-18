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

#ifndef TSOLVER_H
#define TSOLVER_H

#include "Enode.h"
#include "SMTConfig.h"
#include "SolverTypes.h"

class TSolver
{
public:

  TSolver ( const int i, const char * n, SMTConfig & c )
    : id     ( i )
    , name   ( n )
    , config ( c )
  { }

  virtual ~TSolver ( ) { }

  virtual void                inform             ( Enode * ) = 0; // Inform the solver about the existence of a theory atom
  virtual bool                assertLit          ( Enode * ) = 0; // Assert a theory literal
  virtual void                pushBacktrackPoint ( )         = 0; // Push a backtrack point
  virtual void                popBacktrackPoint  ( )         = 0; // Backtrack to last saved point
  virtual void                popUntilDeduced    ( Enode * ) = 0; // Backtrack to a state that deduced the enode
  virtual Enode *             getDeduction       ( )         = 0; // Return an implied node based on the current state
  virtual vector< Enode * > & getConflict        ( )         = 0; // Return a reference to the explanation
  virtual vector< Enode * > & getReason          ( Enode * ) = 0; // Explain why Lit is deduced
  virtual bool                check              ( bool )    = 0; // Check satisfiability

protected:

  const int         id;               // Id of the solver
  const string      name;             // Name of the solver
  SMTConfig &       config;           // Reference to configuration
  vector< Enode * > explanation;      // Stores the explanation
  list< Enode * >   deductions;       // List of deductions
  vector< size_t >  backtrack_points; // Keeps track of backtrack points
};

class OrdinaryTSolver : public TSolver
{
public:

  OrdinaryTSolver ( const int i, const char * n, SMTConfig & c )
    : TSolver ( i, n, c )
  { }

  virtual ~OrdinaryTSolver ( )
  { }

  virtual bool belongsToT ( Enode * ) = 0; // Atom belongs to this theory
};

#endif
