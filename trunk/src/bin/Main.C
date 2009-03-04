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

#include "Egraph.h"
#include "MinisatSMTSolver.h"
#include "Tseitin.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <csignal>

#if defined(__linux__)
#include <fpu_control.h>
#endif

void        printResult( const lbool & );
void        catcher( int );
extern int  smtparse( );
extern int  smtrestart( FILE * );
Egraph *    parser_egraph;
SMTConfig * parser_config;

/*****************************************************************************\
 *                                                                           *
 *                                  MAIN                                     *
 *                                                                           *
\*****************************************************************************/

int main( int argc, char * argv[] )
{
  // Catch SigTerm, so that it answers even on ctrl-c
  signal( SIGTERM, catcher );
  signal( SIGINT, catcher );

#ifndef SMTCOMP
  cerr << "#" << endl
       << "# -------------------------------------------------------------------------" << endl
       << "#                                                                          " << endl
       << "# OpenSMT " << VERSION << " -- Copyright (C) 2008, Roberto Bruttomesso" << endl
       << "#                                                                          " << endl
       << "# OpenSMT is free software: you can redistribute it and/or modify it under " << endl
       << "# the terms of the GNU General Public License as published by the Free     " << endl
       << "# Software Foundation, either version 3 of the License, or (at your option)" << endl
       << "# any later version.                                                       " << endl
       << "#                                                                          " << endl
       << "# OpenSMT is distributed in the hope that it will be useful, but WITHOUT   " << endl
       << "# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    " << endl
       << "# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for " << endl
       << "# more details.                                                            " << endl
       << "#                                                                          " << endl
       << "# You should have received a copy of the GNU General Public License along  " << endl
       << "# with OpenSMT. If not, see <http://www.gnu.org/licenses/>.                " << endl
       << "#                                                                          " << endl
       << "# -------------------------------------------------------------------------" << endl
       << "# http://code.google.com/p/opensmt           <roberto.bruttomesso@unisi.ch>" << endl
       << "# -------------------------------------------------------------------------" << endl
       << "#" << endl;
#endif

  // Allocate configuration
  SMTConfig config( UNDEF, l_Undef );
  // Allocates the egraph
  Egraph egraph( config );                                    
  // Parse the input formula
  parser_egraph = &egraph;                          
  parser_config = &config;
  // Accepts file from stdin if nothing specified
  FILE * fin = NULL;
  if ( argc == 1 )
    fin = stdin;
  else
    if ( (fin = fopen( argv[ 1 ], "rt" )) == NULL ) { cerr << "Error: can't open file " << argv[ 1 ] << endl; exit( 1 ); }
  // Parse
  smtrestart( fin );                               
  smtparse( );                                      
  fclose( fin );                                    
  // Initializes theory solvers
  egraph.initializeTheorySolvers( );
  //
  // This trick (copied from Main.C of MiniSAT) is to allow
  // the repeatability of experiments that might be compromised
  // by the floating point unit approximations on doubles 
  //
#if defined(__linux__) && !defined( SMTCOMP )
  fpu_control_t oldcw, newcw;
  _FPU_GETCW(oldcw); newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE; _FPU_SETCW(newcw);
  reportf("# WARNING: for repeatability, setting FPU to use double precision\n");
#endif

#ifdef PEDANTIC_DEBUG
  reportf("# WARNING: pedantic assertion checking enabled (very slow)\n");
#endif

#ifdef EXTERNAL_TOOL
  reportf("# WARNING: external tool checking enabled (very slow)\n");
#endif

#ifndef OPTIMIZE
  reportf( "# WARNING: this binary is compiled with optimizations disabled (slow)\n" );
#endif

#ifndef SMTCOMP 
  reportf( "#\n" );
#endif
  // Retrieve the formula
  Enode * formula = egraph.getFormula( );          

  if ( formula == NULL )
  {
    cerr << "Error: formula undefined" << endl;
    exit( 1 );
  }

  if ( config.logic == UNDEF )
  {
    cerr << "Error: unable to determine logic" << endl;
    return 1;
  }

  lbool result = l_Undef;

  if ( formula->isTrue( ) )
    result = l_True;
  else if ( formula->isFalse( ) )
    result = l_False;
  else
  {
    // Allocates SMTSolver based on MiniSAT
    MinisatSMTSolver solver( egraph, config );

    // Allocates Tseitin-like cnfizer
    Tseitin cnfizer( egraph, solver, config );

    // CNFize the input formula and fed clauses to the solver
    result = cnfizer.cnfizeAndGiveToSolver( formula );

    // Solve
    result = solver.smtSolve( );		            
  }

  printResult( result );
  // Prints the model (currently disabled)
  // if ( result == l_True ) S.printModel( );

#ifndef SMTCOMP
  if ( config.status != l_Undef && 
       result != l_Undef &&
       result != config.status )
  {
    cerr << "# Error: result does not match with status" << endl;
    return 1;
  }
#endif

  return 0;
}

void catcher(int sig)
{
  switch (sig)
  {
    case SIGINT:
    case SIGTERM:
      printResult( l_Undef );
      exit( 1 );
      break;
  }
}

void printResult( const lbool & result )
{
#ifndef SMTCOMP
  fflush( stderr );
#endif

  if ( result == l_True )			    
    cout << "sat" << endl;
  else if ( result == l_False )
    cout << "unsat" << endl;
  else if ( result == l_Undef )
    cout << "unknown" << endl; 
  else
  {
    cerr << "Error: wrong result" << endl;
    exit( 1 );
  }

  fflush( stdout );

#ifndef SMTCOMP
  //
  // Statistics
  //
  double   cpu_time = cpuTime();
  reportf( "#\n" );
  reportf( "# CPU Time used: %g s\n", cpu_time == 0 ? 0 : cpu_time );
#if defined(__linux__)
  uint64_t mem_used = memUsed();
  reportf( "# Memory used: %.3f MB\n",  mem_used == 0 ? 0 : mem_used / 1048576.0 );
#endif
  reportf( "#\n" );
#endif
}
