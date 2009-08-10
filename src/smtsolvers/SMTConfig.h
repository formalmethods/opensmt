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

#ifndef SMTCONFIG_H
#define SMTCONFIG_H

#include "global.h"
#include "SolverTypes.h"

//
// Forwards declarations
//
class TSolver;
//
// Generic configuration
//
struct GConfig
{
  char   stats_file[80];
  int    verbose;
};
//
// SAT Solver configurations
//
struct SConfig
{
  int    theory_propagation;
  double initial_skip_step;
  double skip_step_factor;
  int    restart_first;
  double restart_inc;
  int    use_luby_restart;
  int    learn_up_to_size;
  int    temporary_learn;
  int    preprocess_booleans;
  int    preprocess_theory;
  int    centrality;
  int    trade_off;
  int    verbose;
};
//
// Theory Solver configurations
//
struct TConfig
{
  int disable;             // Disable the solver
  int theory_propagation;  // Enable theory propagation
  int verbose;             // Enable verbosity
  int int_extract_concat;  // Enable interpretation for extraction/concatenation
  int poly_deduct_size;    // Used to define the size of polynomial to be used for deduction; 0 - no deduction for polynomials
  int trade_off;           // Trade-off value for DL preprocessing
};
//
// Holds informations about the configuration of the solver
//
struct SMTConfig
{
  SMTConfig  ( const char * filename_ )
    : filename   ( filename_ )
    , logic      ( UNDEF )
    , status     ( l_Undef )
    , out_flag   ( false )
  {
    // Set Default configuration
    strcpy( gconfig.stats_file, "$filename.stats" );
    gconfig.verbose               = 1;
    satconfig.theory_propagation  = 1;
    satconfig.verbose             = 1;
    satconfig.initial_skip_step   = 1;
    satconfig.skip_step_factor    = 1;
    satconfig.restart_first       = 100;
    satconfig.restart_inc         = 1.1;
    satconfig.use_luby_restart    = 0;
    satconfig.learn_up_to_size    = 0;
    satconfig.temporary_learn     = 1;
    satconfig.preprocess_booleans = 0;
    satconfig.preprocess_theory   = 0;
    satconfig.centrality          = 18;
    satconfig.trade_off           = 8192;
    ufconfig.disable              = 0;
    ufconfig.theory_propagation   = 1;
    ufconfig.verbose              = 0;
    ufconfig.int_extract_concat   = 0;
    bvconfig.disable              = 0;
    bvconfig.theory_propagation   = 1;
    bvconfig.verbose              = 0;
    dlconfig.disable              = 0;
    dlconfig.theory_propagation   = 1;
    dlconfig.verbose              = 0;
    lraconfig.disable             = 0;
    lraconfig.theory_propagation  = 1;
    lraconfig.verbose             = 0;
    lraconfig.poly_deduct_size    = 0;
#ifndef SMTCOMP
    parseConfig( ".opensmtrc" );
#endif
  }

  ~SMTConfig ( ) { }

  const char *  filename;
  logic_t       logic;
  lbool	        status;
  GConfig       gconfig;
  SConfig       satconfig;
  TConfig       ufconfig;
  TConfig       bvconfig;
  TConfig       dlconfig;
  TConfig       lraconfig;

  inline ostream & getOstream( ) { return out_flag ? out_file : cerr; }

private:

  void parseConfig ( const char * );
  void printConfig ( ostream & out );

  bool           out_flag;
  ofstream       out_file;
};

#endif
