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

#ifndef TSOLVER_H
#define TSOLVER_H

#include "Enode.h"
#include "SMTConfig.h"
#include "SolverTypes.h"

class Egraph;

#ifdef STATISTICS
struct TSolverStats
{
  TSolverStats ( )
    : sat_calls         ( 0 )
    , uns_calls         ( 0 )
    , conflicts_sent    ( 0 )
    , avg_conf_size     ( 0 )
    , max_conf_size     ( 0 )
    , min_conf_size     ( 32767 )
    , deductions_done   ( 0 )
    , deductions_sent   ( 0 )
    , reasons_sent      ( 0 )
    , avg_reas_size     ( 0 )
    , max_reas_size     ( 0 ) 
    , min_reas_size     ( 32767 )
  { }

  // Statistics for theory solvers
  void printStatistics ( ostream & os )
  {
    os << "# Satisfiable calls........: " << sat_calls << endl;
    os << "# Unsatisfiable calls......: " << uns_calls << endl;
    if ( uns_calls > 0 )
    {
      os << "# Conflicts sent...........: " << conflicts_sent << endl;
      if ( conflicts_sent > 0 )
      {
	os << "# Average conflict size....: " << avg_conf_size / (float)conflicts_sent << endl;
	os << "# Max conflict size........: " << max_conf_size << endl;
	os << "# Min conflict size........: " << min_conf_size << endl;
      }
    }
    if ( sat_calls > 0 )
    {
      os << "# Deductions done..........: " << deductions_done << endl;
      os << "# Deductions sent..........: " << deductions_sent << endl;
      os << "# Reasons sent.............: " << reasons_sent << endl;
      if ( reasons_sent > 0 )
      {
	os << "# Average reason size......: " << avg_reas_size / (float)reasons_sent << endl;
	os << "# Max reason size..........: " << max_reas_size << endl;
	os << "# Min reason size..........: " << min_reas_size << endl;
      }
    }
  }

  // Calls statistics
  long  sat_calls;
  long  uns_calls;
  // Conflict statistics
  int   conflicts_sent;
  float avg_conf_size;
  int   max_conf_size;
  int   min_conf_size;
  // Deductions statistics
  int   deductions_done;
  int   deductions_sent;
  int   reasons_sent;
  float avg_reas_size;
  int   max_reas_size;
  int   min_reas_size;
};
#endif

class TSolver
{
public:

  TSolver ( const int i, const char * n, SMTConfig & c )
    : id     ( i )
    , name   ( n )
    , config ( c )
  { }

  virtual ~TSolver ( ) { }

  virtual lbool               inform              ( Enode * )               = 0; // Inform the solver about the existence of a theory atom
  virtual bool                assertLit           ( Enode *, bool = false ) = 0; // Assert a theory literal
  virtual void                pushBacktrackPoint  ( )                       = 0; // Push a backtrack point
  virtual void                popBacktrackPoint   ( )                       = 0; // Backtrack to last saved point
  virtual bool                check               ( bool )                  = 0; // Check satisfiability

  inline const string &       getName             ( ) { return name; }

protected:
  
  const int           id;               // Id of the solver
  const string        name;             // Name of the solver
  SMTConfig &         config;           // Reference to configuration
  vector< size_t >    backtrack_points; // Keeps track of backtrack points
};

class OrdinaryTSolver : public TSolver
{
public:

  OrdinaryTSolver ( const int           i
                  , const char *        n
		  , SMTConfig &         c
		  , Egraph &            e 
		  , vector< Enode * > & x
		  , vector< Enode * > & d
		  , vector< Enode * > & s )
    : TSolver     ( i, n, c )
    , egraph      ( e )
    , explanation ( x )
    , deductions  ( d )
    , suggestions ( s )
  { }

  virtual ~OrdinaryTSolver ( )
  { }

  virtual bool belongsToT   ( Enode * ) = 0; // Atom belongs to this theory
  virtual void computeModel ( )         = 0; // Compute model for variables

protected:

  Egraph &            egraph;      // Reference to egraph
  vector< Enode * > & explanation; // Stores the explanation
  vector< Enode * > & deductions;  // List of deductions
  vector< Enode * > & suggestions; // List of suggestions for decisions
};

class CoreTSolver : public TSolver
{
public:

  CoreTSolver ( const int    i
              , const char * n
	      , SMTConfig &  c )
    : TSolver         ( i, n, c )
    , deductions_next ( 0 )
  { }

  virtual ~CoreTSolver ( ) 
  { }

  virtual vector< Enode * > & getConflict  ( bool = false ) = 0; // Return conflict
  virtual Enode *             getDeduction ( )              = 0; // Return an implied node based on the current state

protected:

  vector< OrdinaryTSolver * > tsolvers;            // List of ordinary theory solvers
#ifdef STATISTICS
  vector< TSolverStats * >    tsolvers_stats;      // Statistical info for tsolvers
#endif
  vector< Enode * >           explanation;         // Stores the explanation
  vector< Enode * >           deductions;          // List of deductions
  size_t                      deductions_next;     // Index of next deduction to communicate
  vector< size_t >            deductions_lim;      // Keeps track of deductions done up to a certain point
  vector< size_t >            deductions_last;     // Keeps track of deductions done up to a certain point
  vector< Enode * >           suggestions;         // List of suggestions for decisions
};

#endif