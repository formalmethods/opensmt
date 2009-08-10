#include "Tableu.h"

//
// Default constructor
//
Tableu::Tableu()
{
  rowsTotal=0;
}

//
// Returns value of A[i][j]
//
Real Tableu::getAij(u_int i, u_int j)
{
  map<TableuCell, Real>::const_iterator c_it = matrix.find(TableuCell(i,j));
  if (c_it!=matrix.end())
    return c_it->second;
  else
    return 0;
}

//
// Sets the value of A[i][j]
//
void Tableu::setAij(u_int i, u_int j, Real v)
{
  TableuCell cell(i, j);
  map<TableuCell, Real>::iterator c_it = matrix.find(cell);
  if (v == 0)
    matrix.erase(cell);
  else if (c_it!=matrix.end())
    c_it->second = v;
  else
    matrix[cell] = v;
}

//
// Inserts a row A[i] correspondent to the constraint
//
void Tableu::addRow(TableuRow & row)
{
  for (TableuRow::const_iterator it = row.begin(); it != row.end(); ++it)
    setAij(rowsTotal, it->first, it->second);
 
  rowsTotal++;
 }
