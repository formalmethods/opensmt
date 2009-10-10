/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@gmail.com>

OpenSMT -- Copyright (C) 2009 Roberto Bruttomesso

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
#include "SimpSMTSolver.h"
#include "Tseitin.h"
#include "ExpandITEs.h"
#include "BVBooleanize.h"
#include "TopLevelProp.h"
#include "DLRescale.h"
#include "Ackermanize.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <csignal>

#if defined(__linux__)
#include <fpu_control.h>
#endif

void        loadCustomSettings ( SMTConfig & );
void        printResult        ( const lbool &, const lbool & = l_Undef );
void        catcher            ( int );
extern int  smtset_in          ( FILE * );
extern int  smtparse           ( );
extern int  ysset_in           ( FILE * );
extern int  ysparse            ( );
Egraph *    parser_egraph;
SMTConfig * parser_config;
extern bool stop;
bool        verbose;

/*****************************************************************************\
 *                                                                           *
 *                                  MAIN                                     *
 *                                                                           *
\*****************************************************************************/

int main( int argc, char * argv[] )
{
  // Catch SigTerm, so that it answers even on ctrl-c
  signal( SIGTERM, catcher );
  signal( SIGINT , catcher );
  stop = false;

  // Allocate configuration
#ifdef STATISTICS
  SMTConfig config( argc == 1 ? NULL : argv[1] );
#else
  SMTConfig config( NULL );
#endif
  // Allocates the egraph
  Egraph egraph( config );
  // Parse the input formula
  parser_egraph = &egraph;
  parser_config = &config;
  // Accepts file from stdin if nothing specified
  FILE * fin = NULL;

  if ( argc > 2 )
  {
    cerr << "#" << endl
         << "# Usage: " << endl
         << "# \t" << argv[0] << " filename.smt" << endl;
    exit( 1 );
  }

  if ( argc == 1 )
  {
    fin = stdin;
  }
  else if ( (fin = fopen( argv[ 1 ], "rt" )) == NULL )
  {
    error( "can't open file ", argv[ 1 ] );
  }

  // Parse
#ifdef SMTCOMP
  smtset_in( fin );
  smtparse( );
#else
  // Parse according to filetype
  if ( fin == stdin )
  {
    smtset_in( fin );
    smtparse( );
  }
  else
  {
    const char * extension = strrchr( argv[ 1 ], '.' );
    if ( strcmp( extension, ".smt" ) == 0 )
    {
      smtset_in( fin );
      smtparse( );
    }
    else if ( strcmp( extension, ".ys" ) == 0 )
    {
      ysset_in( fin );
      ysparse( );
      config.logic = QF_IDL;
    }
    else
    {
      error( "unknown file extension. Please use .smt or .ys or stdin", "" );
    }
  }
  
#endif

#ifndef SMTCOMP
  bool print_sharp = false;
  verbose = config.satconfig.verbose;
  if ( verbose )
  {
    const int len_pack = strlen( PACKAGE_STRING );
    const char * site = "http://verify.inf.usi.ch/opensmt";
    const int len_site = strlen( site );

    cerr << "#" << endl
         << "# -------------------------------------------------------------------------" << endl
         << "# " << PACKAGE_STRING;

    for ( int i = 0 ; i < 73 - len_site - len_pack ; i ++ )
      cerr << " ";

    cerr << site << endl
         << "# -------------------------------------------------------------------------" << endl
         << "#" << endl;
  }
#endif
  //
  // This trick (copied from Main.C of MiniSAT) is to allow
  // the repeatability of experiments that might be compromised
  // by the floating point unit approximations on doubles
  //
#if defined(__linux__) && !defined( SMTCOMP )
  fpu_control_t oldcw, newcw;
  _FPU_GETCW(oldcw); newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE; _FPU_SETCW(newcw);
  // reportf("# WARNING: for repeatability, setting FPU to use double precision\n");
  // print_sharp = true;
#endif

#ifdef PEDANTIC_DEBUG
  reportf("# WARNING: pedantic assertion checking enabled (very slow)\n");
  print_sharp = true;
#endif

#ifdef EXTERNAL_TOOL
  reportf("# WARNING: external tool checking enabled (very slow)\n");
  print_sharp = true;
#endif

#ifndef OPTIMIZE
  reportf( "# WARNING: this binary is compiled with optimizations disabled (slow)\n" );
  print_sharp = true;
#endif

#ifndef SMTCOMP
  if ( print_sharp && verbose ) cerr << "#" << endl;
#endif

#ifdef SMTCOMP
  loadCustomSettings( config );
#endif

  assert( config.ufconfig.int_extract_concat == 0
       || config.logic == QF_BV );

  fclose( fin );

  // Retrieve the formula
  Enode * formula = egraph.getFormula( );

  if ( formula == NULL )
    error( "formula undefined", "" );

  if ( config.logic == UNDEF )
    error( "unable to determine logic", "" );

  // Ackermanize away functional symbols
  if ( config.logic == QF_UFIDL
    || config.logic == QF_UFLRA )
  {
    Ackermanize ackermanizer( egraph, config );
    formula = ackermanizer.doit( formula );
  }

  // Artificially create a boolean
  // abstraction, if necessary
  if ( config.logic == QF_BV )
  {
    BVBooleanize booleanizer( egraph, config );
    formula = booleanizer.doit( formula );
  }

  // Removes ITEs if there is any
  if ( egraph.hasItes( ) )
  {
    ExpandITEs expander( egraph, config );
    formula = expander.doit( formula );
  }

  // Top-Level Propagator. It also canonize atoms
  TopLevelProp propagator( egraph, config );
  formula = propagator.doit( formula );

  // Convert RDL into IDL, also compute if GMP is needed
  if ( config.logic == QF_RDL 
    || config.logic == QF_IDL )
  {
    DLRescale rescaler( egraph, config );
    formula = rescaler.doit( formula );
  }

  lbool result = l_Undef;

  // Solve only if not simplified already
  if ( formula->isTrue( ) )
  {
    result = l_True;
    printResult( result, config.status );
  }
  else if ( formula->isFalse( ) )
  {
    result = l_False;
    printResult( result, config.status );
  }
  else
  {
    // Initializes theory solvers
    egraph.initializeTheorySolvers( );

    // Allocates SMTSolver based on MiniSAT
    SimpSMTSolver solver( egraph, config );

    // Allocates Tseitin-like cnfizer
    Tseitin cnfizer( egraph, solver, config );

    // Compute polarities
    egraph.computePolarities( formula );

    // CNFize the input formula and feed clauses to the solver
    result = cnfizer.cnfizeAndGiveToSolver( formula );

    // Solve
    if ( result == l_Undef )
      result = solver.smtSolve( config.satconfig.preprocess_booleans != 0
	                     || config.satconfig.preprocess_theory   != 0 );

    // If computation has been stopped, return undef
    if ( stop ) result = l_Undef;

    // Prints the result and check against status
    printResult( result, config.status );
  }

  // Prints the model on a file (currently disabled)
  // if ( result == l_True ) S.printModel( file );

  return 0;
}

void catcher(int sig)
{
  switch (sig)
  {
    case SIGINT:
    case SIGTERM:
      if ( stop )
      {
#ifndef SMTCOMP
	if ( verbose )
	{
	  reportf("\n# ----------+--------------------------+----------+------------+-----------\n");
	  reportf("#\n");
	}
#endif
	printResult( l_Undef );
	exit( 1 );
      }
      stop = true;
      break;
  }
}

void printResult( const lbool & result, const lbool & config_status )
{
#ifndef SMTCOMP
  fflush( stderr );

  //
  // For testing purposes we return error if bug is found
  //
  if ( config_status != l_Undef
    && result != l_Undef
    && result != config_status )
    cout << "error" << endl;
  else
#endif
  (void)config_status;
  if ( result == l_True )
    cout << "sat" << endl;
  else if ( result == l_False )
    cout << "unsat" << endl;
  else if ( result == l_Undef )
    cout << "unknown" << endl;
  else
    error( "unexpected result", "" );

  fflush( stdout );

#ifndef SMTCOMP
  if ( verbose )
  {
    //
    // Statistics
    //
    double   cpu_time = cpuTime();
    reportf( "#\n" );
    reportf( "# CPU Time used: %g s\n", cpu_time == 0 ? 0 : cpu_time );
    uint64_t mem_used = memUsed();
    reportf( "# Memory used: %.3f MB\n",  mem_used == 0 ? 0 : mem_used / 1048576.0 );
    reportf( "#\n" );
  }
#endif
}

void loadCustomSettings( SMTConfig & config )
{
  if ( config.logic == QF_UF
    || config.logic == QF_BV )
  {
    config.satconfig.preprocess_booleans = 1;
  }
  else if ( config.logic == QF_LRA )
  {
    config.lraconfig.theory_propagation = 0;
  }
  else if ( config.logic == QF_IDL )
  {
    config.satconfig.preprocess_booleans = 1;
    config.satconfig.preprocess_theory = 1;
  }
}
