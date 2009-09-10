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

#include "SMTConfig.h"

void SMTConfig::parseConfig ( const char * f )
{
  FILE * filein = NULL;
  // Open configuration file
  if ( ( filein = fopen( f, "rt" ) ) == NULL )
  {
    // No configuration file is found. Print out
    // the current configuration
    cerr << "# " << endl;
    cerr << "# WARNING: No configuration file found. Using default setting" << endl;
    cerr << "# WARNING: Dumping default setting to " << f << endl;
    ofstream fileout( f );
    printConfig( fileout );
    fileout.close( );
  }
  else
  {
    int line = 0;

    while ( !feof( filein ) )
    {
      line ++;
      char buf[ 128 ];
      char * res = fgets( buf, 128, filein );
      (void)res;
      // Stop if finished
      if ( feof( filein ) )
	break;
      // Skip comments
      if ( buf[ 0 ] == '#' )
	continue;

      // GENERIC CONFIGURATION
           if ( sscanf( buf, "stats_file %s\n"             , gconfig.stats_file )              == 1 );
      // SAT SOLVER CONFIGURATION                                                              
      else if ( sscanf( buf, "sat_theory_propagation %d\n" , &(satconfig.theory_propagation))  == 1 );
      else if ( sscanf( buf, "sat_verbose %d\n"            , &(satconfig.verbose))             == 1 );
      else if ( sscanf( buf, "sat_initial_skip_step %lf\n" , &(satconfig.initial_skip_step))   == 1 );
      else if ( sscanf( buf, "sat_skip_step_factor %lf\n"  , &(satconfig.skip_step_factor))    == 1 );
      else if ( sscanf( buf, "sat_restart_first %d\n"      , &(satconfig.restart_first))       == 1 );
      else if ( sscanf( buf, "sat_restart_increment %lf\n" , &(satconfig.restart_inc))         == 1 );
      else if ( sscanf( buf, "sat_use_luby_restart %d\n"   , &(satconfig.use_luby_restart))    == 1 );
      else if ( sscanf( buf, "sat_learn_up_to_size %d\n"   , &(satconfig.learn_up_to_size))    == 1 );
      else if ( sscanf( buf, "sat_temporary_learn %d\n"    , &(satconfig.temporary_learn))     == 1 );
      else if ( sscanf( buf, "sat_preprocess_booleans %d\n", &(satconfig.preprocess_booleans)) == 1 );
      else if ( sscanf( buf, "sat_preprocess_theory %d\n"  , &(satconfig.preprocess_theory))   == 1 );
      else if ( sscanf( buf, "sat_centrality %d\n"         , &(satconfig.centrality))          == 1 );
      else if ( sscanf( buf, "sat_trade_off %d\n"          , &(satconfig.trade_off))           == 1 );
      else if ( sscanf( buf, "sat_minimize_conflicts %d\n" , &(satconfig.minimize_conflicts))  == 1 );
      else if ( sscanf( buf, "sat_dump_cnf %d\n"           , &(satconfig.dump_cnf))            == 1 );
      else if ( sscanf( buf, "sat_verbose %d\n"            , &(satconfig.verbose))             == 1 );
      // EUF SOLVER CONFIGURATION                                                              
      else if ( sscanf( buf, "uf_disable %d\n"             , &(ufconfig.disable))              == 1 );
      else if ( sscanf( buf, "uf_theory_propagation %d\n"  , &(ufconfig.theory_propagation))   == 1 );
      else if ( sscanf( buf, "uf_int_extract_concat %d\n"  , &(ufconfig.int_extract_concat))   == 1 );
      else if ( sscanf( buf, "uf_verbose %d\n"             , &(ufconfig.verbose))              == 1 );
      // BV SOLVER CONFIGURATION                                                               
      else if ( sscanf( buf, "bv_disable %d\n"             , &(bvconfig.disable))              == 1 );
      else if ( sscanf( buf, "bv_theory_propagation %d\n"  , &(bvconfig.theory_propagation))   == 1 );
      else if ( sscanf( buf, "bv_verbose %d\n"             , &(bvconfig.verbose))              == 1 );
      // DL SOLVER CONFIGURATION                                                               
      else if ( sscanf( buf, "dl_disable %d\n"             , &(dlconfig.disable))              == 1 );
      else if ( sscanf( buf, "dl_theory_propagation %d\n"  , &(dlconfig.theory_propagation))   == 1 );
      else if ( sscanf( buf, "dl_verbose %d\n"             , &(dlconfig.verbose))              == 1 );
      // LRA SOLVER CONFIGURATION                                                              
      else if ( sscanf( buf, "lra_disable %d\n"            , &(lraconfig.disable))             == 1 );
      else if ( sscanf( buf, "lra_theory_propagation %d\n" , &(lraconfig.theory_propagation))  == 1 );
      else if ( sscanf( buf, "lra_verbose %d\n"            , &(lraconfig.verbose))             == 1 );
      else if ( sscanf( buf, "lra_poly_deduct_size %d\n"   , &(lraconfig.poly_deduct_size))    == 1 );
      else 
      {
	cerr << "# ERROR: unrecognized option " << buf << endl;
	exit( 1 );
      }
    }

    // Close
    fclose( filein );
  }

  string sfile( gconfig.stats_file );

  //
  // Open statistics file if necessary
  //
  if ( sfile == "$stderr"
    || ( filename != NULL && sfile == string("$filename") ) )
  {
    out_flag = false;
  }
  else 
  {
    string fname( sfile );
    string::size_type loc = sfile.find( "$filename", 0 );

    if ( filename != NULL )
    {
      if ( loc != string::npos ) 
	fname.replace( loc, 9, filename );

      out_file.open( fname.c_str( ) );
      out_flag = true;
    }
    else if ( loc == string::npos )
    {
      out_file.open( fname.c_str( ) );
      out_flag = true;
    }
    else
    {
      out_flag = false;
    }
  }
}

void SMTConfig::printConfig ( ostream & out )
{
  out << "#" << endl;
  out << "# OpenSMT Configuration File" << endl;
  out << "# . Options may be written in any order" << endl;
  out << "# . Unrecongnized options will throw an error" << endl;
  out << "# . Use '#' for line comments" << endl;
  out << "# . Remove this file and execute opensmt to generate a new copy with default values" << endl;
  out << "#" << endl;
  out << "# GENERIC CONFIGURATION" << endl;
  out << "#" << endl;
  out << "# Dump statistics to a file. Special values:" << endl
      << "# $stderr       prints statistics on standard error" << endl
      << "# $filename     variable expanded to the input filename" << endl;
  out << "stats_file "             << gconfig.stats_file << endl;
  out << "#" << endl;
  out << "# SAT SOLVER CONFIGURATION" << endl;
  out << "#" << endl;
  out << "# Enables theory propagation" << endl;
  out << "sat_theory_propagation "  << satconfig.theory_propagation << endl;
  out << "# Initial and step factor for theory solver calls" << endl;
  out << "sat_initial_skip_step "   << satconfig.initial_skip_step << endl;
  out << "sat_skip_step_factor "    << satconfig.skip_step_factor << endl;
  out << "# Initial and increment conflict limits for restart" << endl;
  out << "sat_restart_first "       << satconfig.restart_first << endl;
  out << "sat_restart_increment "   << satconfig.restart_inc << endl;
  out << "sat_use_luby_restart "    << satconfig.use_luby_restart << endl;
  out << "# Learn theory-clauses up to the specified size (0 learns nothing)" << endl;
  out << "sat_learn_up_to_size "    << satconfig.learn_up_to_size << endl;
  out << "sat_temporary_learn "     << satconfig.temporary_learn << endl;
  out << "# Preprocess variables and clauses when possible" << endl;
  out << "sat_preprocess_booleans " << satconfig.preprocess_booleans << endl;
  out << "sat_preprocess_theory "   << satconfig.preprocess_theory << endl;
  out << "sat_centrality "          << satconfig.centrality << endl;
  out << "sat_trade_off "           << satconfig.trade_off << endl;
  out << "sat_minimize_conflicts "  << satconfig.minimize_conflicts << endl;
  out << "sat_dump_cnf "            << satconfig.dump_cnf << endl;
  out << "sat_verbose "             << satconfig.verbose << endl;
  out << "#" << endl;
  out << "# EUF SOLVER CONFIGURATION" << endl;
  out << "#" << endl;
  out << "uf_disable "            << ufconfig.disable << endl;
  out << "uf_theory_propagation " << ufconfig.theory_propagation << endl;
  out << "uf_int_extract_concat " << ufconfig.int_extract_concat << endl;
  out << "uf_verbose "            << ufconfig.verbose << endl;
  out << "#" << endl;
  out << "# BITVECTOR SOLVER CONFIGURATION" << endl;
  out << "#" << endl;
  out << "bv_disable "            << bvconfig.disable << endl;
  out << "bv_theory_propagation " << bvconfig.theory_propagation << endl;
  out << "bv_verbose "            << bvconfig.verbose << endl;
  out << "#" << endl;
  out << "# DIFFERENCE LOGIC SOLVER CONFIGURATION" << endl;
  out << "#" << endl;
  out << "dl_disable "            << dlconfig.disable << endl;
  out << "dl_theory_propagation " << dlconfig.theory_propagation << endl;
  out << "dl_verbose "            << dlconfig.verbose << endl;
  out << "#" << endl;
  out << "# LINEAR RATIONAL ARITHMETIC SOLVER CONFIGURATION" << endl;
  out << "#" << endl;
  out << "lra_disable "            << lraconfig.disable << endl;
  out << "lra_theory_propagation " << lraconfig.theory_propagation << endl;
  out << "lra_verbose "            << lraconfig.verbose << endl;
  out << "lra_poly_deduct_size "   << lraconfig.poly_deduct_size << endl;
}
