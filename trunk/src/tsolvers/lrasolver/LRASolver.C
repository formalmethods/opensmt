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

#include "LRASolver.h"
#include "LAVar.h"

#define USE_NUMBERS_POOL 1

//TODO: bad function, requires refactoring

//
// Reads the constraint into the solver
//
lbool LRASolver::inform( Enode * e )
{
  if( status != INIT )
  {
    error( "Inform was already closed", "" );
    return l_Undef;
  }
  assert( e->isAtom( ) );
  assert( e->isLeq( ) );

  Enode * arg1 = e->get1st( );
  Enode * arg2 = e->get2nd( );

  bool revert = false;

  if( !( arg1->isConstant( ) ) )
  {
    arg2 = e->get1st( );
    arg1 = e->get2nd( );
    revert = true;
  }

  assert( arg1->isConstant( ) );
  assert( arg2->isTimes( ) || arg2->isPlus( ) );

  // parse the var x of the contraint a < c*x
  if( arg2->isTimes( ) )
  {
    LAVar * x;

    Enode * coef = arg2->get1st( );
    Enode * var = arg2->get2nd( );

    Real v( 0 );
    // check the value from a
    if( arg1->isConstant( ) )
      v = arg1->getCar( )->getValue( );
    else
      cerr << "Unexpected Number: " << arg1 << endl;

    // divide a by the value from c
    Real c = coef->getCar( )->getValue( );

    if( coef->isConstant( ) )
      v /= c;
    else
      cerr << "Unexpected Coef: " << coef << endl;

    if( c < 0 )
      revert = !revert;

    // check if we need a new LAVar for a given var
    if( var->getId( ) >= ( int )enode_lavar.size( ) )
      enode_lavar.resize( var->getId( ) + 1, NULL );

    if( enode_lavar[var->getId( )] == NULL )
    {
      x = new LAVar( e, var, v, revert );
      slack_vars.push_back( x );
      enode_lavar[var->getId( )] = x;

      if( x->ID( ) >= columns.size( ) )
        columns.resize( x->ID( ) + 1, NULL );
      columns[x->ID( )] = x;

      if( e->getId( ) >= ( int )enode_lavar.size( ) )
        enode_lavar.resize( e->getId( ) + 1, NULL );
      enode_lavar[e->getId( )] = x;
    }
    else
    {
      x = enode_lavar[var->getId( )];
      x->setBounds( e, v, revert );

      if( e->getId( ) >= ( int )enode_lavar.size( ) )
        enode_lavar.resize( e->getId( ) + 1, NULL );
      enode_lavar[e->getId( )] = x;
    }
  }
  // parse the Plus enode of the contraint
  else if( arg2->isPlus( ) )
  {
    if( arg2->getId( ) >= ( int )enode_lavar.size( ) )
      enode_lavar.resize( arg2->getId( ) + 1, NULL );

    if( enode_lavar[arg2->getId( )] != NULL )
    {
      LAVar * x = enode_lavar[arg2->getId( )];
      x->setBounds( e, arg1 );

      if( e->getId( ) >= ( int )enode_lavar.size( ) )
        enode_lavar.resize( e->getId( ) + 1, NULL );
      enode_lavar[e->getId( )] = x;
    }
    else
    {
      // introduce the slack variable with bounds on it
      LAVar * s = new LAVar( e, arg1, arg2, true );
      slack_vars.push_back( s );

      assert( s->basicID( ) != -1 );

      if( s->ID( ) >= columns.size( ) )
        columns.resize( s->ID( ) + 1, NULL );
      columns[s->ID( )] = s;

      if( s->basicID( ) >= ( int )rows.size( ) )
        rows.resize( s->basicID( ) + 1, NULL );
      rows[s->basicID( )] = s;

      assert( numbers_pool.empty( ) );
      Real * p_r = new Real( -1 );

      //      s->polynomial[s->ID( )] = p_r;
      s->polynomial.assign( s->ID( ), p_r );

      if( e->getId( ) >= ( int )enode_lavar.size( ) )
        enode_lavar.resize( e->getId( ) + 1, NULL );
      enode_lavar[e->getId( )] = s;

      if( arg2->getId( ) >= ( int )enode_lavar.size( ) )
        enode_lavar.resize( arg2->getId( ) + 1, NULL );
      enode_lavar[arg2->getId( )] = s;

      Enode * list = arg2->getCdr( );

      //reads the argument of +
      while( !list->isEnil( ) )
      {
        Enode * p = list->getCar( );
        // If p is a monome ai*xi
        if( p->isTimes( ) )
        {
          arg1 = p->get1st( );
          arg2 = p->get2nd( );

          assert( arg1->isVar( ) || arg2->isVar( ) );

          // We store variable in var, coefficient in num
          Enode * var = arg1->isVar( ) ? arg1 : arg2;
          Enode * num = arg1->isVar( ) ? arg2 : arg1;

          // check if we need a new LAVar for a given var
          LAVar * x = NULL;

          if( var->getId( ) >= ( int )enode_lavar.size( ) )
            enode_lavar.resize( var->getId( ) + 1, NULL );

          if( enode_lavar[var->getId( )] != NULL )
          {
            x = enode_lavar[var->getId( )];
          }
          else
          {
            x = new LAVar( var );
            slack_vars.push_back( x );
            enode_lavar[var->getId( )] = x;

            if( x->ID( ) >= columns.size( ) )
              columns.resize( x->ID( ) + 1, NULL );
            columns[x->ID( )] = x;
          }

          assert( x );
          assert( s->basicID( ) != -1 );
          assert( numbers_pool.empty( ) );
          Real * p_r = new Real( num->getCar( )->getValue( ) );
          //          s->polynomial[x->ID( )] = p_r;
          s->polynomial.assign( x->ID( ), p_r );

          x->bindRow( s->basicID( ), p_r );
        }
        list = list->getCdr( );
      }
    }
  }
  else
  {
    cerr << "Unexpected atom: " << e << endl;
  }
#if VERBOSE
  cout << "Informed of constraint " << e << endl;
#endif
  return l_Undef;
}

//
// Performs the main Check procedure to see if the current constraints and Tableau are satisfiable
//
bool LRASolver::check( bool complete )
{
  // check if we stop reading constraints
  if( status == INIT )
    initSolver( );

  LAVar * x;

  // keep doing pivotAndUpdate until the SAT/UNSAT status is confirmed
  while( 1 )
  {

    // clear the explanations vector
    explanation.clear( );

    // look for the basic x with the smallest index which doesn't feat the bounds

    VectorLAVar::const_iterator it = rows.begin( );
    for( ; it != rows.end( ); ++it )
      if( ( *it )->isModelOutOfBounds( ) )
      {
        x = *it;
        break;
      }

    // If not found - SAT (or the current status of the solver)
    if( it == rows.end( ) )
    {
      refineBounds( );
      LAVar::saveModelGlobal( );
      return setStatus( SAT );
    }

    Real * a;
    LAVar * y;

    // Model doesn't feet the lower bound
    if( x->M( ) < x->L( ) )
    {
      // look for nonbasic terms to fix the unbounding
      LARow::const_iterator it = x->polynomial.begin( );
      for( ; it != x->polynomial.end( ); it++ )
      {
        if( it->first == -1 )
          continue;

        y = columns[it->first];
        if( x == y )
          continue;
        a = it->second;
        if( ( ( *a ) > 0 && y->M( ) < y->U( ) ) || ( ( *a ) < 0 && y->M( ) > y->L( ) ) )
          break; // stop on the very first that feats
      }

      // if it was not found - UNSAT
      if( it == x->polynomial.end( ) )
      {
        getConflictingBounds( x, explanation );
        //TODO: Keep the track of updated models and restore only them
        for( unsigned i = 0; i < columns.size( ); i++ )
          if( !columns[i]->skip )
            columns[i]->restoreModel( );
        return setStatus( UNSAT );
      }
      // if it was found - pivot old Basic x with non-basic y and do the model updates
      else
        pivotAndUpdate( x, y, x->L( ) );
    }
    else if( x->M( ) > x->U( ) )
    {
      // look for nonbasic terms to fix the unbounding
      LARow::const_iterator it = x->polynomial.begin( );
      for( ; it != x->polynomial.end( ); it++ )
      {
        if( it->first == -1 )
          continue;

        y = columns[it->first];
        if( x == y )
          continue;
        a = it->second;
        if( ( ( *a ) < 0 && y->M( ) < y->U( ) ) || ( ( *a ) > 0 && y->M( ) > y->L( ) ) )
          break; // stop on the very first that feats
      }

      // if it was not found - UNSAT
      if( it == x->polynomial.end( ) )
      {
        // add the x to explanations
        getConflictingBounds( x, explanation );
        for( unsigned i = 0; i < columns.size( ); i++ )
          if( !columns[i]->skip )
            columns[i]->restoreModel( );
        return setStatus( UNSAT );
      }
      // if it was found - pivot old Basic x with non-basic y and do the model updates
      else
        pivotAndUpdate( x, y, x->U( ) );
    }
    else
      cerr << "Error in bounds comparison" << endl;
  }
}

//
// Push the constraint into the solver and increase the level
//
bool LRASolver::assertLit( Enode * e, bool reason )
{
  // check if we stop reading constraints
  if( status == INIT )
    initSolver( );

  assert( e->hasPolarity( ) );

  bool is_reason = false;

  if( e->isDeduced( ) && e->getDeduced( ) == e->getPolarity( ) && e->getDedIndex( ) == id )
    return getStatus( );
  else if( e->isDeduced( ) && e->getDedIndex( ) == id )
    is_reason = true;

  // Look for the constraint to push
  LAVar* it = enode_lavar[e->getId( )];

  if( it != NULL )
  {
    lbool resultU = l_True;
    lbool resultL = l_True;

    assert( !it->isUnbounded( ) );
    unsigned it_I = it->getIteratorByEnode( e, e->getPolarity( ) == l_False );
    LAVar::StructBound &itBound = it->allBounds[it_I];

    // Check is simple SAT can be given
    if( ( itBound.boundType && it_I >= it->uBound ) || ( !itBound.boundType && it_I <= it->lBound ) )
      return getStatus( );

    // Check if simple UNSAT can be given
    if( ( !itBound.boundType && ( it_I > it->uBound ) ) || ( itBound.boundType && ( it_I < it->lBound ) ) )
    {
      explanation.clear( );
      if( itBound.boundType )
        explanation.push_back( it->allBounds[it->lBound].e );
      else
        explanation.push_back( it->allBounds[it->uBound].e );

      explanation.push_back( e );
      return setStatus( UNSAT );
    }

    // Prepare the history entry
    LAVarHistory &hist = pushed_constraints.back( );
    hist.e = e;
    hist.v = it;

    // Update the Tableau data if needed
    if( itBound.boundType )
    {
      hist.bound = it->uBound;
      hist.boundType = true;
      it->uBound = it_I;

      if( it->isNonbasic( ) && *( itBound.delta ) < it->M( ) )
      {
        update( it, *( itBound.delta ) );
      }
    }
    else
    {
      hist.bound = it->lBound;
      hist.boundType = false;
      it->lBound = it_I;

      if( it->isNonbasic( ) && *( itBound.delta ) > it->M( ) )
      {
        update( it, *( itBound.delta ) );
      }
    }

    bool res = check(true);

    // Check simple deductions;
    if( res && config.lraconfig.theory_propagation == 1 && !is_reason )
    {
      //TODO: Clear out the situation with deductions from the other solvers
      //      assert( !e->isDeduced( ) );
      it->getSimpleDeductions( deductions, id, itBound.boundType );
    }
//    assert(status == SAT);
//    LAVar::saveModelGlobal( );
//    return getStatus( );
    return res;
  }
  // Constraint to push was not find in local storage. Most likely it was not read properly before
  else
  {
    error( "Unexpected push! ", "" );
    return setStatus( ERROR );
  }
}

//
// Push the solver one level down
//
void LRASolver::pushBacktrackPoint( )
{
  // Create and push new history step
  LAVarHistory hist;
  hist.e = NULL;
  pushed_constraints.push_back( hist );
}

//
// Pop the solver one level up
//
void LRASolver::popBacktrackPoint( )
{
  // undo with history
  LAVarHistory &hist = pushed_constraints.back( );

  if( hist.e != NULL )
  {
    if( hist.boundType )
      hist.v->uBound = hist.bound;
    else
      hist.v->lBound = hist.bound;
  }

  pushed_constraints.pop_back( );
  setStatus( SAT );

}

//
// Look for unbounded terms and applies Gaussian elimination to them. Delete the column if succeeded
//
void LRASolver::doGaussianElimination( )
{
  unsigned m;

  for( unsigned i = 0; i < columns.size( ); i++ )
    if( !columns[i]->skip && columns[i]->isNonbasic( ) && columns[i]->isUnbounded( ) && columns[i]->bindedRows.size( ) > 0 )
    {
      LAVar * x = columns[i];

      LARow::const_iterator it = x->bindedRows.begin( );
      for( ; it != x->bindedRows.end( ); it++ )
        if( it->first != -1 )
          break;

      assert( it != x->bindedRows.end( ) );

      int basisRow = it->first;
      LAVar * basis = rows[basisRow];

      Real a = Real( *( it->second ) );
      Real ratio = 0;

      it++;

//      assert( it != x->bindedRows.end( ) );

      for( ; it != x->bindedRows.end( ); it++ )
      {
        if( it->first == -1 )
          continue;

        ratio = Real( ( *( it->second ) ) / a );
        for( LARow::const_iterator it2 = basis->polynomial.begin( ); it2 != basis->polynomial.end( ); it2++ )
        {
          if( it2->first == -1 )
            continue;

          LARow::iterator a_it = rows[it->first]->polynomial.find( it2->first );
          if( a_it == rows[it->first]->polynomial.end( ) )
          {
            Real * c = NULL;
            if( !numbers_pool.empty( ) )
            {
              c = numbers_pool.back( );
              numbers_pool.pop_back( );
              *c = -ratio * ( *( basis->polynomial.find( it2->first )->second ) );
            }
            else
            {
              c = new Real( -ratio * ( *( basis->polynomial.find( it2->first )->second ) ) );
            }
            //            rows[it->first]->polynomial[it2->first] = c;
            rows[it->first]->polynomial.assign( it2->first, c );
            columns[it2->first]->bindRow( it->first, c );
          }
          else
          {
            *( a_it->second ) -= ( *( basis->polynomial.find( it2->first )->second ) ) * ratio;
            if( *( a_it->second ) == 0 )
            {
              assert( a_it->second );
              // Do not remove the number, but store it in the
              // pool of available numbers, to be used later
              // instead of creating a new one
              numbers_pool.push_back( a_it->second );
              rows[it->first]->polynomial.erase( it2->first );
              if( it2->first != x->ID( ) )
                columns[it2->first]->unbindRow( it->first );
            }
          }
        }
      }

      // Clear removed row
      for( LARow::iterator it2 = basis->polynomial.begin( ); it2 != basis->polynomial.end( ); it2++ )
      {
        if( it2->first == -1 )
          continue;

        if( it2->first != basis->ID( ) )
        {
          columns[it2->first]->unbindRow( basisRow );
        }
        assert( it2->second );
        // Do not remove the number, but store it in the
        // pool of available numbers, to be used later
        // instead of creating a new one
        numbers_pool.push_back( it2->second );
        it2->first = -1;
      }
//TODO:: check this!
//      assert( !basis->bindedRows.empty( ) );
//      basis->bindedRows.clear();
      // Replace basisRow slot with the last row in rows vector
      m = rows.size( ) - 1;
      if( m > basisRow )
      {
        for( LARow::iterator it2 = rows[m]->polynomial.begin( ); it2 != rows[m]->polynomial.end( ); it2++ )
        {
          if( it2->first == -1 )
            continue;

          if( it2->first != rows[m]->ID( ) )
          {
            //            columns[it2->first]->unbindRow( m );
            //            LARow::const_iterator it3 = columns[it2->first]->bindedRows.find( m );
            //            if( it3 != columns[it2->first]->bindedRows.end( ) )
            columns[it2->first]->bindRow( basisRow, it2->second );
            columns[it2->first]->unbindRow( m );
          }
        }

        rows[basisRow] = rows[m];
        rows[m]->setBasic( basisRow );
      }
      basis->setNonbasic( );
      rows.pop_back( );

      //TODO: delete empty columns as well
      // Clear removed column
      x->bindedRows.clear( );
      x->skip = true;
    }
}

//
// updates the model values according to asserted bound
//
void LRASolver::update( LAVar * x, const Delta & v )
{
  // update model value for all basic terms
  Delta v_minusM = v - x->M( );
  for( LARow::const_iterator it = x->bindedRows.begin( ); it != x->bindedRows.end( ); it++ )
  {
    if( it->first == -1 )
      continue;

    rows[ it->first ]->incM( *( it->second ) * v_minusM );

    if( rows[it->first]->polynomial.size( ) <= config.lraconfig.poly_deduct_size )
      touched_rows.insert( rows[it->first] );

    //TODO: make a separate config value for suggestions
    //TODO: sort the order of suggestion requesting based on metric (Model increase, out-of-bound distance etc)
    //    if( config.lraconfig.theory_propagation == 3 )
    //    {
    //      if( suggestions.empty( ) )
    //        rows[it->first]->getSuggestions( suggestions, id );
    //    }
  }
  x->setM( v );
}

//
// pivots x and y in solver and does all updates
//
void LRASolver::pivotAndUpdate( LAVar * x, LAVar * y, const Delta & v )
{
  assert( x->isBasic( ) );
  assert( y->isNonbasic( ) );

  // get Tetta (zero if Aij is zero)
  const Real & a = *( x->polynomial.find( y->ID( ) )->second );
  assert( a != 0 );
  Delta tetha = ( v - x->M( ) ) / a;

  // update models of x and y
  x->setM( v );
  y->incM( tetha );

  // update model of Basic variables
  for( LARow::const_iterator it = y->bindedRows.begin( ); it != y->bindedRows.end( ); it++ )
  {
    if( it->first == -1 )
      continue;

    if( rows[it->first] != x )
    {
      //      cout << rows[it->first]->polynomial.find( y->ID( ) )->second << " - " << it->second << (rows[it->first]->polynomial.find( y->ID( ) )->second != it->second?" -> diff":"") << endl;
      rows[it->first]->incM( *( it->second ) * tetha );
      if( rows[it->first]->polynomial.size( ) <= config.lraconfig.poly_deduct_size )
        touched_rows.insert( rows[it->first] );
    }
  }
  // pivoting x and y

  //const Real & inverse = -1 / a;

#if FAST_RATIONALS
  const Real & inverse = -Real_inverse( a );
#else
  const Real & inverse = -1 / a;
#endif

  // first change the attribute values for x  polynomial
  for( LARow::iterator it = x->polynomial.begin( ); it != x->polynomial.end( ); it++ )
    if( it->first == -1 )
      continue;
    else
      *( it->second ) *= inverse;

  // value of a_y should become -1
  assert( !( *( x->polynomial.find( y->ID( ) )->second ) != -1 ) );

  // now change the attribute values for all rows where y was presented
  for( LARow::const_iterator it = y->bindedRows.begin( ); it != y->bindedRows.end( ); it++ )
  {
    if( it->first == -1 )
      continue;

    // check that the modified row is not x (it was changed already)
    if( rows[it->first] != x )
    {
      assert( *( it->second ) != 0 );

      // copy a to the new Real variable (use memory pool)
      Real * p_a = NULL;
      if( !numbers_pool.empty( ) )
      {
        p_a = numbers_pool.back( );
        numbers_pool.pop_back( );
        *p_a = *( it->second );
      }
      else
      {
        p_a = new Real( *( it->second ) );
      }

      const Real& a = *p_a;

      // P_i = P_i + a_y * P_x (iterate over all elements of P_x)
      for( LARow::const_iterator it2 = x->polynomial.begin( ); it2 != x->polynomial.end( ); it2++ )
      {
        if( it2->first == -1 )
          continue;

        const Real &b = *( it2->second );

        LARow::iterator a_it = rows[it->first]->polynomial.find( it2->first );

        // insert new element to P_i
        if( a_it == rows[it->first]->polynomial.end( ) )
        {
          // handle reals via memory pool
          Real * p_c = NULL;
          if( !numbers_pool.empty( ) )
          {
            p_c = numbers_pool.back( );
            numbers_pool.pop_back( );
            *p_c = a * b;
          }
          else
          {
            p_c = new Real( a * b );
          }

          //          rows[it->first]->polynomial[it2->first] = p_c;
          rows[it->first]->polynomial.assign( it2->first, p_c );
          columns[it2->first]->bindRow( it->first, p_c );
        }
        // or add to existing
        else
        {
          *( a_it->second ) += b * a;
          if( *( a_it->second ) == 0 )
          {
            // delete element from P_i if it become 0
            assert( a_it->second );

            // Do not remove the number, but store it in the
            // pool of available numbers, to be used later
            // instead of creating a new one
            numbers_pool.push_back( a_it->second );

            if( a_it->first != y->ID( ) )
              columns[a_it->first]->bindedRows.erase( it->first );
            rows[it->first]->polynomial.erase( a_it );
          }
        }
      }
      numbers_pool.push_back( p_a );

      assert( ( rows[it->first]->polynomial.find( y->ID( ) ) == rows[it->first]->polynomial.end( ) ) );

      // mark the affected row (for deductions)
      if( rows[it->first]->polynomial.size( ) <= config.lraconfig.poly_deduct_size )
        touched_rows.insert( rows[it->first] );
    }
  }

  // swap x and y (basicID, polynomial, bindings)
  y->setBasic( x->basicID( ) );
  x->setNonbasic( );
  rows[y->basicID( )] = y;

  y->polynomial.swap( x->polynomial );
  y->polynomial.is_there.swap( x->polynomial.is_there );
  //TODO: can I move size exchange into swap?
  y->polynomial.setsize( x->polynomial.size( ) );
  x->polynomial.setsize( 0 );

  x->bindRow( y->basicID( ), y->polynomial.find( x->ID( ) )->second );
  y->bindedRows.clear( );

  if( y->polynomial.size( ) <= config.lraconfig.poly_deduct_size )
    touched_rows.insert( y );
  touched_rows.erase( x );

  assert( x->polynomial.size( ) == 0 );
  assert( y->polynomial.size( ) > 0 );
  assert( x->bindedRows.size( ) > 0 );
}

//
// Perform all the required initialization after inform is complete
//
void LRASolver::initSolver( )
{
  if( status == INIT )
  {
    doGaussianElimination( );

    // sort the bounds inserted during inform stage
    for( unsigned it = 0; it < columns.size( ); it++ )
      if( !( columns[it]->skip ) )
        columns[it]->sortBounds( );

    status = SAT;
  }
  else
    error( "Solver can not be initialized in the state different from INIT", "" );
}

//
// Returns boolean value correspondent to the current solver status
//
inline bool LRASolver::getStatus( )
{
  switch( status )
  {
  case SAT:
  {
    return true;
    break;
  }
  case UNSAT:
  {
    return false;
    break;
  }
  case INIT:
  case ERROR:
  default:
    error( "Status is undef!", "" );
    return false;
  }
}

//
// Sets the new solver status and returns the correspondent lbool value
//
inline bool LRASolver::setStatus( LRAsolverStatus s )
{
  status = s;
  return getStatus( );
}

//
// Returns the bounds conflicting with the actual model.
//
void LRASolver::getConflictingBounds( LAVar * x, vector<Enode *> & dst )
{
  Real * a;
  LAVar * y;
  if( x->M( ) < x->L( ) )
  {
    // add all bounds for polynomial elements, which limit lower bound
    LARow::const_iterator it = x->polynomial.begin( );
    for( ; it != x->polynomial.end( ); it++ )
    {
      //skip empty cells in the polynomial vector
      if( it->first == -1 )
        continue;

      a = it->second;
      y = columns[it->first];
      assert( a );
      assert( ( *a ) != 0 );
      if( x == y )
      {
        dst.push_back( y->allBounds[y->lBound].e );
      }
      else if( ( *a ) < 0 )
      {
        assert( !y->L( ).isInf( ) );
        dst.push_back( y->allBounds[y->lBound].e );
      }
      else
      {
        assert( !y->U( ).isInf( ) );
        dst.push_back( y->allBounds[y->uBound].e );
      }
    }
  }
  if( x->M( ) > x->U( ) )
  {
    // add all bounds for polynomial elements, which limit upper bound
    LARow::const_iterator it = x->polynomial.begin( );
    for( ; it != x->polynomial.end( ); it++ )
    {
      if( it->first == -1 )
        continue;

      a = it->second;
      y = columns[it->first];
      assert( a );
      assert( ( *a ) != 0 );
      if( x == y )
      {
        dst.push_back( y->allBounds[y->uBound].e );
      }
      else if( ( *a ) > 0 )
      {
        assert( !y->L( ).isInf( ) );
        dst.push_back( y->allBounds[y->lBound].e );
      }
      else
      {
        assert( !y->U( ).isInf( ) );
        dst.push_back( y->allBounds[y->uBound].e );
      }
    }
  }
  assert( dst.size( ) == x->polynomial.size( ) );
}

//
// Compute the current bounds for each row and tries to deduce something useful
//
void LRASolver::refineBounds( )
{

  // Check if polynomial deduction is enabled
  if( config.lraconfig.poly_deduct_size == 0 )
    return;

  // iterate over all rows affected in the current check
  for( set<LAVar *>::const_iterator t_it = touched_rows.begin( ); t_it != touched_rows.end( ); t_it++ )
  {
    assert( ( *t_it )->isBasic( ) );
    LAVar * row = *t_it;

    bool UpInf = false; // become true when polynomial is infinite on the upper bound
    bool LoInf = false; // become true when polynomial is infinite on the lower bound
    bool UpExists = false; // flag is used to track if Up was initialized
    bool LoExists = false; // flag is used to track if Lo was initialized
    Delta Up( Delta::ZERO ); // upper bound value
    Delta Lo( Delta::ZERO ); // lower bound value
    int UpInfID = -1; // used to track the ID of the only element with infinite upper bound
    int LoInfID = -1; // used to track the ID of the only element with infinite lower bound

    // summarize all bounds for the polynomial
    for( LARow::const_iterator it = row->polynomial.begin( ); it != row->polynomial.end( ); it++ )
    {
      if( it->first == -1 )
        continue;

      Real & a = ( *( it->second ) );
      LAVar * col = columns[it->first];

      assert( a != 0 );
      bool a_lt_zero = a < 0;

      // check if the upper (lower) bound is infinite or can be added to the summarized value of the upper bound
      if( !UpInf && ( ( col->U( ).isPlusInf( ) && !a_lt_zero ) || ( col->L( ).isMinusInf( ) && a_lt_zero ) ) )
      {
        if( UpInfID == -1 )
          UpInfID = col->ID( ); // one element can be unbounded
        else
          UpInf = true; // no upper bound exists
      }
      else if( !UpInf )
      {
        // add lower or upper bound (depending on the sign of a_i)
        if( UpExists )
          Up += a * ( a_lt_zero ? col->L( ) : col->U( ) );
        else
        {
          Up = a * ( a_lt_zero ? col->L( ) : col->U( ) );
          UpExists = true;
        }
      }

      // check if the lower (upper) bound is infinite or can be added to the summarized value of the lower bound
      if( !LoInf && ( ( col->U( ).isPlusInf( ) && a_lt_zero ) || ( col->L( ).isMinusInf( ) && !a_lt_zero ) ) )
      {
        if( LoInfID == -1 ) // one element can be unbounded
          LoInfID = col->ID( );
        else
          LoInf = true; // no lower bound exists
      }
      else if( !LoInf )
      {
        // add lower or upper bound (depending on the sign of a_i)
        if( LoExists )
          Lo += a * ( !a_lt_zero ? col->L( ) : col->U( ) );
        else
        {
          Lo = a * ( !a_lt_zero ? col->L( ) : col->U( ) );
          LoExists = true;
        }
      }

      // stop if both lower or upper bounds become infinite
      if( UpInf && LoInf )
        break;
    }

    // check if the computed values are logically correct
    //    if( UpExists && LoExists && !UpInf && !LoInf && UpInfID == LoInfID )
    //      assert( Up >= Lo );

    // deduct from upper bound (if exists)
    if( !UpInf && UpExists )
    {
      // first check if one element is currently unbounded
      if( UpInfID != -1 )
      {
        LAVar * col = columns[UpInfID];
        const Real & a = ( *( row->polynomial.find( UpInfID )->second ) );
        assert( a != 0 );
        const Delta & b = -1 * Up / a;
        bool a_lt_zero = a < 0;

        if( a_lt_zero && col->U( ) > b )
          col->getDeducedBounds( b, true, deductions, id );
        else if( !a_lt_zero && col->L( ) < b )
          col->getDeducedBounds( b, false, deductions, id );
      }
      // if all are bounded then try to deduce for all of them
      else
      {
        for( LARow::const_iterator it = row->polynomial.begin( ); it != row->polynomial.end( ); it++ )
        {
          if( it->first == -1 )
            continue;

          const Real & a = ( *( it->second ) );
          assert( a != 0 );
          LAVar * col = columns[it->first];
          bool a_lt_zero = a < 0;
          const Delta & b = ( a * ( a_lt_zero ? col->L( ) : col->U( ) ) - Up ) / a;

          if( a_lt_zero && col->U( ) >= b )
            col->getDeducedBounds( b, true, deductions, id );
          else if( !a_lt_zero && col->L( ) <= b )
            col->getDeducedBounds( b, false, deductions, id );
        }
      }
    }

    // deduct from lower bound (if exists)
    if( !LoInf && LoExists )
    {
      // first check if one element is currently unbounded
      if( LoInfID != -1 )
      {
        LAVar * col = columns[LoInfID];
        const Real & a = ( *( row->polynomial.find( LoInfID )->second ) );
        assert( a != 0 );
        const Delta & b = -1 * Lo / a;
        bool a_lt_zero = a < 0;

        if( !a_lt_zero && col->U( ) > b )
          col->getDeducedBounds( b, true, deductions, id );
        else if( a_lt_zero && col->L( ) < b )
          col->getDeducedBounds( b, false, deductions, id );
      }
      // if all are bounded then try to deduce for all of them
      else
      {
        for( LARow::const_iterator it = row->polynomial.begin( ); it != row->polynomial.end( ); it++ )
        {
          if( it->first == -1 )
            continue;

          const Real & a = ( *( it->second ) );
          assert( a != 0 );
          LAVar * col = columns[it->first];
          bool a_lt_zero = a < 0;
          const Delta & b = ( a * ( !a_lt_zero ? col->L( ) : col->U( ) ) - Lo ) / a;

          if( !a_lt_zero && col->U( ) >= b )
            col->getDeducedBounds( b, true, deductions, id );
          else if( a_lt_zero && col->L( ) <= b )
            col->getDeducedBounds( b, false, deductions, id );
        }
      }
    }
  }
  touched_rows.clear( );
}

//
// Prints the current state of the solver (terms, bounds, tableau)
//
void LRASolver::print( ostream & out )
{
  out << "Bounds:" << endl;

  // print the upper bounds
  for( VectorLAVar::iterator it = columns.begin( ); it != columns.end( ); ++it )
    out << ( *it )->U( ) << "\t";
  out << endl;

  // print the lower bounds
  for( VectorLAVar::iterator it = columns.begin( ); it != columns.end( ); ++it )
    out << ( *it )->L( ) << "\t";
  out << endl;

  // print current model values
  out << "Var:" << endl;
  for( VectorLAVar::iterator it = columns.begin( ); it != columns.end( ); ++it )
  {
    out << ( *it ) << "\t";
  }
  out << endl;

  // print current model values
  out << "Model:" << endl;
  for( VectorLAVar::iterator it = columns.begin( ); it != columns.end( ); ++it )
    out << ( *it )->M( ) << "\t";
  out << endl;

  // print the terms IDs
  out << "Tableau:" << endl;
  for( VectorLAVar::iterator it = columns.begin( ); it != columns.end( ); ++it )
    out << ( *it )->ID( ) << "\t";
  out << endl;

  // print the Basic/Nonbasic status of terms
  for( VectorLAVar::iterator it = columns.begin( ); it != columns.end( ); ++it )
    out << ( ( *it )->isBasic( ) ? "B" : "N" ) << "\t";
  out << endl;

  // print the tableau cells (zeros are skipped)
  // iterate over Tableau rows
  for( unsigned i = 0; i < rows.size( ); i++ )
  {
    for( VectorLAVar::iterator it2 = columns.begin( ); it2 != columns.end( ); ++it2 )
    {
      if( rows[i]->polynomial.find( ( *it2 )->ID( ) ) != rows[i]->polynomial.end( ) )
        out << *( rows[i]->polynomial.find( ( *it2 )->ID( ) )->second );
      out << "\t";
    }
    out << endl;
  }
}

//TODO: -belongsToT not yet implemented
//
// Checks if atom belongs to this theory
//
bool LRASolver::belongsToT( Enode * )
{
  return true;
}
