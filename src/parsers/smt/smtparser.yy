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

%{

#include "global.h"

#include "Egraph.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>

extern int smtlineno;
extern int smtlex( );
extern Egraph *    parser_egraph;
extern SMTConfig * parser_config;

vector< unsigned > * createTypeList  ( unsigned );
vector< unsigned > * createTypeList  ( unsigned, const char * );
vector< unsigned > * pushTypeList    ( vector< unsigned > *, unsigned );
vector< unsigned > * pushTypeList    ( vector< unsigned > *, unsigned, const char * );
void		     destroyTypeList ( vector< unsigned > * );

void smterror( char * s )
{
  printf( "%d: %s\n", smtlineno, s );
  exit( 1 );
}

/* Overallocation to prevent stack overflow */
#define YYMAXDEPTH 1024 * 1024

%}

%union
{
  char  *              str;
  Enode *              enode;
  vector< unsigned > * type_list;
}

%token TK_NUM TK_STR TK_BVNUM TK_BVNUM_NO_WIDTH TK_BIT0 TK_BIT1
%token TK_BOOL TK_REAL TK_INT TK_BITVEC TK_U
%token TK_PLUS TK_MINUS TK_TIMES TK_UMINUS TK_DIV
%token TK_NE TK_EQ TK_LEQ TK_GEQ TK_LT TK_GT
%token TK_AND TK_OR TK_NOT TK_IFF TK_XOR TK_ITE TK_IFTHENELSE TK_IMPLIES
%token TK_BENCHMARK TK_SOURCE TK_ARGUMENT TK_STATUS TK_NOTES
%token TK_EXTRASORTS TK_EXTRAPREDS TK_EXTRAFUNS TK_LOGIC TK_CATEGORY TK_DIFFICULTY
%token TK_ASSUMPTION TK_FORMULA TK_TRUE TK_FALSE
%token TK_FLET TK_FLET_STR TK_LET TK_LET_STR TK_DISTINCT
%token TK_BVSLT TK_BVSGT TK_BVSLE TK_BVSGE
%token TK_BVULT TK_BVUGT TK_BVULE TK_BVUGE
%token TK_EXTRACT TK_CONCAT TK_BVAND TK_BVOR TK_BVXOR TK_BVNOT TK_BVADD TK_BVNEG TK_BVMUL
%token TK_SIGN_EXTEND TK_ZERO_EXTEND TK_ROTATE_LEFT TK_ROTATE_RIGHT TK_BVLSHR TK_BVSHL TK_BVSREM TK_BVSDIV TK_BVSUB
%token TK_BVUDIV TK_BVUREM

%type <str> TK_STR TK_NUM TK_BVNUM TK_BVNUM_NO_WIDTH TK_ARGUMENT TK_BOOL TK_REAL TK_INT TK_FLET_STR TK_LET_STR
%type <enode> formula atom expression arithmetic_expression bitvec_expression
%type <enode> formula_list expression_list arithmetic_expression_list bitvec_expression_list
%type <type_list> type_list

%start top

%%

top: '(' all ')'
   ;

all: all header_declaration
   | all variables_declaration
   | all assumption_declaration
   | all formula_declaration
   | header_declaration
   | variables_declaration
   | assumption_declaration
   | formula_declaration
   ;

header_declaration: header_declaration benchmark_declaration
		  | header_declaration notes_declaration
		  | header_declaration source_declaration 
                  | header_declaration status_declaration
                  | header_declaration category_declaration
		  | header_declaration difficulty_declaration
		  | header_declaration logic_declaration
                  | benchmark_declaration
                  | notes_declaration
                  | source_declaration
                  | status_declaration
                  | category_declaration
                  | difficulty_declaration
                  | logic_declaration
		  ;

benchmark_declaration: TK_BENCHMARK TK_STR
		       { free( $2 ); }
		     ;

notes_declaration: TK_NOTES TK_ARGUMENT
		   { free( $2 ); }
		 ;

source_declaration: TK_SOURCE TK_ARGUMENT
		    { free( $2 ); }
		  ;

status_declaration: TK_STATUS TK_STR
		    { 
                      if ( strcmp( $2, "sat" ) == 0 )  parser_config->status = l_True;
                      if ( strcmp( $2, "unsat" ) == 0 ) parser_config->status = l_False;
		      free( $2 ); 
		    }
		  ;

category_declaration: TK_CATEGORY TK_ARGUMENT
		      { free( $2 ); }
		    ;

difficulty_declaration: TK_DIFFICULTY TK_ARGUMENT
		        { free( $2 ); }
		      ;

logic_declaration: TK_LOGIC TK_STR
		   { 
                          if ( strcmp( $2, "EMPTY" ) == 0 ) parser_config->logic = EMPTY;
                     else if ( strcmp( $2, "QF_UF" ) == 0 ) parser_config->logic = QF_UF;
                     else if ( strcmp( $2, "QF_BV" ) == 0 ) parser_config->logic = QF_BV;
                     else if ( strcmp( $2, "QF_RDL" ) == 0 ) parser_config->logic = QF_RDL;
                     else if ( strcmp( $2, "QF_IDL" ) == 0 ) parser_config->logic = QF_IDL;
                     else if ( strcmp( $2, "QF_LRA" ) == 0 ) parser_config->logic = QF_LRA;
                     else if ( strcmp( $2, "QF_LIA" ) == 0 ) parser_config->logic = QF_LIA;
                     else if ( strcmp( $2, "QF_UFRDL" ) == 0 ) parser_config->logic = QF_UFRDL;
                     else if ( strcmp( $2, "QF_UFIDL" ) == 0 ) parser_config->logic = QF_UFIDL;
                     else if ( strcmp( $2, "QF_UFLRA" ) == 0 ) parser_config->logic = QF_UFLRA;
                     else if ( strcmp( $2, "QF_UFLIA" ) == 0 ) parser_config->logic = QF_UFLIA;
                     else if ( strcmp( $2, "QF_UFBV" ) == 0 ) parser_config->logic = QF_UFBV;
		     free( $2 ); 
                   }
		 ;

variables_declaration: variables_declaration bool_variable_declaration
		     | variables_declaration real_variable_declaration
		     | variables_declaration int_variable_declaration
		     | variables_declaration bitvec_variable_declaration
		     | variables_declaration u_function_declaration
		     | variables_declaration u_predicate_declaration
                     | sort_declaration
		     | bool_variable_declaration
		     | real_variable_declaration
		     | int_variable_declaration
		     | bitvec_variable_declaration
		     | u_function_declaration
		     | u_predicate_declaration
                     ;

sort_declaration: TK_EXTRASORTS '(' TK_STR ')'
		  { parser_egraph->newSort( $3 ); free( $3 ); }

bool_variable_declaration: TK_EXTRAPREDS '(' bool_variable_list ')'
			 ;

bool_variable_list: bool_variable_list '(' TK_STR ')'
		    { 
		      vector< unsigned > tmp;
		      tmp.push_back( DTYPE_BOOL );
		      parser_egraph->newSymbol( $3, tmp ); free( $3 ); 
		    }
		  | '(' TK_STR ')'
		    { 
		      vector< unsigned > tmp;
		      tmp.push_back( DTYPE_BOOL );
		      parser_egraph->newSymbol( $2, tmp ); free( $2 ); 
		    }
                  ;
		     
real_variable_declaration: TK_EXTRAFUNS '(' real_variable_list ')'
			 ;

real_variable_list: real_variable_list '(' TK_STR TK_REAL ')'
		    { 
		      vector< unsigned > tmp;
		      tmp.push_back( DTYPE_REAL );
		      parser_egraph->newSymbol( $3, tmp ); free( $3 ); 
		    }
		  | '(' TK_STR TK_REAL ')'
		    { 
		      vector< unsigned > tmp;
		      tmp.push_back( DTYPE_REAL );
		      parser_egraph->newSymbol( $2, tmp ); free( $2 ); 
		    }
                  ;

int_variable_declaration: TK_EXTRAFUNS '(' int_variable_list ')'
			;

int_variable_list: int_variable_list '(' TK_STR TK_INT ')'
		   { 
		     vector< unsigned > tmp;
		     tmp.push_back( DTYPE_INT );
		     parser_egraph->newSymbol( $3, tmp ); free( $3 ); }
		  | '(' TK_STR TK_INT ')'
		   { 
		     vector< unsigned > tmp;
		     tmp.push_back( DTYPE_INT );
		     parser_egraph->newSymbol( $2, tmp ); free( $2 ); 
		   }
                  ;

bitvec_variable_declaration: TK_EXTRAFUNS '(' bitvec_variable_list ')'
			   ;

bitvec_variable_list: bitvec_variable_list '(' TK_STR TK_BITVEC '[' TK_NUM ']' ')'
		      { 
			if ( atoi( $6 ) > MAX_WIDTH ) error( "bitvector width too large, max is ", MAX_WIDTH );
			vector< unsigned > tmp;
			tmp.push_back( DTYPE_BITVEC | atoi( $6 ) );
			parser_egraph->newSymbol( $3, tmp ); free( $3 ); free( $6 ); 
		      }
		    | '(' TK_STR  TK_BITVEC '[' TK_NUM ']' ')'
		      { 
			if ( atoi( $5 ) > MAX_WIDTH ) error( "bitvector width too large, max is ", MAX_WIDTH );
			vector< unsigned > tmp;
			tmp.push_back( DTYPE_BITVEC | atoi( $5 ) );
			parser_egraph->newSymbol( $2, tmp ); free( $2 ); free( $5 ); 
		      }
                    ;

u_predicate_declaration: TK_EXTRAPREDS '(' u_predicate_list ')'
		      ;

u_predicate_list: u_predicate_list '(' TK_STR type_list ')'
	         { (*$4).push_back( DTYPE_BOOL ); parser_egraph->newSymbol( $3, (*$4) ); free( $3 ); destroyTypeList( $4 ); }
               | '(' TK_STR type_list ')'
                 { (*$3).push_back( DTYPE_BOOL ); parser_egraph->newSymbol( $2, (*$3) ); free( $2 ); destroyTypeList( $3 ); }
	       ;

u_function_declaration: TK_EXTRAFUNS '(' u_function_list ')'
		      ;

u_function_list: u_function_list '(' TK_STR type_list ')'
	         { parser_egraph->newSymbol( $3, (*$4) ); free( $3 ); destroyTypeList( $4 ); }
               | '(' TK_STR type_list ')'
                 { parser_egraph->newSymbol( $2, (*$3) ); free( $2 ); destroyTypeList( $3 ); }
	       ;

type_list: type_list TK_U
	  { $$ = pushTypeList( $1, DTYPE_U ); }
	| type_list TK_INT
	  { $$ = pushTypeList( $1, DTYPE_INT ); }
	| type_list TK_REAL
	  { $$ = pushTypeList( $1, DTYPE_REAL ); }
	| type_list TK_BITVEC '[' TK_NUM ']'
	  { $$ = pushTypeList( $1, DTYPE_BITVEC, $4 ); free( $4 ); }
	| type_list TK_STR
	  { $$ = pushTypeList( $1, parser_egraph->getSort( $2 ) ); free( $2 ); }
	| TK_U
          { $$ = createTypeList( DTYPE_U ); }
	| TK_INT
          { $$ = createTypeList( DTYPE_INT ); }
	| TK_REAL
          { $$ = createTypeList( DTYPE_REAL ); }
	| TK_BITVEC '[' TK_NUM ']'
          { $$ = createTypeList( DTYPE_BITVEC, $3 ); free( $3 ); }
	| TK_STR
          { $$ = createTypeList( parser_egraph->getSort( $1 ) ); free( $1 ); }
	;

assumption_declaration: assumption_declaration TK_ASSUMPTION formula
			{ parser_egraph->addAssumption( $3 ); }
		      | TK_ASSUMPTION formula
		        { parser_egraph->addAssumption( $2 ); }
		      ;

formula_declaration: TK_FORMULA formula
		     { parser_egraph->setTopEnode( $2 ); }
		   ;

formula: '(' TK_AND formula_list ')' 
	 { $$ = parser_egraph->mkAnd( $3 ); } 
       | '(' TK_OR formula_list ')'
	 { $$ = parser_egraph->mkOr( $3 ); } 
       | '(' TK_IMPLIES formula_list ')'
	 { $$ = parser_egraph->mkImplies( $3 ); } 
       | '(' TK_NOT formula_list ')'
	 { $$ = parser_egraph->mkNot( $3 ); }
       | '(' TK_IFF formula_list ')'
         { $$ = parser_egraph->mkIff( $3 ); }
       | '(' TK_XOR formula_list ')'
         { $$ = parser_egraph->mkXor( $3 ); }
       | '(' TK_IFTHENELSE formula formula formula ')'
         { $$ = parser_egraph->mkIfthenelse( $3, $4, $5 ); }
       | '(' TK_FLET flet_def formula ')'
         { $$ = $4; }
       | '(' TK_LET let_def formula ')'
	 { $$ = $4; }
       | TK_FLET_STR
         { $$ = parser_egraph->getDefine( $1 ); free( $1 ); }
       | atom
	 { $$ = $1; }
       ;

formula_list: formula formula_list
	      { $$ = parser_egraph->cons( $1, $2 ); }
	    | formula
	      { $$ = parser_egraph->cons( $1 ); }   
	    ;

let_def: '(' TK_LET_STR expression ')'
	 { parser_egraph->mkDefine( $2, $3 ); free( $2 ); } 

flet_def: '(' TK_FLET_STR formula ')'
	  { parser_egraph->mkDefine( $2, $3 ); free( $2 ); } 

atom: '(' TK_GEQ arithmetic_expression_list ')'
      { $$ = parser_egraph->mkGeq( $3 ); }
    | '(' TK_LEQ arithmetic_expression_list ')'
      { $$ = parser_egraph->mkLeq( $3 ); }
    | '(' TK_GT arithmetic_expression_list ')'
      { $$ = parser_egraph->mkGt( $3 ); }
    | '(' TK_LT arithmetic_expression_list ')'
      { $$ = parser_egraph->mkLt( $3 ); }
    | '(' TK_BVSLT bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvslt( $3 ); }
    | '(' TK_BVSGT bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvsgt( $3 ); }
    | '(' TK_BVSLE bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvsle( $3 ); }
    | '(' TK_BVSGE bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvsge( $3 ); }
    | '(' TK_BVULT bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvult( $3 ); }
    | '(' TK_BVUGT bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvugt( $3 ); }
    | '(' TK_BVULE bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvule( $3 ); }
    | '(' TK_BVUGE bitvec_expression_list ')'
      { $$ = parser_egraph->mkBvuge( $3 ); }
    | '(' TK_EQ expression_list ')'
      { $$ = parser_egraph->mkEq( $3 ); }
    | '(' TK_DISTINCT expression_list ')'
      { $$ = parser_egraph->mkDistinct( $3 ); }
    | TK_STR
      { $$ = parser_egraph->mkVar( $1 ); free( $1 ); }
    | TK_TRUE
      { $$ = parser_egraph->mkTrue( ); }
    | TK_FALSE
      { $$ = parser_egraph->mkFalse( ); }
    | '(' TK_STR expression_list ')'
      { $$ = parser_egraph->mkUp( $2, $3 ); free( $2 ); }
    ;

expression: arithmetic_expression 
	    { $$ = $1; }
	  | bitvec_expression 
            { $$ = $1; }
	  | '(' TK_STR expression_list ')'
	    { $$ = parser_egraph->mkUf( $2, $3 ); free( $2 ); }
	  ;

expression_list: expression expression_list
	         { $$ = parser_egraph->cons( $1, $2 ); }
	       | expression
		 { $$ = parser_egraph->cons( $1 ); }   
               ;

arithmetic_expression: '(' TK_PLUS arithmetic_expression_list ')' 
		       { $$ = parser_egraph->mkPlus( $3 ); }
		     | '(' TK_MINUS arithmetic_expression_list ')'
		       { $$ = parser_egraph->mkMinus( $3 ); }
                     | '(' TK_TIMES arithmetic_expression_list ')'
		       { $$ = parser_egraph->mkTimes( $3 ); }
                     | '(' TK_UMINUS arithmetic_expression_list ')'
                       { $$ = parser_egraph->mkUminus( $3 ); }
                     | '(' TK_DIV TK_NUM TK_NUM ')'
                       { $$ = parser_egraph->mkNum( $3, $4 ); free( $3 ); free( $4 ); }
                     | '(' TK_DIV arithmetic_expression_list ')'
                       { $$ = parser_egraph->mkDiv( $3 ); }
                     | TK_NUM
                       { $$ = parser_egraph->mkNum( $1 ); free( $1 ); }
                     | TK_STR
                       { $$ = parser_egraph->mkVar( $1 ); free( $1 ); }
	             | TK_LET_STR
	               { $$ = parser_egraph->getDefine( $1 ); free( $1 ); }
	             | '(' TK_ITE formula expression expression ')'
	               { $$ = parser_egraph->mkIte( $3, $4, $5 ); }
	             | '(' TK_STR expression_list ')'
	               { $$ = parser_egraph->mkUf( $2, $3 ); free( $2 ); }
                     ;

arithmetic_expression_list: arithmetic_expression arithmetic_expression_list
	                    { $$ = parser_egraph->cons( $1, $2 ); }
			  | arithmetic_expression
		            { $$ = parser_egraph->cons( $1 ); }   
                          ;

bitvec_expression: '(' TK_CONCAT bitvec_expression_list ')'
		   { $$ = parser_egraph->mkConcat( $3 ); }
                 | '(' TK_EXTRACT '[' TK_NUM ':' TK_NUM ']' TK_BVNUM_NO_WIDTH ')'
		   { 
		     char tmp[64]; 
		     sprintf( tmp, "%s[%d]", $8, atoi( $4 ) - atoi( $6 ) + 1 ); 
		     $$ = parser_egraph->mkBvnum( tmp ); 
		     free( $4 ); free( $6 ); free( $8 ); 
		   }
                 | '(' TK_EXTRACT '[' TK_NUM ':' TK_NUM ']' bitvec_expression ')'
                   { $$ = parser_egraph->mkExtract( atoi( $4 ), atoi( $6 ), $8 ); free( $4 ); free( $6 ); } 
                 | '(' TK_BVAND bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvand( $3 ); }
                 | '(' TK_BVOR bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvor( $3 ); }
                 | '(' TK_BVXOR bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvxor( $3 ); }
                 | '(' TK_BVNOT bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvnot( $3 ); }
                 | '(' TK_BVADD bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvadd( $3 ); }
                 | '(' TK_BVSUB bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvsub( $3 ); }
                 | '(' TK_BVMUL bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvmul( $3 ); }
                 | '(' TK_BVNEG bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvneg( $3 ); }
                 | '(' TK_BVUDIV bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvudiv( $3 ); }
                 | '(' TK_BVUREM bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvurem( $3 ); }
                 | '(' TK_BVSDIV bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvsdiv( $3 ); }
                 | '(' TK_BVSREM bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvsrem( $3 ); }
                 | '(' TK_SIGN_EXTEND '[' TK_NUM ']' bitvec_expression ')'
		   { $$ = parser_egraph->mkSignExtend( atoi( $4 ), $6 ); free( $4 ); }
                 | '(' TK_ZERO_EXTEND '[' TK_NUM ']' bitvec_expression ')'
		   { $$ = parser_egraph->mkZeroExtend( atoi( $4 ), $6 ); free( $4 ); }
                 | '(' TK_ROTATE_LEFT '[' TK_NUM ']' bitvec_expression ')'
		   { $$ = parser_egraph->mkRotateLeft( atoi( $4 ), $6 ); free( $4 ); }
                 | '(' TK_ROTATE_RIGHT '[' TK_NUM ']' bitvec_expression ')'
		   { $$ = parser_egraph->mkRotateRight( atoi( $4 ), $6 ); free( $4 ); }
                 | '(' TK_BVLSHR bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvlshr( $3 ); }
                 | '(' TK_BVSHL bitvec_expression_list ')'
		   { $$ = parser_egraph->mkBvshl( $3 ); }
                 | TK_BVNUM
		   { $$ = parser_egraph->mkBvnum( $1 ); free( $1 ); }
		 | TK_BIT0
		   { $$ = parser_egraph->mkBvnum( const_cast< char * >( "0" ) ); }
		 | TK_BIT1
		   { $$ = parser_egraph->mkBvnum( const_cast< char * >( "1" ) ); }
                 | TK_STR
		   { $$ = parser_egraph->mkVar( $1 ); free( $1 ); }
	         | TK_LET_STR
	           { $$ = parser_egraph->getDefine( $1 ); free( $1 ); }
	         | '(' TK_ITE formula expression expression ')'
	           { $$ = parser_egraph->mkIte( $3, $4, $5 ); }
	         | '(' TK_STR expression_list ')'
	           { $$ = parser_egraph->mkUf( $2, $3 ); free( $2 ); }
                 ;

bitvec_expression_list: bitvec_expression bitvec_expression_list
		        { $$ = parser_egraph->cons( $1, $2 ); }
		      | bitvec_expression
			{ $$ = parser_egraph->cons( $1 ); }   
                      ;

%%

//=======================================================================================
// Auxiliary Routines

vector< unsigned > * createTypeList( unsigned t )
{
  vector< unsigned > * l = new vector< unsigned >;
  l->push_back( t );
  return l;
} 

vector< unsigned > * createTypeList( unsigned t, const char * size )
{
  assert( t == DTYPE_BITVEC );
  vector< unsigned > * l = new vector< unsigned >;
  const unsigned int_size = atoi( size ); 
  assert( int_size <= MAX_WIDTH );
  l->push_back( t | int_size );
  return l;
} 

vector< unsigned > * pushTypeList( vector< unsigned > * l, unsigned t )
{
  l->push_back( t );
  return l;
}

vector< unsigned > * pushTypeList( vector< unsigned > * l, unsigned t, const char * size )
{
  const unsigned int_size = atoi( size );
  assert( int_size <= MAX_WIDTH );
  l->push_back( t | int_size );
  return l;
}

void destroyTypeList( vector< unsigned > * l )
{
  delete l;
}
