#ifndef TABLEU_H_
#define TABLEU_H_

#include "global.h"
#include "Otl.h"

#define TableuRow map<int, Real>
#define TableuCell pair<u_int, u_int>

//TODO: check if current storage is effective for the task, 2-vector storage might be better


//
// Class to keep the data of the Simplex method tableu (sparse matrix)
//
class Tableu
{
  map<TableuCell, Real> matrix; // tableu storage

public:

  Tableu(); // default constructor
  void addRow(TableuRow &); // Adds a row correspondent to a constraint
  void setAij(u_int, u_int, Real); // Sets A[i][j]
  Real getAij(u_int, u_int); // Reads A[i][j]

  u_int rowsTotal; // Total number of rows in the Tableu
};

#endif /*TABLEU_H_*/
