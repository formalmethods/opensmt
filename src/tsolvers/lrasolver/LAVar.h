/*********************************************************************
 Author: Aliaksei Tsitovich <aliaksei.tsitovich@lu.unisi.ch>
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

#ifndef LAVAR_H_
#define LAVAR_H_

#include "global.h"
#include "Enode.h"
#include "Delta.h"
#include "LARow.h"
//
// Class to store the term of constraints as a column of Simplex method tableau
//
class LAVar
{
public:
  struct StructBound
  {
    Enode * e;
    Delta * delta;
    bool boundType;
    bool reverse;
    bool active;

    StructBound( Delta * _delta, Enode * _e, bool _boundType, bool _reverse )
    {
      delta = _delta;
      e = _e;
      boundType = _boundType;
      reverse = _reverse;
      active = false;
    }

    inline friend bool operator==( const StructBound &a, const StructBound &b )
    {
      if( ( a.e == b.e ) && ( a.delta == b.delta ) && ( a.boundType == b.boundType ) && ( a.reverse == b.reverse ) )
        return true;
      else
        return false;
    }
  };

protected:
  struct structBounds_ptr_cmp
  {
    bool operator()( StructBound lhs, StructBound rhs )
    {
      assert( lhs.delta );
      assert( rhs.delta );
      if( lhs == rhs )
        return true;
      else if( *( lhs.delta ) != *( rhs.delta ) )
        return *( lhs.delta ) < *( rhs.delta );
      else
      {
        if( lhs.boundType != rhs.boundType )
          return rhs.boundType;
        else
        {
          // if this assertion fails then you have duplicates in the bounds list. Check the canonizer.
          assert( rhs.reverse != lhs.reverse );
          return rhs.reverse;
        }
      }
    }
  };

  typedef vector<StructBound> VectorBounds;

protected:
  int columnID; // ID (column number) for LAVar
  int rowID; // rowID (row number) for LAVar. For public known as basicID :)

  static int columnCount; // Static counter to create ID for LAVar
  static int rowCount; // Static counter for rows keep track of basic variables

  static Delta plusInfBound; //
  static Delta minusInfBound; //

  Delta * M1;
  Delta * M2;

  static unsigned modelGlobalCounter;

  unsigned modelLocalCounter;

public:

  Enode * e; //pointer to original Enode. In case of slack variable points to polynome.

  LARow polynomial;
  //  setRows bindedRows; // original bounds binded to a constraint id.

  LARow bindedRows; // original bounds binded to a constraint id.

  bool skip;

  VectorBounds allBounds;

  unsigned uBound;
  unsigned lBound;

  void setBounds( Enode * e, Enode * e_bound ); // Set the bounds from Enode of original constraint (used on reading/construction stage)
  void setBounds( Enode * e, Real & v, bool revert ); // Set the bounds according to enode type and a given value (used on reading/construction stage)

  LAVar( Enode * e_orig ); // Default constructor
  LAVar( Enode * e_orig, Enode * e_bound, Enode * e_var, bool basic ); // Constructor with bounds
  LAVar( Enode * e_orig, Enode * e_var, Real & v, bool revert ); // Constructor with bounds from real

  virtual ~LAVar( );

  void sortBounds( );
  void printBounds( );

  inline bool isBasic( ); // Checks if current LAVar is Basic in current solver state
  inline bool isNonbasic( ); // Checks if current LAVar is NonBasic in current solver state

  inline bool isModelOutOfBounds( ); // Check if current Model for LAVar does not feat into the bounds.
  inline const Delta& outOfBoundDistance( );
  inline bool isUnbounded( ); // Check if LAVar has no bounds at all (it can be eliminated if possible).

  void getDeducedBounds( const Delta& c, bool upper, vector<Enode *>& dst, int solver_id );
  void getDeducedBounds( bool upper, vector<Enode *>& dst, int solver_id )
  {
    getDeducedBounds( upper ? U( ) : L( ), upper, dst, solver_id );
  }
  ;

  void getSuggestions( vector<Enode *>& dst, int solver_id );

  void getSimpleDeductions( vector<Enode *>& dst, bool upper, int solver_id );

  unsigned getIteratorByEnode( Enode * e, bool );

  inline int ID( ); // Returns the ID of the LAVar
  inline int basicID( ); // Returns the basicID (row id) of the basic LAVar (-1 if it is Nonbasic)

  inline void setNonbasic( ); //Makes LAVar Nonbasic
  inline void setBasic( int row ); // Makes LAVar Basic and set the row number it corresponds

  inline void bindRow( int row, Real * a );
  inline void unbindRow( int row );

  inline void saveModel( );
  inline void restoreModel( );
  static inline void saveModelGlobal( );

  inline const Delta & U( ); // The latest upper bound of LAVar (+inf by default)
  inline const Delta & L( ); // The latest lower bound of LAVar (-inf by default)
  inline const Delta & M( ); // The latest model of LAVar (0 by default)

  inline void incM( const Delta &v );
  inline void setM( const Delta &v );

  inline friend ostream & operator <<( ostream & out, LAVar * v )
  {
    if( v->e->isVar( ) )
      out << v->e;
    else
      out << "s" << v->ID( );

    return out;
  }

  inline friend ostream & operator <<( ostream & out, LAVar & v )
  {
    out << &v;
    return out;
  }
};

//
// Stupid gcc wants all inline functions in .h file :(
//

bool LAVar::isBasic( )
{
  return ( rowID != -1 );
}

bool LAVar::isModelOutOfBounds( )
{
  return ( M( ) > U( ) || M( ) < L( ) );
}

const Delta& LAVar::outOfBoundDistance( )
{
  return ( std::min( U( ) - M( ), M( ) - L( ) ) );
}

bool LAVar::isUnbounded( )
{
  return allBounds.size( ) < 3;
}

bool LAVar::isNonbasic( )
{
  return !isBasic( );
}

int LAVar::ID( )
{
  return columnID;
}

int LAVar::basicID( )
{
  return rowID;
}

void LAVar::setNonbasic( )
{
  rowID = -1;
}

void LAVar::setBasic( int row )
{
  rowID = row;
}

void LAVar::bindRow( int row, Real * a )
{
  assert( this->bindedRows.find( row ) == this->bindedRows.end( ) );
  this->bindedRows.assign( row, a );
}

void LAVar::unbindRow( int row )
{
  assert( this->bindedRows.find( row ) != this->bindedRows.end( ) || this->isBasic( ) );
  this->bindedRows.erase( row );
}

void LAVar::saveModel( )
{
  *M2 = *M1;
  modelLocalCounter = modelGlobalCounter;
}

void LAVar::saveModelGlobal( )
{
  modelGlobalCounter++;
}

void LAVar::restoreModel( )
{
  if( modelLocalCounter == modelGlobalCounter )
  {
    *M1 = *M2;
    modelLocalCounter--;
  }
}

const Delta & LAVar::U( )
{
  assert( allBounds[uBound].delta );
  return *( allBounds[uBound].delta );
}

const Delta & LAVar::L( )
{
  assert( allBounds[lBound].delta );
  return *( allBounds[lBound].delta );
}

const Delta & LAVar::M( )
{
  return ( *M1 );
}

void LAVar::incM( const Delta &v )
{
  setM( M( ) + v );
}

void LAVar::setM( const Delta &v )
{
  if( modelLocalCounter != modelGlobalCounter )
    saveModel( );
  ( *M1 ) = v;
}

#endif /*LAVAR_H_*/
