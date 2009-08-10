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

#include "LACanonizer.h"

Enode * LACanonizer::canonize( Enode * formula )
{
  assert( formula );

  egraph.initDupMap( );

  vector<Enode *> unprocessed_enodes; // Stack for unprocessed enodes
  unprocessed_enodes.push_back( formula ); // formula needs to be processed
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * enode = unprocessed_enodes.back( );
    //
    // Skip if the node has already been processed before
    //

    if( egraph.valDupMap( enode ) != NULL )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;

    Enode * arg_list;
    for( arg_list = enode->getCdr( ); arg_list != egraph.enil; arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only children that haven't been processed yet,
      // but never children of a theory atom
      //
      if( !enode->isTAtom( ) && egraph.valDupMap( arg ) == NULL )
      {
        unprocessed_enodes.push_back( arg );
        unprocessed_children = true;
      }
    }
    //
    // SKip if unprocessed_children
    //
    if( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );
    Enode * result = NULL;

    if( enode->isTAtom( ) )
    {
      result = canonize_atom( enode );
    }
    else
    {
      assert( enode->isTerm( ) );
      assert( enode->isDTypeBool( ) );
      arg_list = enode->getCdr( );
      result = egraph.copyEnodeEtypeTermWithCache( enode );
    }

    assert( egraph.valDupMap( enode ) == NULL );
    egraph.storeDupMap( enode, result );
  }

  Enode * new_formula = egraph.valDupMap( formula );
  assert( new_formula );
  egraph.doneDupMap( );

  return new_formula;
}

//
// Performs reccursive canonization of an atom, results are returned in vars;
//
Enode * LACanonizer::canonize_atom( Enode * e )
{
  assert( e->isTAtom( ) );
  assert( e->isEq( ) || e->isLeq( ) || e->isLt( ) || e->isGeq( ) || e->isGt( ) );
  assert( e->getCdr( ) );
  assert( e->getCdr( )->getCdr( ) );
  assert( e->getCdr( )->getCdr( )->getCdr( )->isEnil( ) );

  //  cout << e << endl;


  // Canonise both parts of the constraint
  Enode * arg1 = e->get1st( );
  Enode * arg2 = e->get2nd( );
  map_int2Real vars, vars2;
  canonize_atom_rec( arg2, vars );
  canonize_atom_rec( arg1, vars2 );
  multiply_vars( vars2, -1 );
  merge_vars( vars, vars2 );

  Enode * la_members = const_cast<Enode *> ( egraph.enil );

  mpz_class gcd( 0 );
  mpz_class lcm( 0 );

  // Gets the free member of an equation
  Real value = 0;
  if( vars.find( -1 ) != vars.end( ) )
  {
    value = vars[-1];
    vars.erase( -1 );
  }

  // Constructs an equation from the resulting coefficients
  unsigned zero_counter( 0 );
  bool reversed = false;
  for( map_int2Real::iterator it = vars.begin( ); it != vars.end( ); it++ )
  {
    if( it->second != 0 )
    {
      Enode * coefficients;
      coefficients = egraph.cons( vars_hash[it->first] );
      if( ( la_members->isEnil( ) && it->second < 0 ) || reversed )
      {
        reversed = true; // stupid by repeating assignment twice is a mess
#if FAST_RATIONALS
        coefficients = egraph.cons(egraph.mkNum(-it->second), coefficients);
#else
        coefficients = egraph.cons( egraph.mkNum( -1 * it->second ), coefficients );
#endif
      }
      else
        coefficients = egraph.cons( egraph.mkNum( it->second ), coefficients );
      la_members = egraph.cons( egraph.mkTimes( coefficients ), la_members );
    }
    else
      zero_counter++;
  }
  if( !reversed )
    value *= -1;

  // Merge free member and equation into constraint
  Enode * result;

  // Check if there is any non-zero coefficient
  if( zero_counter == vars.size( ) )
  {
    if( ( e->isEq( ) && value == 0 ) || ( e->isLt( ) && value < 0 ) || ( e->isGt( ) && value > 0 ) || ( e->isLeq( ) && value <= 0 ) || ( e->isGeq( ) && value
        >= 0 ) )
      result = egraph.mkTrue( );
    else
      result = egraph.mkFalse( );

    return result;
  }
  else if( ( vars.size( ) - zero_counter ) == 1 )
    result = la_members;
  else
    result = egraph.cons( egraph.mkPlus( la_members ) );

  result = egraph.cons( egraph.mkNum( value ), result );

  if( e->isEq( ) )
  {
    Enode * or1 = egraph.mkLeq( result );
    Enode * or2 = egraph.mkGeq( result );
    result = egraph.cons( or1, egraph.cons( or2 ) );
    result = egraph.mkAnd( result );
  }
  else if( e->isLeq( ) && !reversed || e->isGeq( ) && reversed )
    result = egraph.mkLeq( result );
  else if( e->isGeq( ) && !reversed || e->isLeq( ) && reversed )
    result = egraph.mkGeq( result );
  else if( e->isLt( ) && !reversed || e->isGt( ) && reversed )
    result = egraph.mkLt( result );
  else if( e->isGt( ) && !reversed || e->isLt( ) && reversed )
    result = egraph.mkGt( result );
  else
    result = egraph.cons( e->getCar( ), result );

  //  cout << result << endl;
  return result;
}

//
// Performs reccursive canonization of an atom, results are returned in vars;
//
void LACanonizer::canonize_atom_rec( Enode * atom, map_int2Real & vars )
{
  // Check all accepting types of atoms
  if( atom->isPlus( ) )
  {
    // canonize all operands and merge the resulting vars maps
    for( Enode * arg_list = atom->getCdr( ); arg_list != egraph.enil; arg_list = arg_list->getCdr( ) )
    {
      map_int2Real tmp;
      canonize_atom_rec( arg_list->getCar( ), tmp );
      merge_vars( vars, tmp );
    }
  }
  else if( atom->isMinus( ) )
  {
    Enode * arg_list = atom->getCdr( );
    // canonize first operand
    if( arg_list != egraph.enil )
      canonize_atom_rec( arg_list->getCar( ), vars );
    else
      assert( 0 );

    // canonize all negated operands
    for( arg_list = arg_list->getCdr( ); arg_list != egraph.enil; arg_list = arg_list->getCdr( ) )
    {
      map_int2Real tmp;
      canonize_atom_rec( arg_list->getCar( ), tmp );
      multiply_vars( tmp, -1 );
      merge_vars( vars, tmp );
    }
  }
  else if( atom->isTimes( ) )
  {
    assert( atom->getCdr( ) );
    assert( atom->getCdr( )->getCdr( ) );
    assert( atom->getCdr( )->getCdr( )->getCdr( )->isEnil( ) );
    Enode * arg1 = atom->get1st( );
    Enode * arg2 = atom->get2nd( );

    // canonize both operands
    map_int2Real tmp1, tmp2;
    canonize_atom_rec( arg1, tmp1 );
    canonize_atom_rec( arg2, tmp2 );

    // if tmp1 is a constant
    if( tmp1.size( ) == 1 && tmp1.find( -1 ) != tmp1.end( ) )
    {
      multiply_vars( tmp2, tmp1[-1] );
      merge_vars( vars, tmp2 );
    }
    // if tmp2 is a constant
    else if( tmp2.size( ) == 1 && tmp2.find( -1 ) != tmp2.end( ) )
    {
      multiply_vars( tmp1, tmp2[-1] );
      merge_vars( vars, tmp1 );
    }
    // None of operands is constant -> non-linear
    else
    {
      cerr << "Atom is non-linear: " << atom << endl;
      assert( 0 );
    }
  }
  else if( atom->isConstant( ) )
  {
    // Read numerical value
    vars[-1] = atom->getCar( )->getValue( );
  }
  else if( atom->isUminus( ) )
  {
    // RB: Unary minus might contain a generic term as far as I know
#if 0
    // Read numerical value prefixed by unary minus
    vars[-1] = -1 * atom->get1st( )->getCar( )->getValue();
#else
    map_int2Real tmp;
    canonize_atom_rec( atom->get1st( ), tmp );
    multiply_vars( tmp, -1 );
    merge_vars( vars, tmp );
#endif
  }
  else if( atom->isVar( ) )
  {
    // Read the variable
    vars[atom->getId( )] = 1;
    vars_hash[atom->getId( )] = atom;
  }
  else if( atom->isDiv( ) )
  {
    // Read the (/ a b) clause
    assert( atom->get1st( )->isConstant( ) || atom->get1st( )->isUminus( ) );
    assert( atom->get2nd( )->isConstant( ) );
    if( atom->get1st( )->isUminus( ) )
#if FAST_RATIONALS
      vars[-1] = -atom->get1st( )->get1st()->getCar( )->getValue();
#else
      vars[-1] = -1 * atom->get1st( )->get1st( )->getCar( )->getValue( );
#endif
    else
      vars[-1] = atom->get1st( )->getCar( )->getValue( );

    vars[-1] /= atom->get2nd( )->getCar( )->getValue( );
  }
  else
  {
    cerr << "Atom has something strange: " << atom << endl;
    assert( 0 );
  }
}

//
// Multiply all values in map_int2Real by real
//
void LACanonizer::multiply_vars( map_int2Real & vars, const Real value )
{
  for( map_int2Real::iterator it = vars.begin( ); it != vars.end( ); it++ )
    it->second = it->second * value;
}

//
// Merge map_int2Real dst and src into dst
//
void LACanonizer::merge_vars( map_int2Real & dst, map_int2Real & src )
{
  // Values of intersecting entries are summarized
  for( map_int2Real::iterator it = src.begin( ); it != src.end( ); it++ )
    if( dst.find( it->first ) != dst.end( ) )
      dst[it->first] += it->second;
    else
      dst[it->first] = it->second;
}
