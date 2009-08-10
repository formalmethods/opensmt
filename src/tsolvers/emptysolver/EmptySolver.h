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

#ifndef EMPTYSOLVER_H
#define EMPTYSOLVER_H

#include "TSolver.h"

class EmptySolver : public OrdinaryTSolver
{
public:

  EmptySolver( const int           i
             , const char *        n
	     , SMTConfig &         c
	     , Egraph &            e
	     , vector< Enode * > & x
	     , vector< Enode * > & d
             , vector< Enode * > & s )
    : OrdinaryTSolver ( i, n, c, e, x, d, s )
  { }

  ~EmptySolver ( ) 
  { }

  lbool               inform              ( Enode * );
  bool                assertLit           ( Enode *, bool = false );
  void                pushBacktrackPoint  ( );
  void                popBacktrackPoint   ( );
  bool                check               ( bool );
  bool                belongsToT          ( Enode * );
};

#endif
