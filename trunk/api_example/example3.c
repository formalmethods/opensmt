#include "opensmt_c.h"
#include <stdio.h>

int main( void )
{
  // Creating context
  printf( "Creating context\n" );
  opensmt_context ctx = opensmt_mk_context( qf_bv );

  // Setting verbosity
  opensmt_set_verbosity( ctx, 0 );

  // Creating boolean variables
  printf( "Creating some boolean variables\n" );
  opensmt_expr a = opensmt_mk_bool_var( ctx, "a" );
  opensmt_expr b = opensmt_mk_bool_var( ctx, "b" );
  opensmt_expr c = opensmt_mk_bool_var( ctx, "c" );
  // Creating integer variables
  printf( "Creating some bv variables\n" );
  opensmt_expr x = opensmt_mk_bv_var( ctx, "x", 32 );
  opensmt_expr y = opensmt_mk_bv_var( ctx, "y", 32 );
  opensmt_expr z = opensmt_mk_bv_var( ctx, "z", 32 );
  // Creating inequality
  printf( "Creating x - y <= 0\n" );

  printf( "  Creating x - y\n" );
  opensmt_expr minus = opensmt_mk_bv_sub( ctx, x, y );
  printf( "  Expression created: " );
  opensmt_print_expr( minus );
  printf( "\n" );

  printf( "  Creating 0\n" );
  opensmt_expr zero = opensmt_mk_bv_constant( ctx, 32, 0 );
  printf( "  Expression created: " );
  opensmt_print_expr( zero );
  printf( "\n" );

  printf( "  Creating x - y <= 0\n" );
  opensmt_expr leq = opensmt_mk_bv_le( ctx, minus, zero );
  printf( "  Expression created: " );
  opensmt_print_expr( leq );
  printf( "\n" );
  
  // Creating inequality 2
  printf( "Creating y - z <= 0\n" );
  opensmt_expr minus2 = opensmt_mk_bv_sub( ctx, y, z );
  opensmt_expr leq2 = opensmt_mk_bv_le( ctx, minus2, zero );
  printf( "  Expression created: " );
  opensmt_print_expr( leq2 );
  printf( "\n" );

  // Creating inequality 3
  printf( "Creating z - x <= 0\n" );
  opensmt_expr minus3 = opensmt_mk_bv_sub( ctx, z, x );
  opensmt_expr leq3 = opensmt_mk_bv_le( ctx, minus3, zero );
  printf( "  Expression created: " );
  opensmt_print_expr( leq3 );
  printf( "\n" );

  // Asserting first inequality
  printf( "Asserting ");
  opensmt_print_expr( leq );
  printf( "\n" );
  opensmt_assert( ctx, leq );
  opensmt_push( ctx );

  // Checking for consistency
  printf( "\nChecking for consistency: " );
  opensmt_result res = opensmt_check( ctx );
  printf( "%s\n\n", res == l_false ? "unsat" : "sat" );

  // Asserting second inequality
  printf( "Asserting ");
  opensmt_print_expr( leq2 );
  printf( "\n" );
  opensmt_assert( ctx, leq2 );
  opensmt_push( ctx );

  // Checking for consistency
  printf( "\nChecking for consistency: " );
  res = opensmt_check( ctx );
  printf( "%s\n\n", res == l_false ? "unsat" : "sat" );

  // Asserting third inequality
  printf( "Asserting ");
  opensmt_print_expr( leq3 );
  printf( "\n" );
  opensmt_assert( ctx, leq3 );
  opensmt_push( ctx );

  // Checking for consistency
  printf( "\nChecking for consistency: " );
  res = opensmt_check( ctx );
  printf( "%s\n\n", res == l_false ? "unsat" : "sat" );
  
  // Resetting context
  printf( "Resetting context\n" );
  opensmt_reset( ctx );
  // Deleting context
  printf( "Deleting context\n" );
  opensmt_del_context( ctx );

  return 0;
}
