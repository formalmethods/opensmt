/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@gmail.com>
      , Aliaksei Tsitovich <aliaksei.tsitovich@usi.ch>

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

#ifndef LACANONIZER_H
#define LACANONIZER_H

#include "global.h"
#include "Otl.h"
#include "Egraph.h"

typedef map<int, Real> map_int2Real;

class LACanonizer
{
public:

  LACanonizer( Egraph & egraph_ )
    : egraph  ( egraph_ ), tmp_count(0)
  { }

  ~LACanonizer( ) { }

  Enode * canonize( Enode * );

private:

  Enode * canonize_atom( Enode * );
  Egraph & egraph;

  void multiply_vars(map_int2Real & vars, const Real value);
  void merge_vars(map_int2Real & dst, map_int2Real & src);
  void canonize_atom_rec(Enode * atom, map_int2Real & vars);

  map< enodeid_t, Enode* > vars_hash;

  uint tmp_count;
};

#endif
