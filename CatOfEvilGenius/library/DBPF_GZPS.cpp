#include <cstdlib>
#include <cstdio>
#include <string>

#include "DBPF_GZPS.h"



unsigned int DBPF_GZPStype::getAge() const
{
  DBPF_CPFitemType item;
  if( false == this->getPropertyValue( "age", item ) )
    return 0;
  return item.miValue;
}

string DBPF_GZPStype::getFamily() const
{
  DBPF_CPFitemType item;
  if ( false == this->getPropertyValue( "family", item ) )
    return "";
  return item.mstrValue;
}


bool DBPF_GZPStype::setFamily( string family )
{
  DBPF_CPFitemType item;
  item.miType = CPF_STRING;
  item.mstrValue = family;
  return( this->setPropertyValue( "family", item ) );
}

/**
  * Sets hairtone to an arbitrary string value. Use setHairColor() for an easier
  * interface for standard hair colors.
 **/
bool DBPF_GZPStype::setHairtone(string hairtone)
{
  DBPF_CPFitemType item;
  item.miType = CPF_STRING;
  item.mstrValue.assign(hairtone);

  return this->setPropertyValue("hairtone", item);
}


/**
 * Sets the following properties:
 * genetic (float): black/brown 1, blond/red 2, grey 0
 * hairtone (string)
**/
bool DBPF_GZPStype::setHairColor( int color )
{
  DBPF_CPFitemType item2, item3;
  item2.miType = CPF_FLOAT;
  item3.miType = CPF_STRING;

  if( HAIR_BLACK == color )
  {
    item2.mfValue = 1;
    item3.mstrValue.assign( "00000001-0000-0000-0000-000000000000" );
  }
  else if( HAIR_BROWN == color )
  {
    item2.mfValue = 1;
    item3.mstrValue.assign( "00000002-0000-0000-0000-000000000000" );
  }
  else if( HAIR_BLOND == color )
  {
    item2.mfValue = 2;
    item3.mstrValue.assign( "00000003-0000-0000-0000-000000000000" );
  }
  else if( HAIR_RED == color )
  {
    item2.mfValue = 2;
    item3.mstrValue.assign( "00000004-0000-0000-0000-000000000000" );
  }
  else if( HAIR_GREY == color )
  {
    item2.mfValue = 0;
    item3.mstrValue.assign( "00000005-0000-0000-0000-000000000000" );
  }
  else
  {
    fprintf( stderr, "ERROR: DBPF_GZPStype.setHairColor, unknown color %i\n", color );
    return false;
  }

  bool bSuccess = true;
  if( false == this->setPropertyValue( "genetic", item2 ) )
    bSuccess = false;
  if( false == this->setPropertyValue( "hairtone", item3 ) )
    bSuccess = false;

  return bSuccess;
}
