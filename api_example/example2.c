/*********************************************************************
Author: Aliaksei Tsitovich <aliaksei.tsitovich@usi.ch>

Example of OpenSMT API usage - bounded model checker for a given
automata.

*********************************************************************/


#include "opensmt_c.h"
#include <stdio.h>

int main( void )
{
  // Creating context
  printf( "Construction of the symbolic engine that unroles the following program:\n" );
  printf( "-----------\n int i=0;\n while (i<10)\n\t i++; \n assert(i=11);\n-----------\n" );

  printf( "Creating OpenSMT context with LRA support \n" );
  opensmt_context ctx = opensmt_mk_context( qf_lra );

  // Creating integer variables
  printf( "Creating variables for the first instruction\n" );

  int i = 0;
  char var[32];
  sprintf(var, "i%d", i);
  opensmt_expr int_i = opensmt_mk_int_var( ctx, var );
  opensmt_expr zero = opensmt_mk_num_from_string( ctx, "0" );
  opensmt_expr eq = opensmt_mk_eq( ctx, int_i, zero );

  printf( "First instruction:\t" );
  opensmt_print_expr( eq );
  printf( "\n" );
  opensmt_assert( ctx, eq );
  opensmt_push( ctx );

  opensmt_expr one = opensmt_mk_num_from_string( ctx, "1" );

  opensmt_expr expr_list[2];
  expr_list[1] = one;
  while (i<10)
  {
    i++;
    printf( "Loop iteration %d:\t", i );
    expr_list[0] = int_i;
    opensmt_expr plus = opensmt_mk_plus( ctx, expr_list, 2 );
    sprintf(var, "i%d", i);
    opensmt_expr int_i_prime = opensmt_mk_int_var( ctx, var );
    eq = opensmt_mk_eq( ctx, int_i_prime, plus );
    opensmt_print_expr( eq );
    opensmt_assert( ctx, eq );
    opensmt_push( ctx );
    int_i = int_i_prime;
    printf( "\n" );
  }

  printf( "Assertion:\t\t" );

  opensmt_expr eleven = opensmt_mk_num_from_string( ctx, "11" );
  eq = opensmt_mk_not( ctx, opensmt_mk_eq( ctx, int_i, eleven ) );
  opensmt_print_expr( eq );
  printf( "\n" );

  opensmt_assert( ctx, opensmt_mk_not( ctx, eq ) );
  opensmt_push( ctx );

  // Checking for consistency
  printf( "Checking for consistency: " );
  opensmt_result res = opensmt_check( ctx );
  printf( "%s\n", res == l_false ? "unsat" : (res == l_true ? "sat" : "unknown" ) );

  // Resetting context
  printf( "Resetting context\n" );
  opensmt_reset( ctx );
  // Deleting context
  printf( "Deleting context\n" );
  opensmt_del_context( ctx );

  return 0;
}
