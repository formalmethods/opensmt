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

#ifndef SMTCONFIG_H
#define SMTCONFIG_H

#include "global.h"
#include "SolverTypes.h"

//
// Forwards declarations
//
class TSolver;
// 
// SAT Solver configurations
//
struct SConfig
{
  int theory_propagation;
  int verbose;
};
// 
// Theory Solver configurations
//
struct TConfig
{
  int theory_propagation;  // Enable theory propagation
  int verbose;             // Enable verbosity
  int int_extract_concat;  // Enable interpretation for extraction/concatenation
};
//
// Holds informations about the configuration of the solver
//
struct SMTConfig
{
  SMTConfig  ( const logic_t logic_
             , const lbool status_ )
    : logic  ( logic_ )
    , status ( status_ )
  { 
    // Set Default configuration
    satconfig.theory_propagation = 1;
    satconfig.verbose            = 0;
    ufconfig.theory_propagation  = 1;
    ufconfig.verbose             = 0;
    ufconfig.int_extract_concat  = 0;
    bvconfig.theory_propagation  = 0;
    bvconfig.verbose             = 0;
    lraconfig.theory_propagation = 0;
    lraconfig.verbose            = 0;
    readConfig( "config.cfg" );
  }

  ~SMTConfig ( ) { }

  logic_t  logic; 
  lbool	   status;
  SConfig  satconfig;
  TConfig  ufconfig;
  TConfig  bvconfig;
  TConfig  lraconfig;

private:

  //
  // It is possible to read configuration from file
  // (experimental, not completely working)
  //
  void readConfig ( const char * filename )
  {
    FILE * file = NULL;
    // Open configuration file. Do nothing if no configuration is found
    if ( ( file = fopen( filename, "rt" ) ) == NULL )
      return;

    // Read SAT Solver Configuration
    fscanf( file, "SAT Solver Configuration\n" );
    fscanf( file, "Theory propagation....: %d\n", &(satconfig.theory_propagation) );
    fscanf( file, "Verbose...............: %d\n", &(satconfig.verbose) );
    // Read UF Solver Configuration
    fscanf( file, "UF Solver Configuration\n" );
    fscanf( file, "Theory propagation....: %d\n", &(ufconfig.theory_propagation) );
    fscanf( file, "Verbose...............: %d\n", &(ufconfig.verbose) );
    fscanf( file, "Int extract concat....: %d\n", &(ufconfig.int_extract_concat) );
    // Read BV Solver Configuration
    fscanf( file, "BV Solver Configuration\n" );
    fscanf( file, "Theory propagation....: %d\n", &(bvconfig.theory_propagation) );
    fscanf( file, "Verbose...............: %d\n", &(bvconfig.verbose) );
    // Read LRA Solver Configuration
    fscanf( file, "LRA Solver Configuration\n" );
    fscanf( file, "Theory propagation....: %d\n", &(lraconfig.theory_propagation) );
    fscanf( file, "Verbose...............: %d\n", &(lraconfig.verbose) );

    // Close
    fclose( file );
  }
};

#endif
