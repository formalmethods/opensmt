/*********************************************************************
 Author: Aliaksei Tsitovich <aliaksei.tsitovich@lu.unisi.ch>
 Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

 OpenSMT -- Copyright (C) 2007, Roberto Bruttomesso

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

#include "LAVar.h"

//
// Stupid gcc doesn't want to have static initialization in the .h file :)
//
int LAVar::columnCount = 0;
int LAVar::rowCount = 0;

Delta LAVar::plusInfBound = Delta( Delta::UPPER );
Delta LAVar::minusInfBound = Delta( Delta::LOWER );

unsigned LAVar::modelGlobalCounter = 1;

//
// Default constructor
//
LAVar::LAVar( Enode * e_orig = NULL )
{
  columnID = columnCount++;
  rowID = -1;
  skip = false;

  // zero as default model
  M1 = new Delta( Delta::ZERO );
  M2 = new Delta( Delta::ZERO );
  modelLocalCounter = 0;

  Enode * e_null = NULL;
  StructBound pb1( &minusInfBound, e_null, false, false );
  StructBound pb2( &plusInfBound, e_null, true, false );
  allBounds.push_back( pb1 );
  allBounds.push_back( pb2 );
  uBound = 1;
  lBound = 0;

  e = e_orig;
}

//
// Constructor with bounds initialization
//
LAVar::LAVar( Enode * e_orig, Enode * e_bound, Enode * e_var, bool basic = false )
{
  columnID = columnCount++;

  if( basic )
    rowID = rowCount++;
  else
    rowID = -1;

  skip = false;

  // zero as default model
  M1 = new Delta( Delta::ZERO );
  M2 = new Delta( Delta::ZERO );
  modelLocalCounter = 0;

  Enode * e_null = NULL;
  StructBound pb1( &minusInfBound, e_null, false, false );
  StructBound pb2( &plusInfBound, e_null, true, false );
  allBounds.push_back( pb1 );
  allBounds.push_back( pb2 );
  uBound = 1;
  lBound = 0;

  e = e_var;
  // set original bounds from Enode
  setBounds( e_orig, e_bound );
}

LAVar::LAVar( Enode * e_orig, Enode * e_var, Real & v, bool revert )
{
  columnID = columnCount++;
  rowID = -1;

  skip = false;

  // zero as default model
  M1 = new Delta( Delta::ZERO );
  M2 = new Delta( Delta::ZERO );
  modelLocalCounter = 0;

  Enode * e_null = NULL;
  StructBound pb1( &minusInfBound, e_null, false, false );
  StructBound pb2( &plusInfBound, e_null, true, false );
  allBounds.push_back( pb1 );
  allBounds.push_back( pb2 );
  uBound = 1;
  lBound = 0;

  e = e_var;

  // set original bounds from Enode
  setBounds( e_orig, v, revert );

}

LAVar::~LAVar( )
{
  // Remove bounds
  while( !allBounds.empty( ) )
  {
    assert( allBounds.back( ).delta );
    if( allBounds.back( ).delta != &minusInfBound && allBounds.back( ).delta != &plusInfBound )
      delete allBounds.back( ).delta;
    allBounds.pop_back( );
  }
  // Remove polynomial coefficients
  for( LARow::iterator it = polynomial.begin( ); it != polynomial.end( ); it++ )
  {
    assert( it->second );
    if( it->first != -1 )
      delete it->second;
    it->second = NULL;
  }
  delete ( M2 );
  delete ( M1 );
}

//TODO: more intelligent value parsing would be nice.
//
// Parse the bound value and the type of the constraint
//
void LAVar::setBounds( Enode * e, Enode * e_bound )
{
  assert( e->isAtom( ) );
  assert( e_bound->isConstant( ) );
  Real v;

  bool revert = false;

  if( !( e->get1st( )->isConstant( ) ) )
    revert = true;

  if( e_bound->isConstant( ) )
    v = e_bound->getCar( )->getValue( );
  else
  {
    error( "unexpected Num: ", e_bound );
    return;
  }
  setBounds( e, v, revert );
}

//
// Reads the type of the bounds from enode type
//
void LAVar::setBounds( Enode * e, Real & v, bool revert )
{

  assert( e->isLeq( ) );

  Delta * bound = NULL;
  Delta * boundRev = NULL;

  Delta::deltaType bound_type = Delta::UPPER;

  if( revert )
  {
    bound = new Delta( v );
    boundRev = new Delta( v, 1 );
  }
  else
  {
    bound = new Delta( v );
    boundRev = new Delta( v, -1 );
    bound_type = Delta::LOWER;
  }

  assert( bound );
  assert( boundRev );
  assert( bound != boundRev );

  // save currently active bounds
  assert( allBounds.size()>1 && uBound < allBounds.size() && lBound < allBounds.size());
  allBounds[uBound].active = true;
  allBounds[lBound].active = true;

  StructBound pb1( bound, e, ( bound_type == Delta::UPPER ), false );
  StructBound pb2( boundRev, e, ( bound_type != Delta::UPPER ), true );

  allBounds.push_back( pb1 );
  allBounds.push_back( pb2 );

  //TODO: Instead of sorting all bounds after insertion,
  //      I should check if it fits on left(right) of current pointers and sort only there
  sortBounds();

  // restore lower bound
  if( allBounds[lBound].active )
  {
    allBounds[lBound].active = false;
  }
  else
  {
    for( unsigned i = 0; i < allBounds.size( ); i++ )
    {
      if( !allBounds[i].boundType && allBounds[i].active )
      {
        lBound = i;
        allBounds[i].active = false;
        break;
      }
    }
  }

  // restore upper bound
  if( allBounds[uBound].active )
  {
    allBounds[uBound].active = false;
  }
  else
  {
    for( int i = allBounds.size( ) - 1; i >= 0; i-- )
    {
      if( allBounds[i].boundType && allBounds[i].active )
      {
        uBound = i;
        allBounds[uBound].active = false;
        break;
      }
    }
  }
}

void LAVar::getDeducedBounds( const Delta& c, bool upper, vector<Enode *>& dst, int solver_id )
{
  if( upper )
  {
    int it = uBound - 1;
    while( ( *( allBounds[it].delta ) ) >= c ) //&& M() < (*( allBounds[it].delta ))
    {
      if( allBounds[it].boundType && !allBounds[it].e->hasPolarity( ) && !allBounds[it].e->isDeduced( ) )
      {
        allBounds[it].e->setDeduced( ( allBounds[it].reverse ? l_False : l_True ), solver_id );
        dst.push_back( allBounds[it].e );
      }
      it--;
    }
  }
  else
  {
    int it = lBound + 1;
    while( ( *( allBounds[it].delta ) ) <= c )
    {
      if( !allBounds[it].boundType && !allBounds[it].e->hasPolarity( ) && !allBounds[it].e->isDeduced( ) )
      {
        allBounds[it].e->setDeduced( ( allBounds[it].reverse ? l_False : l_True ), solver_id );
        dst.push_back( allBounds[it].e );
      }
      it++;
    }
  }
}

void LAVar::getSimpleDeductions( vector<Enode *>& dst, bool upper, int solver_id )
{
  if( !upper && !allBounds[lBound].delta->isInf( ) )
  {
    assert( lBound > 0 );
    for( int it = lBound - 1; it > 0; it-- )
    {
      if( !allBounds[it].boundType && !allBounds[it].e->hasPolarity( ) && !allBounds[it].e->isDeduced( ) )
      {
        allBounds[it].e->setDeduced( ( allBounds[it].reverse ? l_False : l_True ), solver_id );
        dst.push_back( allBounds[it].e );
//        cout  << "Deduced from lower " << allBounds[it].e << endl;
      }
    }
  }

  if( upper && !allBounds[uBound].delta->isInf( ) )
  {
    for( int it = uBound + 1; it < static_cast< int >( allBounds.size( ) ) - 1; it++ )
    {
      if( allBounds[it].boundType && !allBounds[it].e->hasPolarity( ) && !allBounds[it].e->isDeduced( ) )
      {
        allBounds[it].e->setDeduced( ( allBounds[it].reverse ? l_False : l_True ), solver_id );
        dst.push_back( allBounds[it].e );
//        cout  << "Deduced from upper as " << (allBounds[it].reverse ? "FALSE " : "TRUE ") << allBounds[it].e << endl;
      }
    }
  }
}

void LAVar::getSuggestions( vector<Enode *>& dst, int solver_id )
{
  (void)solver_id;
  if( M( ) > U( ) )
  {
    allBounds[uBound].e->setDecPolarity( allBounds[uBound].reverse );
    dst.push_back( allBounds[uBound].e );
  }
  else if( M( ) < L( ) )
  {
    allBounds[lBound].e->setDecPolarity( allBounds[lBound].reverse );
    dst.push_back( allBounds[lBound].e );
  }
}

//TODO:: Can I do better here? Iterate from different sides? - YES

unsigned LAVar::getIteratorByEnode( Enode * _e, bool _reverse )
{
  unsigned it;
  it = allBounds.size( ) - 2;
  assert( it != 0 );
  while( it > 0 && !( allBounds[it].e == _e && allBounds[it].reverse == _reverse ) )
    it--;
  assert( it != 0 ); // we assume Enode is in!
  return it;
}

void LAVar::sortBounds( )
{
  sort( allBounds.begin( ), allBounds.end( ), structBounds_ptr_cmp( ) );

  uBound = allBounds.size( ) - 1;
  lBound = 0;

}

void LAVar::printBounds( )
{
  cerr << endl << this << " | ";
  for( unsigned i = 0; i < allBounds.size( ); i++ )
    cerr << *( allBounds[i].delta ) << ( allBounds[i].boundType ? "[U]" : "[L]" ) << ( allBounds[i].reverse ? "rev" : "" ) << " ";
}
