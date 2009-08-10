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

#ifndef TSEITIN_H
#define TSEITIN_H

#include "global.h"
#include "Otl.h"
#include "SMTSolver.h"
#include "Egraph.h"
#include "Cnfizer.h"

class Tseitin : public Cnfizer
{
public:

  Tseitin( Egraph & egraph_, SMTSolver & solver_, SMTConfig & config_ )
    : Cnfizer( egraph_, solver_, config_ )
  { }

  ~Tseitin( ) { }

private:

  bool cnfize           ( Enode *, Map( int, Enode * ) & ); // Do the actual cnfization
  void cnfizeAnd        ( Enode *, Enode * );               // Cnfize conjunctions
  void cnfizeOr         ( Enode *, Enode * );               // Cnfize disjunctions
  void cnfizeIff        ( Enode *, Enode * );               // Cnfize iffs
  void cnfizeImplies    ( Enode *, Enode * );               // Cnfize implies
  void cnfizeXor        ( Enode *, Enode * );               // Cnfize xors
  void cnfizeIfthenelse ( Enode *, Enode * );               // Cnfize if then elses
};

#endif
