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

#ifndef CNFIZER_H
#define CNFIZER_H

#include "global.h"
#include "Otl.h"
#include "SMTSolver.h"
#include "Egraph.h"

//
// Generic class for conversion into CNF
//
class Cnfizer
{
public:

  Cnfizer( Egraph & egraph_, SMTSolver & solver_, SMTConfig & config_ )
   : egraph  ( egraph_ )
   , solver  ( solver_ )
   , config  ( config_ )
  { }

  virtual ~Cnfizer( ) { }

  lbool cnfizeAndGiveToSolver ( Enode *, bool = false );               // Main routine. Flag to true writes cnf encoding into /tmp/cnf.smt

protected:
  
  virtual bool cnfize	       ( Enode *, Map( int, Enode * ) & ) = 0; // Actual cnfization. To be implemented in derived classes
  bool         deMorganize     ( Enode * );			       // Apply deMorgan rules whenever feasible
  Enode *      rewriteMaxArity ( Enode *, Map( int, int ) & );         // Rewrite terms using maximum arity

  bool  checkCnf                   ( Enode * );			       // Check if formula is in CNF
  bool  checkDeMorgan              ( Enode * );                        // Check if formula can be deMorganized
  bool  giveToSolver               ( Enode * );                        // Gives formula to the SAT solver

  void  retrieveTopLevelFormulae   ( Enode *, vector< Enode * > & );   // Retrieves the list of top-level formulae
  void  retrieveClause             ( Enode *, vector< Enode * > & );   // Retrieve a clause from a formula
  void  retrieveConjuncts          ( Enode *, vector< Enode * > & );   // Retrieve the list of conjuncts

  Enode * toggleLit		   ( Enode * );                        // Handy function for toggling literals

  void  dumpClause                 ( vector< Enode * > & );            // Dump clause to dump_out
  void  declareStuff               ( set< int > &, Enode * );          // Declare variables/functions into dump_out

  Egraph &    egraph;                                                  // Reference to Egraph
  SMTSolver & solver;                                                  // Reference to Solver
  SMTConfig & config;                                                  // Reference to Config

  bool                        dump_cnf;                                // If true dumps encoding into file
  ofstream                    dump_out;                                // Stream where to dump
  vector< vector< Enode * > > dump_list;                               // Llist of formulae to be dumped

private:

  void    computeIncomingEdges ( Enode *, Map( int, int ) & );                        // Computes the list of incoming edges for a node
  Enode * mergeEnodeArgs       ( Enode *, Map( int, Enode * ) &, Map( int, int ) & ); // Subroutine for rewriteMaxArity

  bool    checkConj            ( Enode *, Set( int ) & );                             // Check if a formula is a conjunction
  bool    checkClause          ( Enode *, Set( int ) & );                             // Check if a formula is a clause
  bool    checkPureConj        ( Enode *, Set( int ) & );                             // Check if a formula is purely a conjuntion
};

#endif
