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

#ifndef TOP_LEVEL_PROP_HH
#define TOP_LEVEL_PROP_HH

#include "global.h"
#include "Otl.h"
#include "Egraph.h"

class TopLevelProp
{
public:

  TopLevelProp ( Egraph & egraph_, SMTConfig & config_ )
    : egraph  ( egraph_ )
    , config  ( config_ )
  { }

  virtual ~TopLevelProp( ) { }

  Enode * doit ( Enode * ); // Main routine

private:

  typedef map< int, Real > map_int2Real;

  /*
  void    retrieveDistinctions            ( Enode * );
  void    retrieveNe                      ( Enode * );
  */
  void    retrieveSubstitutions           ( Enode *, Map( enodeid_t, Enode * ) &, Map( enodeid_t, bool ) & );
  bool    contains                        ( Enode *, Enode * );
  Enode * substitute                      ( Enode *, Map( enodeid_t, Enode * ) &, Map( enodeid_t, bool ) &, bool & );
  Enode * normalizeDLAtom                 ( Enode * );
  void    normalizeDLAtomRec              ( Enode *, map_int2Real & );
  void    multiplyVars                    ( map_int2Real &, const Real );
  void    mergeVars                       ( map_int2Real &, map_int2Real & );
  Enode * learnTransitivity               ( Enode * );
  Enode * simplifyTwinEqualities          ( Enode *, bool & );
  Enode * propagateUnconstrainedVariables ( Enode *, bool & );
  Enode * replaceUnconstrainedTerms       ( Enode *, vector< int > & , bool & );
  void    computeIncomingEdges            ( Enode *, vector< int > & );
  
  map< enodeid_t, Enode* > vars_hash;

  Egraph &                       egraph;               // Reference to Egraph
  SMTConfig &                    config;               // Reference to Config

  map< enodeid_t, int >          enode_to_dist_index;  
  vector< Enode * >              distinctions;
  vector< Enode * >              ne;
};

#endif
