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

#ifndef DL_TOP_LEVEL_PROP_HH
#define DL_TOP_LEVEL_PROP_HH

#include "global.h"
#include "Otl.h"
#include "Egraph.h"

class DLTopLevelProp
{
public:

  DLTopLevelProp ( Egraph & egraph_ )
    : egraph      ( egraph_ )
    , nof_vars    ( 0 )
  { }

  virtual ~DLTopLevelProp( ) { }

  Enode * doit ( Enode * ); // Main routine

private:

  struct DLVar
  {
    DLVar( const int id_, Enode * v_ ) : id ( id_ ), v ( v_ ) { };

    const int id;
    Enode * v;
  };

  unsigned countVariables       ( Enode * );
  bool     topLevelInfo         ( Enode * );
  bool     retrieveUpperBounds  ( Enode * );
  bool     floydWarshall        ( );
  Enode *  simplify             ( Enode * );
  bool     isCase               ( Enode *, vector< Enode * > &, vector< Enode * > &, set< Real > & );
  
  Egraph &          egraph;          // Reference to Egraph
  vector< DLVar * > id_to_dlvar;     // Table enode id --> dlvar
  vector< DLVar * > dlvars;          // DLVar id --> DLVar
  unsigned          nof_vars;        // Counts how many variables needed
  vector< Enode * > top_level_facts; // Stores top-level info that has to be restored

  Real **           upperBounds;    // Holds upperbounds
};

#endif
