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

#ifndef ENODE_H
#define ENODE_H

#include "EnodeTypes.h"

class Enode
{
public:
  
  //
  // Constructor for Enil
  //
  Enode  ( );                                          
  //
  // Constructor for symbols
  //
  Enode ( const int    // id 
        , const char   // symbol/number
        , const char   // arity
        , const int    // type
        , const char * // name/value
      );
  //
  // Constructor for terms and lists
  //
  Enode ( const int  // id
        , const char // term/list
	, Enode *    // car
	, Enode *    // cdr
        );
  //
  // Constructor for defs
  //
  Enode ( const int  // id
	, Enode *    // def
	);		
  //
  // Destructor
  //
  ~Enode ( );
  //
  // Check if a node is Enil
  //
  inline bool isEnil            ( ) const { return id == ENODE_ID_ENIL; }
  inline bool isList            ( ) const { return etype == ENODE_TYPE_LIST; }
  inline bool isTerm            ( ) const { return etype == ENODE_TYPE_TERM; }
  inline bool isSymb            ( ) const { return etype == ENODE_TYPE_SYMBOL; }
  inline bool isNumb            ( ) const { return etype == ENODE_TYPE_NUMBER; }
  inline bool isDef             ( ) const { return etype == ENODE_TYPE_DEF; }
  //
  // Check if a term node represents a certain symbol
  //
  inline bool isPlus            ( ) const { return hasSymbolId( ENODE_ID_PLUS	    ); }
  inline bool isMinus           ( ) const { return hasSymbolId( ENODE_ID_MINUS	    ); }
  inline bool isUminus          ( ) const { return hasSymbolId( ENODE_ID_UMINUS	    ); }
  inline bool isTimes           ( ) const { return hasSymbolId( ENODE_ID_TIMES	    ); }
  inline bool isDiv             ( ) const { return hasSymbolId( ENODE_ID_DIV	    ); }
  inline bool isEq              ( ) const { return hasSymbolId( ENODE_ID_EQ	    ); }
  inline bool isLeq             ( ) const { return hasSymbolId( ENODE_ID_LEQ	    ); }
  inline bool isGeq             ( ) const { return hasSymbolId( ENODE_ID_GEQ	    ); }
  inline bool isLt              ( ) const { return hasSymbolId( ENODE_ID_LT	    ); }
  inline bool isGt              ( ) const { return hasSymbolId( ENODE_ID_GT	    ); }
  inline bool isBvslt           ( ) const { return hasSymbolId( ENODE_ID_BVSLT       ); }
  inline bool isBvsgt           ( ) const { return hasSymbolId( ENODE_ID_BVSGT       ); }
  inline bool isBvsleq          ( ) const { return hasSymbolId( ENODE_ID_BVSLEQ      ); }
  inline bool isBvsgeq          ( ) const { return hasSymbolId( ENODE_ID_BVSGEQ      ); }
  inline bool isBvult           ( ) const { return hasSymbolId( ENODE_ID_BVULT       ); }
  inline bool isBvugt           ( ) const { return hasSymbolId( ENODE_ID_BVUGT       ); }
  inline bool isBvuleq          ( ) const { return hasSymbolId( ENODE_ID_BVULEQ      ); }
  inline bool isBvugeq          ( ) const { return hasSymbolId( ENODE_ID_BVUGEQ      ); }
  inline bool isConcat          ( ) const { return hasSymbolId( ENODE_ID_CONCAT      ); }
  inline bool isBvand           ( ) const { return hasSymbolId( ENODE_ID_BVAND       ); }
  inline bool isBvor            ( ) const { return hasSymbolId( ENODE_ID_BVOR        ); }
  inline bool isBvxor           ( ) const { return hasSymbolId( ENODE_ID_BVXOR       ); }
  inline bool isBvnot           ( ) const { return hasSymbolId( ENODE_ID_BVNOT       ); }
  inline bool isBvadd           ( ) const { return hasSymbolId( ENODE_ID_BVADD       ); }
  inline bool isBvsub           ( ) const { return hasSymbolId( ENODE_ID_BVSUB       ); }
  inline bool isBvmul           ( ) const { return hasSymbolId( ENODE_ID_BVMUL       ); }
  inline bool isBvneg           ( ) const { return hasSymbolId( ENODE_ID_BVNEG       ); }
  inline bool isBvlshr          ( ) const { return hasSymbolId( ENODE_ID_BVLSHR      ); }
  inline bool isBvshl           ( ) const { return hasSymbolId( ENODE_ID_BVSHL       ); }
  inline bool isBvsrem          ( ) const { return hasSymbolId( ENODE_ID_BVSREM      ); }
  inline bool isBvurem          ( ) const { return hasSymbolId( ENODE_ID_BVUREM      ); }
  inline bool isBvsdiv          ( ) const { return hasSymbolId( ENODE_ID_BVSDIV      ); }
  inline bool isBvudiv          ( ) const { return hasSymbolId( ENODE_ID_BVUDIV      ); }
  inline bool isSign_extend     ( ) const { return hasSymbolId( ENODE_ID_SIGN_EXTEND ); }
  inline bool isZero_extend     ( ) const { return hasSymbolId( ENODE_ID_ZERO_EXTEND ); }
  inline bool isImplies         ( ) const { return hasSymbolId( ENODE_ID_IMPLIES     ); }
  inline bool isAnd             ( ) const { return hasSymbolId( ENODE_ID_AND         ); }
  inline bool isOr              ( ) const { return hasSymbolId( ENODE_ID_OR          ); }
  inline bool isNot             ( ) const { return hasSymbolId( ENODE_ID_NOT         ); }
  inline bool isIff             ( ) const { return hasSymbolId( ENODE_ID_IFF         ); }
  inline bool isXor             ( ) const { return hasSymbolId( ENODE_ID_XOR         ); }
  inline bool isTrue            ( ) const { return hasSymbolId( ENODE_ID_TRUE        ); }
  inline bool isFalse           ( ) const { return hasSymbolId( ENODE_ID_FALSE       ); }
  inline bool isIte             ( ) const { return hasSymbolId( ENODE_ID_ITE         ); }
  inline bool isIfthenelse      ( ) const { return hasSymbolId( ENODE_ID_IFTHENELSE  ); }
  inline bool isDistinct        ( ) const { return hasSymbolId( ENODE_ID_DISTINCT    ); }
  inline bool isUp              ( )       { return car->id > ENODE_ID_LAST && isAtom( ); }

  bool isExtract                ( int * msb, int * lsb );
  inline bool isExtract         ( ) { int lsb, msb; return isExtract( &msb, &lsb ); }
                                              
  inline int  width             ( ) const { assert( false ); return 0; }

  bool        isVar             ( );       // True if it is a variable
  bool        isConstant        ( );       // True if it is a constant
  bool        isUf              ( );       // True if it is an uninterpreted function
  bool        isLit             ( );       // True if it is a literal
  bool        isAtom            ( );       // True if it is an atom
  bool        isTAtom           ( );       // True if it is a theory atom
  bool        isBooleanOperator ( );       // True if it is a boolean operator

  inline bool hasCongData       ( ) { return cong_data != NULL; }

  void       allocCongData      ( ); 

  //
  // Getty and Setty methods
  //
  inline int     getId          ( ) const { return id; }
  inline char    getArity       ( ) const { assert( isSymb( ) ); assert( symb_data ); return symb_data->arity; }
  inline int     getType        ( ) const { assert( isSymb( ) || isNumb( ) ); assert( symb_data ); return symb_data->type; }
  inline char *  getName        ( ) const { assert( isSymb( ) || isNumb( ) ); assert( symb_data ); return symb_data->name; }
  inline Enode * getCar         ( ) const { return car; }
  inline Enode * getCdr         ( ) const { return cdr; }
  inline Enode * getDef         ( ) const { assert( isDef( ) ); assert( car ); return car; }            

  inline Enode * getNext        ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->next; }
  inline int     getSize        ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->size; }
  inline Enode * getParent      ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->parent; }
  inline Enode * getSameCar     ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->same_car; }
  inline Enode * getSameCdr     ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->same_cdr; }
  inline int     getParentSize  ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->parent_size; }
  inline Enode * getCgPtr       ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->cg_ptr; }
  inline Elist * getForbid      ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->forbid; }
  inline dist_t  getDistClasses ( ) const { assert( isTerm( ) || isList( ) ); assert( cong_data ); return cong_data->dist_classes; }

  const Real &   getValue          ( ) const;
  Enode *        getRoot           ( ) const;
  int            getCid            ( ) const;
  Enode *        getConstant       ( ) const;

  inline Enode * getExpParent      ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_parent; }
  inline Enode * getExpRoot        ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_root; }
  inline int     getExpClassSize   ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_class_size; }
  inline Enode * getExpHighestNode ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_highest_node; }
  inline Enode * getExpReason      ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_reason; }
  inline int     getExpTimeStamp   ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->exp_time_stamp; }

  inline lbool   getPolarity       ( ) const { assert( isTerm( ) && atom_data ); return atom_data->polarity; }
  inline lbool   getDeduced        ( ) const { assert( isTerm( ) && atom_data ); return atom_data->deduced; }
  inline int     getDedIndex       ( ) const { assert( isTerm( ) && atom_data ); return atom_data->ded_index; }
  inline int     getDistIndex      ( ) const { assert( isTerm( ) && atom_data ); return atom_data->dist_index; }

  inline Enode * getCb             ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return cong_data->term_data->cb; }
  inline Enode * getRef            ( ) const { assert( isTerm( ) && cong_data && cong_data->term_data ); return getConstant( ) == NULL ? cong_data->root : getConstant( ); }

  inline void    setValue          ( const Real & v )       { assert( cong_data->term_data->value ); *(cong_data->term_data->value) = v; }            
  inline void    setRoot           ( Enode * e )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->root = e; }
  inline void    setCid            ( const int c )          { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->cid = c; }               
  inline void    setDef            ( Enode * e )            { assert( e ); assert( isDef( ) ); car = e; }            
  inline void    setNext           ( Enode * e )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->next = e; }
  inline void    setSize           ( const int s )          { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->size = s; }
  inline void    setParent         ( Enode * e )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->parent = e; }          
  inline void    setSameCar        ( Enode * e )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->same_car = e; }     
  inline void    setSameCdr        ( Enode * e )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->same_cdr = e; }       
  inline void    setParentSize     ( const int s )          { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->parent_size = s; }       
  inline void    setCgPtr          ( Enode * e )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->cg_ptr = e; }            
  inline void    setForbid         ( Elist * l )            { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->forbid = l; }            
  inline void    setDistClasses    ( const dist_t & d )     { assert( isTerm( ) || isList( ) ); assert( cong_data ); cong_data->dist_classes = d; }            
                                                            
  inline void    setConstant       ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); assert( e == NULL || e->isConstant( ) ); cong_data->term_data->constant = e; }                  
  inline void    setExpParent      ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_parent = e; }             
  inline void    setExpRoot        ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_root = e; }                   
  inline void    setExpClassSize   ( const int s )          { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_class_size = s; }            
  inline void    setExpHighestNode ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_highest_node = e; }                
  inline void    setExpReason      ( Enode * e )            { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_reason = e; }                    
  inline void    setExpTimeStamp   ( const int t )          { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->exp_time_stamp = t; }             
  inline void    setPolarity       ( const lbool p )        { assert( isTerm( ) && atom_data ); atom_data->polarity = p; } 
  inline void    setDeduced        ( const lbool d, int i ) { assert( isTerm( ) && atom_data ); atom_data->deduced = d; atom_data->ded_index = i; }                     
  inline void    setDistIndex      ( const int d )	    { assert( isTerm( ) && atom_data ); atom_data->dist_index = d; }                
  
  inline void    setCb             ( Enode * e )	    { assert( isTerm( ) && cong_data && cong_data->term_data ); cong_data->term_data->cb = e; }                            
  // 
  // Congruence closure related routines
  //
  void                 addParent            ( Enode * );   // Adds a parent to this node. Useful for congruence closure
  void                 removeParent         ( Enode * );   // Remove a parent of this node
  //
  // Shortcuts for retrieving a term's arguments
  //
  inline Enode *       get1st               ( ) const;     // Get first argument in constant time
  inline Enode *       get2nd               ( ) const;     // Get second argument in constant time 
  inline Enode *       get3rd               ( ) const;     // Get third argument in constant time

  bool                 addToCongruence      ( ) const;
  unsigned             sizeInMem            ( ) const;

  void                 print	            ( ostream & ); // Prints the enode
  void                 printSig	            ( ostream & ); // Prints the enode signature

  inline const Pair( int ) getSig    ( )                         const { return make_pair( car->getRoot( )->getCid( ), cdr->getRoot( )->getCid( ) ); }
  inline friend ostream &  operator<<( ostream & os, Enode * e )       { assert( e ); e->print( os ); return os; }

private:
  //
  // Standard informations for terms
  //
  const int   id;         // Node unique identifier
  const char  etype;      // Term, List, Def, Symbol, Number
  Enode *     car;        // For car / defs
  Enode *     cdr;        // For cdr
  union {
  CongData *  cong_data;  // For terms and lists that go in the congruence
  SymbData *  symb_data;  // For symbols/numbers
  };
  AtomData *  atom_data;  // For atom terms only

  inline bool hasSymbolId  ( const int id ) const { assert( isTerm( ) ); return car->getId( ) == id; }
};

inline const Real & Enode::getValue ( ) const 
{ 
  assert( isTerm( ) || isNumb( ) );
  if ( isTerm( ) )
  {
    assert( isTerm( ) );
    assert( cong_data );
    assert( cong_data->term_data );
    assert( cong_data->term_data->value );
    return *(cong_data->term_data->value); 
  }
  
  assert( symb_data );
  return *(symb_data->value);
}

inline Enode * Enode::getRoot ( ) const 
{ 
  assert( !isDef( ) ); 
  if ( (isTerm( ) || isList( )) && cong_data != NULL ) 
    return cong_data->root;
  return const_cast<Enode *>(this);
}

inline int Enode::getCid ( ) const 
{ 
  assert( !isDef( ) );
  if ( (isTerm( ) || isList( )) && cong_data )
    return cong_data->cid;
  return id;
}

inline Enode * Enode::getConstant ( ) const 
{ 
  assert( isTerm( ) || isList( ) ); 
  assert( cong_data ); 
  if ( isTerm( ) ) 
  {
    assert( cong_data );
    assert( cong_data->term_data );
    return cong_data->term_data->constant;
  }

  return NULL;
}

//
// enode is a literal if it is 
// an atom or a negated atom
//
inline bool Enode::isLit( )
{
  if ( !isTerm( ) ) return false;
  if ( isAtom( ) ) return true;          
  if ( car->getArity( ) != ENODE_ARITY_1 ) return false;
  Enode * arg = get1st( );
  return isNot( ) && arg->isAtom( );   
}
//
// enode is an atom if it has boolean type and
// it is not a boolean operator. Constants true
// and false are considered atoms
//
inline bool Enode::isAtom( )
{
  if ( !isTerm( ) ) return false;
  if ( car->getType( ) != TYPE_BOOL ) return false; 
  if ( isBooleanOperator( ) ) return false; 

  return true;
}
//
// enode is a tatom if it has boolean type and
// it is not a boolean operator nor a boolean variable
// nor constants true and false.
//
inline bool Enode::isTAtom( )
{
  if ( !isTerm( ) ) return false;
  if ( !isAtom( ) ) return false;
  if ( isVar( ) ) return false;
  if ( isTrue( ) ) return false;
  if ( isFalse( ) ) return false;
  return true;
}

inline bool Enode::isVar( )
{
  if ( car->getId( )    <= ENODE_ID_LAST ) return false; // If the code is predefined, is not a variable
  if ( isConstant( ) ) return false;                     // If it's a constant is not a variable
  if ( !isTerm( ) ) return false;		         // If is not a term is not a variable
  if ( car->getArity( ) != ENODE_ARITY_0 ) return false; // If arity != 0 is not a variable
  return car->isSymb( );	                         // Final check 
}

inline bool Enode::isConstant( )
{
  if ( !isTerm( ) ) return false;		         // Check is a term
  return isTrue( ) || isFalse( ) || car->isNumb( );      // Only numbers, true, false are constant
}

inline bool Enode::isBooleanOperator( )
{
  return isAnd( ) || isOr( )  || isNot( ) 
      || isIff( ) || isXor( ) || isImplies( ) 
      || isIfthenelse( );
}

inline Enode * Enode::get1st ( ) const
{
  assert( etype == ENODE_TYPE_TERM );
  assert( car->getArity( ) > ENODE_ARITY_0 );
  return getCdr( )->getCar( );
}

inline Enode * Enode::get2nd ( ) const
{
  assert( etype == ENODE_TYPE_TERM );
  assert( car->getArity( ) > ENODE_ARITY_1 );
  return getCdr( )->getCdr( )->getCar( );
}

inline Enode * Enode::get3rd ( ) const
{
  assert( etype == ENODE_TYPE_TERM );
  assert( car->getArity( ) > ENODE_ARITY_2 );
  return getCdr( )->getCdr( )->getCdr( )->getCar( );
}

inline bool Enode::isExtract ( int * msb, int * lsb ) 
{ 
  assert( isTerm( ) );
  return sscanf( car->getName( ), "extract[%d:%d]", msb, lsb ) == 2; 
}

inline unsigned Enode::sizeInMem( ) const 
{
  unsigned size = sizeof( Enode );
  if ( isSymb( ) )
  {
    assert( symb_data );
    assert( symb_data->name );
    return size + sizeof( SymbData ) + 8 * strlen( symb_data->name );
  }
  if ( isNumb( ) )
  {
    assert( symb_data );
    assert( symb_data->name );
    assert( symb_data->value );
    return size + sizeof( SymbData ) + 8 * strlen( symb_data->name ) + sizeof( symb_data->value );
  }
  if ( atom_data ) size += sizeof( AtomData );
  if ( cong_data ) 
  {
    size += sizeof( CongData );
    if ( cong_data->term_data ) size += sizeof( TermData );
  }
  return size;
}

inline void Enode::allocCongData( ) 
{ 
  assert( isTerm( ) || isList( ) );
  assert( cong_data == NULL ); 
  cong_data = new CongData( id, const_cast<Enode *>(this) ); 
  if ( isTerm( ) )
    cong_data->term_data = new TermData( const_cast<Enode *>(this) );
}

#endif
