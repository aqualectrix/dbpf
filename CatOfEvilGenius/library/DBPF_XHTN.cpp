#include <cstdlib>
#include <cstdio>
#include <string>

#include "DBPF_XHTN.h"



bool DBPF_XHTNtype::setFamily( string family )
{
  DBPF_CPFitemType item;
  item.miType = CPF_STRING;
  item.mstrValue = family;
  return( this->setPropertyValue( "family", item ) );
}


/**
 * Sets the following properties:
 * 1 name (string): Black, Brown, Blond, Red
 * 2 genetic (float): black/brown 1, blond/red 2
 * 3 proxy (string)
**/
bool DBPF_XHTNtype::setHairColor( int color )
{
  DBPF_CPFitemType item, item2, item3;
  item.miType = CPF_STRING;
  item2.miType = CPF_FLOAT;
  item3.miType = CPF_STRING;

  if( HAIR_BLACK == color )
  {
    item.mstrValue.assign( "Black" );
    item2.mfValue = 1;
    item3.mstrValue.assign( "00000001-0000-0000-0000-000000000000" );
  }
  else if( HAIR_BROWN == color )
  {
    item.mstrValue.assign( "Brown" );
    item2.mfValue = 1;
    item3.mstrValue.assign( "00000002-0000-0000-0000-000000000000" );
  }
  else if( HAIR_BLOND == color )
  {
    item.mstrValue.assign( "Blond" );
    item2.mfValue = 2;
    item3.mstrValue.assign( "00000003-0000-0000-0000-000000000000" );
  }
  else if( HAIR_RED == color )
  {
    item.mstrValue.assign( "Red" );
    item2.mfValue = 2;
    item3.mstrValue.assign( "00000004-0000-0000-0000-000000000000" );
  }
  else if( HAIR_GREY == color )
  {
    item.mstrValue.assign( "Grey" );
    item2.mfValue = 0;
    item3.mstrValue.assign( "00000005-0000-0000-0000-000000000000" );
  }
  else
  {
    fprintf( stderr, "ERROR: DBPF_XHTNtype.setHairColor, unknown color %i\n", color );
    return false;
  }

  bool bSuccess = true;
  if( false == this->setPropertyValue( "name", item ) )
    bSuccess = false;
  if( false == this->setPropertyValue( "genetic", item2 ) )
    bSuccess = false;
  if( false == this->setPropertyValue( "proxy", item3 ) )
    bSuccess = false;

  return bSuccess;
}
