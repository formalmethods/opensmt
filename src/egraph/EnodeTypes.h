/*********************************************************************
Author: Roberto Bruttomesso <roberto.bruttomesso@unisi.ch>

OpenSMT -- Copyright (C) 2008, Roberto Bruttomesso

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

#ifndef ENODE_TYPES_H
#define ENODE_TYPES_H

#include "SolverTypes.h"
#include "global.h"
#include "Otl.h"

//
// Predefined list of identifiers to allow 
// fast term creation for common operators
// Except for extract, which is created 
// on demand
// 
#define ENODE_ID_UNDEF	          (-2)
#define ENODE_ID_ENIL	          (-1)
#define ENODE_ID_TRUE              (0)
#define ENODE_ID_FALSE             (1)
#define ENODE_ID_PLUS		   (2)
#define ENODE_ID_MINUS	           (3)
#define ENODE_ID_UMINUS	           (4)
#define ENODE_ID_TIMES	           (5)
#define ENODE_ID_DIV		   (6)
#define ENODE_ID_EQ	           (7)
#define ENODE_ID_LEQ	           (8)
#define ENODE_ID_GEQ	           (9)
#define ENODE_ID_LT	          (10)
#define ENODE_ID_GT	          (11)
#define ENODE_ID_BVSLT            (12) 
#define ENODE_ID_BVSGT            (13)
#define ENODE_ID_BVSLEQ           (14)
#define ENODE_ID_BVSGEQ           (15)
#define ENODE_ID_BVULT            (16)
#define ENODE_ID_BVUGT            (17)
#define ENODE_ID_BVULEQ           (18)
#define ENODE_ID_BVUGEQ           (19)
#define ENODE_ID_CONCAT           (20)
#define ENODE_ID_DISTINCT         (21)
#define ENODE_ID_BVAND            (22) 
#define ENODE_ID_BVOR             (23)
#define ENODE_ID_BVXOR            (24)
#define ENODE_ID_BVNOT            (25)
#define ENODE_ID_BVADD            (26)
#define ENODE_ID_BVSUB            (27)
#define ENODE_ID_BVMUL            (28)
#define ENODE_ID_BVNEG            (29)
#define ENODE_ID_BVLSHR           (30)
#define ENODE_ID_BVSHL            (31)
#define ENODE_ID_BVSREM           (32) 
#define ENODE_ID_BVUREM           (33)
#define ENODE_ID_BVSDIV           (34)
#define ENODE_ID_BVUDIV           (35)
#define ENODE_ID_SIGN_EXTEND      (36)
#define ENODE_ID_ZERO_EXTEND      (37)
#define ENODE_ID_IMPLIES          (38)
#define ENODE_ID_AND              (39)
#define ENODE_ID_OR               (40)
#define ENODE_ID_NOT              (41)
#define ENODE_ID_IFF		  (42) 
#define ENODE_ID_XOR              (43)
#define ENODE_ID_ITE              (44)
#define ENODE_ID_IFTHENELSE       (45)
#define ENODE_ID_LAST		  (45) // This must be equal to the last predefined ID

// TODO: turn into bit-fields
//
// Various enode types
//
const char ENODE_TYPE_UNDEF  = 0;
const char ENODE_TYPE_SYMBOL = 1; 
const char ENODE_TYPE_NUMBER = 2; 
const char ENODE_TYPE_LIST   = 3; 
const char ENODE_TYPE_TERM   = 4;
const char ENODE_TYPE_DEF    = 5;
//
// Various enode arities
//
const char ENODE_ARITY_0 = 0;
const char ENODE_ARITY_1 = 1;
const char ENODE_ARITY_2 = 2;
const char ENODE_ARITY_3 = 3;
const char ENODE_ARITY_N = 4;
//
// Sort type
//
typedef enum
{
    TYPE_UNDEF  = 0x0000
  , TYPE_BOOL   = 0x0100
  , TYPE_REAL   = 0x0200
  , TYPE_INT    = 0x0400
  , TYPE_ARIT   = TYPE_REAL | TYPE_INT
  , TYPE_BITVEC = 0x0800
  , TYPE_U      = 0x1000
  , TYPE_I      = 0x2000
};

//
// Forward declaration
//
class Enode;
//
// Datatype for distinctions
//
typedef uint32_t dist_t;    
//
// Data structure used to store forbid lists 
//
struct Elist
{
  Elist * link;             // Link to the next element in the list
  Enode * e;                // Enode that differs from this
  Enode * reason;           // Reason for this distinction
};
//
// Data used for terms in congruence only 
//
struct TermData
{
  TermData ( Enode * e )
    : value            ( NULL )
    , exp_parent       ( NULL )
    , exp_root         ( e )
    , exp_class_size   ( 1 )
    , exp_highest_node ( e )
    , exp_reason       ( NULL )
    , exp_time_stamp   ( 0 )
    , constant         ( NULL )
    , cb               ( e )
    , width            ( 0 )
  { }

  Real *            value;            // The value
  Enode *           exp_parent;       // Parent in the explanation tree
  Enode *           exp_root;         // Compressed parent in the eq classes of the explanations
  int               exp_class_size;   // Size of the eq class of the explanation
  Enode *           exp_highest_node; // Highest node of the class
  Enode *           exp_reason;       // Reason for the merge of this and exp_parent
  int               exp_time_stamp;   // Time stamp for NCA
  Enode *           constant;         // Store the constant the node is currently equivalent with
  Enode *           cb;               // Pointer for coarsest base
  int               width;            // Width for bv terms
  vector< Enode * > enode;            // This is the field T_Unknown.enode
};
//
// Data used for congruence closure, for
// both terms and lists 
//
struct CongData
{
  CongData ( const int id
           , Enode * e ) 
    : root         ( e )
    , cid          ( id )
    , next         ( e )
    , size         ( 1 )
    , parent       ( NULL )
    , same_car     ( NULL )
    , same_cdr     ( NULL )
    , parent_size  ( 0 )
    , cg_ptr       ( e )
    , forbid       ( NULL )
    , dist_classes ( 0 )
    , term_data    ( NULL )
  { }

  ~CongData ( )
  {
    if ( term_data ) delete term_data;
  }

  Enode *    root;           // Quick find
  int        cid;            // Congruence id. It may change
  Enode *    next;           // Next element in equivalence class
  int        size;           // Size of the eq class of this node
  Enode *    parent;         // Parent in the congruence
  Enode *    same_car;       // Circular list of all the car-parents of the congruence
  Enode *    same_cdr;       // Circular list of all the cdr-parents of the congruence
  int        parent_size;    // Size of the parents of this eq class
  Enode *    cg_ptr;         // Congruence pointer. Parent node in the Fischer-Galler tree
  Elist *    forbid;         // List of enodes unmergable with this
  dist_t     dist_classes;   // Stores info about distictions
  TermData * term_data;      // Stores info about terms
};
//
// Data used for atom terms only
//
struct AtomData
{
  AtomData ( )
    : polarity    ( l_Undef )
    , deduced     ( l_Undef )
    , ded_index   ( -2 )
    , dist_index  ( -1 )
  { }

  lbool   polarity;         // Associated polarity on the trail
  lbool   deduced;          // Associated deduced polarity. l_Undef means not deduced
  int     ded_index;        // Index of the solver that deduced this atom
  int     dist_index;       // If this node is a distinction, dist_index is the index in dist_classes that refers to this distinction
};
//
// Data for symbols and numnbers
//
struct SymbData
{
  // 
  // Constructor for Symbols
  //
  SymbData ( const char   arity_
           , const int    type_
	   , const char * name_ )
      : arity ( arity_ )
      , type  ( type_ )
      , value ( NULL )
  { 
    name = new char[ strlen( name_ ) + 1 ];
    strcpy( name, name_ );
  }
  // 
  // Constructor for Numbers
  //
  SymbData ( const int    type_,
             const char * name_ )
    : arity ( ENODE_ARITY_0 )
    , type  ( type_ )
  { 
    name = new char[ 32 ];
    value = new Real( atof( name_ ) );
    sprintf( name, "%lf", *value );
  }

  ~SymbData ( )
  {
    assert( name );
    delete [] name;
    if ( value )
      delete value;
  }

  const char arity;
  const int  type;
  char *     name;
  Real *     value;
};  

#endif
