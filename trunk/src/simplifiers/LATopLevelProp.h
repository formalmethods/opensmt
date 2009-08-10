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

#ifndef LA_TOP_LEVEL_PROP_HH
#define LA_TOP_LEVEL_PROP_HH

#include "global.h"
#include "Otl.h"
#include "Egraph.h"

class LATopLevelProp
{
public:

  LATopLevelProp ( Egraph & egraph_ )
    : egraph      ( egraph_ )
  { }

  virtual ~LATopLevelProp( ) { }

  Enode * doit ( Enode * ); // Main routine

private:

  bool     retrieveTopLevelFacts  ( Enode * );
  Enode *  simplify               ( Enode * );
  
  Egraph &          egraph;          // Reference to Egraph
  vector< Enode * > top_level_facts; // Stores top-level info that has to be restored
};

#endif
