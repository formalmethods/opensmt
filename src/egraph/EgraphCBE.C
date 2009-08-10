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

#include "Egraph.h"

//=============================================================================
// Extraction and Concatenation Interpretation

bool Egraph::assertBv( Enode * x, Enode * y, Enode * r, bool negated )
{
  // They are equal already
  if( x->getRoot( ) == y->getRoot( ) )
  {
    if( negated )
      assertNEq( x, y, new Reason( r ) );
    else
      return true;
  }

  if ( negated )
  {
    neq_stored     .push_back( r );
    undo_stack_term.push_back( NULL );
    undo_stack_oper.push_back( CBENEQSTORE );
  }

  assert( x );
  assert( y );
  assert( r );
  assert( config.ufconfig.int_extract_concat );
  assert( !x->isConstant( ) || !y->isConstant( ) ); // Should be blocked in mkEq

  bool res = true;

  cbeDetectNewSlices( x, y );

  if ( !negated )
  {
    list< Enode * > xslices;
    list< Enode * > yslices;
    //
    // Fix point computation of cut sets
    //
    int hard_stop = 0;
    do
    {
      hard_stop ++;
      if ( hard_stop > x->getWidth( ) )
	error( "something went wrong during fix-point computation", "" );
      xslices.clear( );
      yslices.clear( );
      cbeCompute( x, y, xslices, yslices );
    }
    while ( xslices.size( ) != yslices.size( ) );

    assert( xslices.size( ) == yslices.size( ) );
    assert( xslices.size( ) >= 1 );

    if ( xslices.size( ) > 1 )
    {
      // 
      // Equate corresponding slicings
      //
      list< Enode * >::iterator xt = xslices.begin( );
      list< Enode * >::iterator yt = yslices.begin( );
      int lsb = 0, msb;
      for ( ; xt != xslices.end( ) && res ; xt ++, yt ++ )
      {
	assert( (*xt)->getWidth( ) == (*yt)->getWidth( ) );
	msb = (*xt)->getWidth( ) - 1 + lsb;
	res = assertEq( *xt, *yt, new Reason( r, msb, lsb ) );
	lsb = msb + 1;
      }
    }
  }
  //
  // Finally push the (dis)equality
  //
  if ( res &&  negated ) res = assertNEq( x, y, new Reason( r ) );
  if ( res && !negated ) res = assertEq ( x, y, new Reason( r ) );

  return res;
}

void Egraph::cbeCompute( Enode * p
                       , Enode * q
		       , list< Enode * > & pslices
		       , list< Enode * > & qslices )
{
  assert( p );
  assert( q );
  assert( config.ufconfig.int_extract_concat );
  assert( pslices.empty( ) );
  assert( qslices.empty( ) );
  assert( !p->isConstant( ) || !q->isConstant( ) );
  //
  // Retrieve cut points slice descriptions
  //
  OrderedSet( int ) slices_int;
  list< Enode * > args;

  if ( !p->isConstant( ) ) cbeRetrieve( p, p->getWidth( ) - 1, 0, true, slices_int, args );
  if ( !q->isConstant( ) ) cbeRetrieve( q, q->getWidth( ) - 1, 0, true, slices_int, args );
  if ( !p->isConstant( ) && !q->isConstant( ) )
  {
    int lsb = 0, msb;
    for ( OrderedSet( int )::iterator it = slices_int.begin( ) 
	; it != slices_int.end( ) 
	; it ++ )
    {
      msb = *it - 1;
      if ( msb >= p->getWidth( ) ) break; // Done
      cbeUpdate( p, lsb, msb );
      cbeUpdate( q, lsb, msb );
      lsb = msb + 1; 
    }
    //
    // Retrive slice descriptions
    // (automatically returned to caller)
    // 
    cbeRetrieve( p, p->getWidth( ) - 1, 0, false, slices_int, pslices );
    cbeRetrieve( q, q->getWidth( ) - 1, 0, false, slices_int, qslices );
  }
  else
  {
    Enode * constant = p->isConstant( ) ? p : q;
    list< Enode * > & const_slices = p->isConstant( ) ? pslices : qslices;
    Enode * term = p->isConstant( ) ? q : p;
    list< Enode * > & term_slices = p->isConstant( ) ? qslices : pslices;
    //
    // Retrieve constant slices
    //
    if ( slices_int.size( ) == 1 )
    {
      const_slices.push_back( constant );
    }
    else
    {
      int lsb = 0, msb;
      for ( OrderedSet( int )::iterator it = slices_int.begin( ) 
	  ; it != slices_int.end( ) 
	  ; it ++ )
      {
	msb = *it - 1;
	// cerr << "msb: " << msb << ", lsb: " << lsb << endl;
	if ( msb >= p->getWidth( ) ) break; // Done
	const_slices.push_back( cbeGetSlice( msb, lsb, constant ) );
	lsb = msb + 1; 
      }
    }
    //
    // Retrieve term slices
    //
    cbeRetrieve( term, p->getWidth( ) - 1, 0, false, slices_int, term_slices );
    assert( term_slices.size( ) == const_slices.size( ) );
  }

  assert( pslices.size( ) >= 1 );
  assert( qslices.size( ) >= 1 );
}

void Egraph::cbeUpdate( Enode * e, const int lsb, const int msb )
{
  assert( e );
  assert( !e->isConstant( ) );
  assert( config.ufconfig.int_extract_concat );
  assert( 0 <= lsb );
  assert( lsb <= msb );
  assert( msb < e->getWidth( ) );

  vector< Enode * > unproc_slice;
  vector< int > unproc_lsb, unproc_msb;

  if ( cbeIsInCb( e ) )
  {
    unproc_slice.push_back( e );
    unproc_lsb  .push_back( lsb );
    unproc_msb  .push_back( msb );
  }
  //
  // We are trying to update an external
  // slice. Update the root instead
  //
  else
  {
    assert( e->isExtract( ) );
    int arg_lsb, arg_msb;
    e->isExtract( &arg_msb, &arg_lsb );
    assert( arg_msb - arg_lsb >= msb - lsb );
    Enode * arg = e->get1st( );

    unproc_slice.push_back( arg );
    unproc_lsb  .push_back( lsb + arg_lsb );
    unproc_msb  .push_back( msb + arg_lsb );
  }

  while( !unproc_slice.empty( ) )
  {
    Enode * current_slice = unproc_slice.back( );
    const int current_lsb = unproc_lsb.back( );
    const int current_msb = unproc_msb.back( );

    assert( !current_slice->isCbe( ) );
    unproc_slice.pop_back( );
    unproc_lsb.pop_back( );
    unproc_msb.pop_back( );

    if ( current_lsb == 0 && current_msb == current_slice->getWidth( ) - 1 )
      continue;

    // 
    // If slice is not in cbe update the root
    //
    if ( !cbeIsInCb( current_slice ) )
    {
      assert( current_slice->isExtract( ) );
      int arg_lsb, arg_msb;
      current_slice->isExtract( &arg_msb, &arg_lsb );
      assert( arg_msb - arg_lsb >= current_msb - current_lsb );
      Enode * arg = current_slice->get1st( );
      unproc_slice.push_back( arg );
      unproc_lsb  .push_back( current_lsb + arg_lsb );
      unproc_msb  .push_back( current_msb + arg_lsb );
      continue;
    } 

    Enode * cbe = current_slice->getCb( );
    //
    // Go down in the cbe
    //
    if ( cbe->isCbe( ) )
    {
      int width_left = cbe->getWidth( );
      for ( Enode * list = cbe->getCdr( ) 
	  ; !list->isEnil( )
	  ; list = list->getCdr( ) )
      {
	Enode * arg = list->getCar( );
	const int arg_msb = width_left - 1;
	const int arg_lsb = width_left - arg->getWidth( );
	//
	// Stop if external slice on the right
	//
	if ( current_lsb > arg_msb ) 
	{
	  break;
	}

	//
	// Skip external slices on the left
	//
	if ( current_msb < arg_lsb ) 
	  ; // Do nothing
	//
	// Slice is already there
	//
	else if ( ( arg_lsb == current_lsb && current_msb == arg_msb )
	     || ( current_lsb < arg_lsb && arg_msb < current_msb ) )
	  ; // Do nothing
	//
	// Slice is completely contained in existing
	// slices. Continue selecting the slice
	//
	else if ( arg_lsb <= current_lsb && current_msb <= arg_msb )
	{
	  unproc_lsb.push_back( current_lsb - arg_lsb ); 
	  unproc_msb.push_back( current_msb - arg_lsb ); 
	  unproc_slice.push_back( arg );
	  // Useless to go any further
	  break;
	}
	//
	// Overlapping 1
	//
	else if ( current_lsb <= arg_msb 
	       && arg_msb <= current_msb 
	       && current_lsb > arg_lsb )
	{
	  unproc_lsb.push_back( current_lsb - arg_lsb ); 
	  unproc_msb.push_back( arg_msb - arg_lsb ); 
	  unproc_slice.push_back( arg );
	}
	//
	// Overlapping 2
	//
	else if ( current_lsb <= arg_lsb 
	       && arg_lsb <= current_msb 
	       && current_msb < arg_msb )
	{
	  unproc_lsb.push_back( 0 ); 
	  unproc_msb.push_back( current_msb - arg_lsb ); 
	  unproc_slice.push_back( arg );
	}
	width_left = width_left - arg->getWidth( );
      }
    }
    else if ( current_slice != current_slice->getRef( ) 
	   || current_slice->isConstant( ) )
    {
      assert( !current_slice->getRef( )->isConstant( ) );
      unproc_slice.push_back( current_slice->getRef( ) );
      unproc_lsb  .push_back( current_lsb );
      unproc_msb  .push_back( current_msb );
    }
    else
    {
      cbeUpdateSplit( current_slice, current_msb, current_lsb );
    }
  }
}

void Egraph::cbeUpdateSplit( Enode * slice, const int i, const int j )
{
  assert( cbeIsInCb( slice ) );
  assert( !slice->isConstant( ) );

  int msb = i;
  int lsb = j;
  int ub = slice->getWidth( );
  int lb = 0;

  list< Enode * > args;

  Enode * sel_slice = slice;

  int l, m;
  if ( slice->isExtract( &m, &l ) 
    && slice->get1st( )->isConstant( ) )
  {
    sel_slice = slice->get1st( )->getRef( );

    lsb = l + j;   // lsb is moved by offset l
    msb = l + i;   // msb is moved by offset l
    lb = l;        // Lower bound is set to l
    ub = m + 1;    // Upper bound is set to m + 1 

    assert( lb <= lsb );
    assert( msb <= ub );
  }

  const int w = slice->getWidth( );
  assert( ub - lb == w );

  if ( lsb == lb )
  {
    //
    // Create new slices
    //
    Enode * sl = mkExtract( msb   , lb     , sel_slice );
    Enode * sh = mkExtract( ub - 1, msb + 1, sel_slice );
    assert( !sl->isConstant( ) );
    assert( !sh->isConstant( ) );
    args.push_back( sl );
    args.push_back( sh );
    //
    // Transfer constant as well
    //
    if ( slice->getConstant( ) != NULL )
    {
      Enode * constant = slice->getConstant( );
      assert( slice != constant );
      bool res;
      res = assertEq( cbeGetSlice( i    , 0    , constant ), sl, new Reason( REASON_CONSTANT, slice ) ); assert( res );
      res = assertEq( cbeGetSlice( w - 1, i + 1, constant ), sh, new Reason( REASON_CONSTANT, slice ) ); assert( res );
    }
  }
  else if ( msb == ub - 1 )
  {
    //
    // Create new slices
    //
    Enode * sl = mkExtract( lsb - 1, lb , sel_slice );
    Enode * sh = mkExtract( msb    , lsb, sel_slice );
    assert( !sl->isConstant( ) );
    assert( !sh->isConstant( ) );
    args.push_back( sl );
    args.push_back( sh );
    //
    // Transfer constant as well
    //
    if ( slice->getConstant( ) != NULL )
    {
      Enode * constant = slice->getConstant( );
      assert( slice != constant );
      bool res;
      res = assertEq( cbeGetSlice( j - 1, 0, constant ), sl, new Reason( REASON_CONSTANT, slice ) ); assert( res );
      res = assertEq( cbeGetSlice( i    , j, constant ), sh, new Reason( REASON_CONSTANT, slice ) ); assert( res );
    }
  }
  else
  {
    //
    // Create new slices
    //
    Enode * sl = mkExtract( lsb - 1, lb     , sel_slice );
    Enode * sm = mkExtract( msb    , lsb    , sel_slice );
    Enode * sh = mkExtract( ub - 1 , msb + 1, sel_slice );
    assert( !sl->isConstant( ) );
    assert( !sm->isConstant( ) );
    assert( !sh->isConstant( ) );
    args.push_back( sl );
    args.push_back( sm );
    args.push_back( sh );
    //
    // Transfer constant as well
    //
    if ( slice->getConstant( ) != NULL )
    {
      Enode * constant = slice->getConstant( );
      assert( slice != constant );
      bool res;
      res = assertEq( cbeGetSlice( j - 1, 0    , constant ), sl, new Reason( REASON_CONSTANT, slice ) ); assert( res );
      res = assertEq( cbeGetSlice( i    , j    , constant ), sm, new Reason( REASON_CONSTANT, slice ) ); assert( res );
      res = assertEq( cbeGetSlice( w - 1, i + 1, constant ), sh, new Reason( REASON_CONSTANT, slice ) ); assert( res );
    }
  }

  Enode * cb_new = mkCbe( cons( args ) );

  for ( Enode * list = cb_new->getCdr( ) 
      ; !list->isEnil( )
      ; list = list->getCdr( ) )
  {
    Enode * arg = list->getCar( );
    cbeAddToCb( arg );
  }

  slice->setCb( cb_new );
  undo_stack_oper.push_back( SPLIT );
  undo_stack_term.push_back( slice );
}

void Egraph::cbeRetrieve( Enode * e
                        , const int msb
			, const int lsb
			, bool cut_set
			, OrderedSet( int ) & cuts
			, list< Enode * > & cbe_args )
{
  assert( e );
  assert( config.ufconfig.int_extract_concat );

  Enode * cb = e->getCb( );
  assert( cb->isCbe( ) || e == cb );

  list< Enode * > unproc_slice;
  list< int > unproc_lsb, unproc_msb, unproc_off, unproc_shift;

  assert( !e->isConstant( ) );
  if ( cbeIsInCb( e ) )
  {
    unproc_slice.push_back( e );
    unproc_lsb.push_back( lsb );
    unproc_msb.push_back( msb );
    unproc_off.push_back( 0 );
    unproc_shift.push_back( 0 );
  }
  else
  {
    assert( e->isExtract( ) );
    int arg_lsb, arg_msb;
    e->isExtract( &arg_msb, &arg_lsb );
    assert( arg_msb - arg_lsb >= msb - lsb );
    Enode * arg = e->get1st( );

    unproc_slice.push_back( arg );
    unproc_lsb.push_back( lsb + arg_lsb );
    unproc_msb.push_back( msb + arg_lsb );
    unproc_off.push_back( 0 );
    unproc_shift.push_back( arg_lsb );
  }

  while ( !unproc_slice.empty( ) )
  {
    Enode * current_slice = unproc_slice.back( );
    const int current_lsb = unproc_lsb.back( );
    const int current_msb = unproc_msb.back( );
    const int current_off = unproc_off.back( );
    const int current_shift = unproc_shift.back( );
    assert( current_lsb <= current_msb );
    assert( !current_slice->isCbe( ) );

    unproc_slice.pop_back( );
    unproc_lsb  .pop_back( );
    unproc_msb  .pop_back( );
    unproc_off  .pop_back( );
    unproc_shift.pop_back( );

    // 
    // If slice is not in cbe retrieve from the root
    //
    if ( !cbeIsInCb( current_slice ) )
    {
      assert( current_slice->isExtract( ) );
      int arg_lsb, arg_msb;
      current_slice->isExtract( &arg_msb, &arg_lsb );
      assert( arg_msb - arg_lsb >= current_msb - current_lsb );
      Enode * arg = current_slice->get1st( );
      unproc_slice.push_back( arg );
      unproc_lsb  .push_back( current_lsb + arg_lsb );
      unproc_msb  .push_back( current_msb + arg_lsb );
      unproc_off  .push_back( 0 );
      unproc_shift.push_back( arg_lsb );
      continue;
    } 

    Enode * cbe = current_slice->getCb( );
    //
    // Consider cb
    //
    if ( cbe->isCbe( ) )
    {
      int width_left = cbe->getWidth( );
      for ( Enode * list = cbe->getCdr( ) 
	  ; !list->isEnil( ) 
	  ; list = list->getCdr( ) )
      {
	Enode * arg = list->getCar( );
	const int arg_msb = width_left - 1;
	const int arg_lsb = width_left - arg->getWidth( );
	width_left -= arg->getWidth( );
	if ( current_lsb <= arg_lsb && arg_msb <= current_msb )
	{
	  unproc_slice.push_back( arg );
	  unproc_msb  .push_back( arg->getWidth( ) - 1 );
	  unproc_lsb  .push_back( 0 );
	  unproc_off  .push_back( current_off + width_left );
	  unproc_shift.push_back( current_shift );
	}
	else if ( arg_lsb <= current_lsb && current_msb <= arg_msb )
	{
	  unproc_slice.push_back( arg );
	  unproc_msb  .push_back( current_msb - arg_lsb );
	  unproc_lsb  .push_back( current_lsb - arg_lsb );
	  unproc_off  .push_back( current_off + width_left );
	  unproc_shift.push_back( current_shift );
	}
	else if ( arg_lsb <= current_msb && current_msb <= arg_msb )
	{
	  assert( current_lsb < arg_lsb );
	  unproc_slice.push_back( arg );
	  unproc_msb  .push_back( current_msb - arg_lsb );
	  unproc_lsb  .push_back( 0 );
	  unproc_off  .push_back( current_off + width_left );
	  unproc_shift.push_back( current_shift );
	}
	else if ( arg_lsb <= current_lsb && current_lsb <= arg_msb )
	{
	  assert( current_msb > arg_msb );
	  unproc_slice.push_back( arg );
	  unproc_msb  .push_back( arg_msb );
	  unproc_lsb  .push_back( current_lsb - arg_lsb );
	  unproc_off  .push_back( current_off + width_left );
	  unproc_shift.push_back( current_shift );
	}
	else
	  assert( current_lsb > arg_msb || current_msb < arg_lsb );
      }
    }
    //
    // Recursively call on subslices
    //
    else if ( ( current_slice != current_slice->getRef( ) 
	// Remove the following to obtain canonical form.
	// We don't want it when retrieving slices for cbeCompute
	// because passing to ref will prevent the reason n n->root
	// to show up inside the conflict. We move to the reference
	// only if it is not a leaf of the CBE
	   && current_slice->getRef( )->getCb( )->isCbe( ) )
	)
    {
      unproc_slice.push_back( current_slice->getRef( ) );
      unproc_lsb  .push_back( current_lsb );
      unproc_msb  .push_back( current_msb );
      unproc_off  .push_back( current_off );
      unproc_shift.push_back( current_shift );
    }
    else if ( current_slice->isConstant( ) )
    {
      unproc_slice.push_back( current_slice->getRef( ) );
      unproc_lsb  .push_back( current_lsb );
      unproc_msb  .push_back( current_msb );
      unproc_off  .push_back( current_off );
      unproc_shift.push_back( current_shift );
    }
    else
    {
      assert( cbeIsInCb( current_slice ) );
      assert( !current_slice->isConstant( ) );
      assert( current_slice->getWidth( ) == current_msb - current_lsb + 1 );
      if ( cut_set )
      {
	assert( current_slice->getWidth( ) + current_off - current_shift >= 0 );
	cuts.insert( current_slice->getWidth( ) + current_off - current_shift );
      }
      else
      {
	cbe_args.push_back( current_slice );
      }
    }
  }

  assert( !cut_set || cuts.size( ) >= 1 );
  assert(  cut_set || cbe_args.size( ) >= 1 );
}

Enode * Egraph::cbe( Enode * e )
{
  list< Enode * > args;
  OrderedSet( int ) cuts;
  cbeRetrieve( e, e->getWidth( ) - 1, 0, false, cuts, args );
  Enode * res = mkCbe( cons( args ) );
  assert( e->getWidth( ) == res->getWidth( ) );
  return res;
}

Enode * Egraph::cbeGetSlice( const int msb, const int lsb, Enode * cbe )
{
  assert( cbe );
  assert( cbe->isCbe( ) || cbe->isConstant( ) );

  Enode * res = NULL;

  if ( cbe->isConstant( ) )
  {
    const char * value = cbe->getCar( )->getName( );
    int width = cbe->getWidth( );
    char new_value[ msb - lsb + 2 ];
    const int bit_i = width - 1 - msb;
    const int bit_j = width - 1 - lsb;

    for ( int h = bit_i ; h <= bit_j ; h ++ )
      new_value[ h - bit_i ] = value[ h ];

    new_value[ msb - lsb + 1 ] = '\0';

    assert( (int)strlen( new_value ) == msb - lsb + 1 );
    
    res = mkBvnum( new_value );
  }
  else
  {
    // Arguments of cbe
    list< Enode * > args;
    int width_left = cbe->getWidth( );
    for ( Enode * list = cbe->getCdr( ) 
	; !list->isEnil( ) 
	; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      const int arg_msb = width_left - 1;
      // Stop if out of range
      if ( lsb >  arg_msb ) break;
      // Add if in range
      if ( msb >= arg_msb ) args.push_front( arg );
      // Decrease width left 
      width_left = width_left - arg->getWidth( );
    }

    assert( args.size( ) >= 1 );
    // Create cbe
    res = mkCbe( cons( args ) );
  }

  assert( res );
  assert( res->getWidth( ) == msb - lsb + 1 );
  return res;
}

void Egraph::cbeExplainCb( Enode * e )
{
  assert( e );
  assert( config.ufconfig.int_extract_concat );

  Enode * cb = e->getCb( );
  //
  // Recursively consider references
  //
  if ( cb->isCbe( ) )
  {
    for ( Enode * list = cb->getCdr( ) 
	; !list->isEnil( ) 
	; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      cbeExplainCb( arg );
    }
  }
  //
  // Else reached the bottom of cb(e)
  //
  else if ( e != e->getRef( ) )
  {
    exp_pending.push_back( e );
    exp_pending.push_back( e->getRef( ) );
    cbeExplainCb( e->getRef( ) );
  }
}

void Egraph::cbeExplainSlice( const int msb, const int lsb, Enode * e )
{
  Enode * cb = e->getCb( );
  assert( cb->isCbe( ) || e == cb );

  if ( cb->isCbe( ) )
  {
    int width_left = cb->getWidth( );
    for ( Enode * list = cb->getCdr( ) 
	; !list->isEnil( ) 
	; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      const int arg_msb = width_left - 1;
      const int arg_lsb = width_left - arg->getWidth( );
      // Choose recursion in the right branch
      if ( arg_lsb <= lsb && msb <= arg_msb )
      {
	cbeExplainSlice( msb - arg_lsb, lsb - arg_lsb, arg );
	break;
      }
      width_left = width_left - arg->getWidth( );
    }
  }
  else if ( e != e->getRef( ) && e->getWidth( ) >= msb - lsb + 1 /*e->getWidth( ) > msb - lsb + 1*/ )
  {
     exp_pending.push_back( e );
     exp_pending.push_back( e->getRef( ) );
     cbeExplainSlice( msb, lsb, e->getRef( ) );
  }
}

void Egraph::cbeExplainConstant( Enode * e )
{
  assert( e );
  assert( config.ufconfig.int_extract_concat );

  int lsb, msb;

  assert( e->getConstant( ) != NULL );
  if ( e->isExtract( &msb, &lsb ) )
  {
    Enode * e_arg = e->get1st( );
    cbeExplainConstantRec( msb, lsb, e_arg );
  }
  else
    cbeExplainConstantRec( e->getWidth( ) - 1, 0, e );
}

void Egraph::cbeExplainConstantRec( const int msb, const int lsb, Enode * e )
{
  Enode * cb = e->getCb( );
  assert( cb->isCbe( ) || e == cb );

  if ( e->getConstant( ) != NULL )
  {
    if ( !e->isConstant( ) )
    {
      exp_pending.push_back( e );
      exp_pending.push_back( e->getConstant( ) );
    }
  }
  // Recursion on the right slice
  else if ( cb->isCbe( ) )
  {
    int width_left = cb->getWidth( );
    for ( Enode * list = cb->getCdr( ) 
	; !list->isEnil( ) 
	; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      const int arg_msb = width_left - 1;
      const int arg_lsb = width_left - arg->getWidth( );
      // Choose recursion in the right branch
      if ( arg_lsb <= lsb && msb <= arg_msb )
	cbeExplainConstantRec( msb - arg_lsb, lsb - arg_lsb, arg );
      width_left = width_left - arg->getWidth( );
    }
  }
  else if ( e != e->getRef( ) )
  {
    exp_pending.push_back( e );
    exp_pending.push_back( e->getRef( ) );
    cbeExplainConstantRec( msb, lsb, e->getRef( ) );
  }
  // This must not happen !
  else
    assert( false );
}

void Egraph::cbeDetectNewSlices( Enode * p, Enode * q )
{
  //
  // Scan the atom, and retrieve all extractions
  //
  initDup1( );
  vector< Enode * > unprocessed_enodes;
  unprocessed_enodes.push_back( p );
  unprocessed_enodes.push_back( q );
  //
  // Visit the DAG of the formula from the leaves to the root
  //
  while( !unprocessed_enodes.empty( ) )
  {
    Enode * e = unprocessed_enodes.back( );
    //
    // Skip if the node has already been processed before
    //
    if ( isDup1( e ) )
    {
      unprocessed_enodes.pop_back( );
      continue;
    }

    bool unprocessed_children = false;
    Enode * arg_list;
    for ( arg_list = e->getCdr( ) ; 
	arg_list != enil ; 
	arg_list = arg_list->getCdr( ) )
    {
      Enode * arg = arg_list->getCar( );
      assert( arg->isTerm( ) );
      //
      // Push only if it is unprocessed 
      //
      if ( !isDup1( arg ) )
      {
	unprocessed_enodes.push_back( arg );
	unprocessed_children = true;
      }
    }
    //
    // SKip if unprocessed_children
    //
    if ( unprocessed_children )
      continue;

    unprocessed_enodes.pop_back( );                      

    int lsb, msb; 
    //
    // Add node to the list of terms to equate
    //
    if ( (int)terms_stored.size( ) <= e->getId( ) )
      terms_stored.resize( e->getId( ) + 1, false );

    if ( e->isConstant( ) )
    {
      cbeAddToCb( e );
    }
    // 
    // If term is not stored, store it
    //
    else if ( !terms_stored[ e->getId( ) ] )
    {
      terms_stored[ e->getId( ) ] = true;
      undo_stack_oper.push_back( CBETSTORE );
      undo_stack_term.push_back( e );
      if ( e->isExtract( &msb, &lsb ) )
      {
	Enode * arg = e->get1st( );
	assert( terms_stored[ arg->getId( ) ] );
	cbeUpdate( arg, lsb, msb );
      }
      else 
      {
	cbeAddToCb( e );
      }
    }

    assert( !isDup1( e ) );
    storeDup1( e );
  }

  doneDup1( );
}

void Egraph::cbeUndoStore( Enode * e )
{
  assert( (int)terms_stored.size( ) > e->getId( ) );
  assert( terms_stored[ e->getId( ) ] );
  terms_stored[ e->getId( ) ] = false;
}

void Egraph::cbeAddToCb( Enode * e )
{
  if ( (int)terms_in_cb.size( ) <= e->getId( ) )
    terms_in_cb.resize( e->getId( ) + 1, 0 );
  terms_in_cb[ e->getId( ) ] ++;
}

void Egraph::cbeRemoveFromCb( Enode * e )
{
  assert( terms_in_cb[ e->getId( ) ] );
  terms_in_cb[ e->getId( ) ] --;
}

bool Egraph::cbeIsInCb( Enode * e )
{
  if ( (int)terms_in_cb.size( ) <= e->getId( ) )
    return false;
  return terms_in_cb[ e->getId( ) ] != 0;
}

bool Egraph::cbeIsExternal( Enode * e, const int msb, const int lsb )
{
  assert( !e->isExtract( ) );
  Enode * cb = e->getCb( );
  if ( !cb->isCbe( ) )
    return false;
  for ( Enode * list = cb->getCdr( ) ; !list->isEnil( ) ; list = list->getCdr( ) )
  {
    int m, l;
    list->getCar( )->isExtract( &m, &l );
    if ( msb < l ) continue;
    // Intersection
    if ( lsb < l ) return true;
    if ( l <= lsb && msb <= m ) return false;
  }

  assert( false );
  return true;
}
//=============================================================================
// Debugging Routines

void Egraph::printCbeStructure( ostream & os, Enode * e, Set( enodeid_t ) & cache )
{
  if ( cache.find( e->getId( ) ) != cache.end( ) )
    return;

  cache.insert( e->getId( ) );

  os << e->getId( ) << " [label=\"" << e << "\"];" << endl;

  Enode * cb = e->getCb( );
  
  if ( cb->isCbe( ) )
  {
    int width_left = e->getWidth( );
    int msb, lsb;
    for ( Enode * list = cb->getCdr( ) 
	; !list->isEnil( ) 
	; list = list->getCdr( ) )
    {
      Enode * arg = list->getCar( );
      msb = width_left - 1;
      lsb = width_left - arg->getWidth( );
      os << e->getId( ) << " -> " << arg->getId( ) << " [label=\"" << msb << ":" << lsb << "\"];" << endl;
      printCbeStructure( os, arg, cache );
      width_left = width_left - arg->getWidth( );
    }
  }
  else if ( e != e->getRef( ) )
  {
    os << e->getId( ) << " -> " << e->getRef( )->getId( ) << " [label=ref];" << endl;
    printCbeStructure( os, e->getRef( ), cache );
  }

  cache.insert( e->getId( ) );
}
