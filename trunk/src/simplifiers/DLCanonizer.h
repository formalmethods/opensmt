/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@gmail.com>
      , Aliaksei Tsitovich  <aliaksei.tsitovich@usi.ch>
      , Edgar Pek           <edgar.pek@usi.ch>

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

#ifndef DLCANONIZER_H
#define DLCANONIZER_H

#include "global.h"
#include "Otl.h"
#include "Egraph.h"

class DLCanonizer
{
public:

  DLCanonizer( Egraph & egraph_, SMTConfig & config_ ) 
    : egraph            ( egraph_ ) 
    , config            ( config_ ) 
    , curr_constant_sum ( 0 )
  { }

  ~DLCanonizer( ) { }

  Enode * canonize( Enode * );
  Enode * rescale ( Enode * );

private:

  typedef map< int, Real > map_int2Real;

  Enode * canonize_atom     ( Enode * );
  void    multiply_vars     ( map_int2Real &, const Real );
  void    merge_vars        ( map_int2Real &, map_int2Real & );
  void    canonize_atom_rec ( Enode * atom, map_int2Real & );

  vector< Enode * >  additional_clauses; 
  map< int, Enode* > vars_hash;
  Egraph &           egraph;
  SMTConfig &        config;
  Real               curr_constant_sum;
};

#endif
