/*********************************************************************
 Author: Aliaksei Tsitovich <aliaksei.tsitovich@usi.ch>
 , Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

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

#ifndef LAROW_H_
#define LAROW_H_

#include "global.h"
#include "Enode.h"
#include "Delta.h"

//
// The class to store the rows and columns of the simplex tableau.
//
class LARow: public list<pair<int, Real *> >
{
private:
  vector<bool> is_there; // used to perform the fast check if the value is in the row

public:
  LARow::iterator find( int key );              // find the iterator by the key
  void erase( int key );                        // delete the element form the row by key
  void erase( LARow::iterator it );             // delete the element form the row by iterator
  void clear( );                                // clear the row;
  void assign( const int & key, Real * a );     // assign the value by key
  void assign( LARow::iterator it, Real * a );  // assign the value by iterator
};

#endif /* LAROW_H_ */
