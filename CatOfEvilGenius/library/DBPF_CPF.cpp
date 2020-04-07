/**
 * file:   DBPF_CPF.cpp
 * author: CatOfEvilGenius
**/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#ifdef __GNUC__
#include <ext/hash_map>
#else
#include <hash_map>
#endif

#include "DBPF_CPF.h" // DBPF_propertiesType
#include "DBPF_byteStreamFunctions.h"

using namespace std;
#ifdef __GNUC__
using namespace __gnu_cxx;
#else
using namespace stdext;
#endif

typedef pair< string, string > strPairType;
typedef pair< string, DBPF_CPFitemType > CPFpairType;



DBPF_propertiesType::DBPF_propertiesType()
{
  clear();
}


DBPF_CPFtype::DBPF_CPFtype()
{
  clear();
}


DBPF_propertiesType::~DBPF_propertiesType()
{
  clear();
}


DBPF_CPFtype::~DBPF_CPFtype()
{
  clear();
}


void DBPF_propertiesType::clear()
{
  muPropertyCount = 0;
  if( this->mPropertyKeys.empty() == false )
    this->mPropertyKeys.clear();
  if( this->mProperties.empty() == false )
    this->mProperties.clear();
}


void DBPF_CPFtype::clear()
{
  muTypeID = 0;
  muVersion = 0;

  muPropertyCount = 0;
  if( this->mPropertyKeys.empty() == false )
    this->mPropertyKeys.clear();
  if( this->mProperties.empty() == false )
    this->mProperties.clear();
}


#ifdef _DEBUG
void DBPF_propertiesType::dump( FILE * f ) const
{
  char str[256];
  char str2[256];
  fprintf( f, "property count: %u\n", this->muPropertyCount );
  for( unsigned int i = 0; i < this->muPropertyCount; ++i )
  {
    strcpy( str, (this->mPropertyKeys)[i].c_str() );
    strcpy( str2, (((this->mProperties).find( str ))->second).c_str() );
    fprintf( f, "  %s: %s\n", str, str2 );
  }
}
#endif


#ifdef _DEBUG
void DBPF_CPFtype::dump( FILE * f ) const
{
  char key[256];
  char str2[256];
  char strType[20];
  DBPF_CPFitemType item;

  fprintf( f, "CPF typeID: %x\n", this->muTypeID );
  fprintf( f, "CPF version: %i\n", this->muVersion );

  fprintf( f, "CPF property count: %u\n", this->muPropertyCount );
  for( unsigned int i = 0; i < this->muPropertyCount; ++i )
  {
    // key
    strcpy( key, this->mPropertyKeys.at(i).c_str() );

    // value
    item = (this->mProperties.find( key ))->second;

    // convert to strings
    item.typeToString( strType );
    if( 0 == strcmp( "age", key )
     || 0 == strcmp( "category", key ) )
    { sprintf( str2, "0x%x", item.miValue );
    }
    else
    { item.valueToString( str2 );
    }

    // display
    fprintf( f, "  %s: (%s) %s\n", key, strType, str2 );
  }
}
#endif


// add key/value pair, key must be unique
bool DBPF_propertiesType::addPair( string propName, string propValue )
{
  // if key is not in hash, add key/value pair
  hash_map< string, string >::const_iterator iter;
  iter = this->mProperties.find( propName );
  if( iter == this->mProperties.end() )
  {
    this->mPropertyKeys.push_back( propName );
    this->mProperties.insert( strPairType( propName, propValue ) );
    ++muPropertyCount;
    return true;
  }

  return false;
}


// add key/value pair, key must be unique
bool DBPF_CPFtype::addPair( string propName, DBPF_CPFitemType & propValue )
{
  // if key is not in hash, add key/value pair
  hash_map< string, DBPF_CPFitemType >::const_iterator iter;
  iter = this->mProperties.find( propName );
  if( iter == this->mProperties.end() )
  {
    this->mPropertyKeys.push_back( propName );
    this->mProperties.insert( CPFpairType( propName, propValue ) );
    ++muPropertyCount;
    return true;
  }

  return false;
}


// get i'th key
bool DBPF_propertiesType::getKeyAt( const unsigned int i, string & key ) const
{
  if( i >= this->muPropertyCount )
    return false;

  key = this->mPropertyKeys[i];
  return true;
}


// get i'th key
bool DBPF_CPFtype::getKeyAt( const unsigned int i, string & key ) const
{
  if( i >= this->muPropertyCount )
    return false;

  key = this->mPropertyKeys[i];
  return true;
}


bool DBPF_propertiesType::getPairAt( const unsigned int i, string & key, string & val ) const
{
  if( i >= this->muPropertyCount )
    return false;

  key = this->mPropertyKeys[i];
  return( getPropertyValue( key, val ) );
}


bool DBPF_CPFtype::getPairAt( const unsigned int i, string & key, DBPF_CPFitemType & val ) const
{
  if( i >= this->muPropertyCount )
    return false;

  key = this->mPropertyKeys[i];
  return( getPropertyValue( key, val ) );
}


/**
 * Given a property name, outputs the property value.
 * Returns false if there is no property with such a name.
**/
bool DBPF_propertiesType::getPropertyValue( const string propName, string & propValue ) const
{
  hash_map< string, string >::const_iterator iter;
  iter = this->mProperties.find( propName );
  if( iter == this->mProperties.end() )
    return false;

  propValue.assign( iter->second.c_str() );

  return true;
}


/**
 * Given a property name, outputs the property value.
 * Returns false if there is no property with such a name.
**/
bool DBPF_CPFtype::getPropertyValue( const string propName, DBPF_CPFitemType & propValue ) const
{
  hash_map< string, DBPF_CPFitemType >::const_iterator iter;
  iter = this->mProperties.find( propName );
  if( iter == this->mProperties.end() )
    return false;

  propValue = iter->second;
  propValue.mstrValue.assign( iter->second.mstrValue.c_str() );

  return true;
}


/**
<pre>
 * input:   propName - property name, must match an existing property name
 *          propValue - new value for the property
 * output:  bChanged - true if a property was changed
 *          iChangeInRawBytesCount - just what it says
 * returns: true - property was set
 *          false - property was not set because there is no such property
 *
 * purpose: set a property, if the new property value is different from the old one
</pre>
**/
bool DBPF_propertiesType::setPropertyValue( const string propName, string & propValue,
                                            bool & bChanged, int & iChangeInRawBytesCount )
{
  bChanged = false;
  iChangeInRawBytesCount = 0;

  // does this property exist?

  string strOldValue;
  if( false == this->getPropertyValue( propName, strOldValue ) )
    return false;

  // is the new value the same as the old value?

  if( 0 == strcmp( strOldValue.c_str(), propValue.c_str() ) )
    return true;

  // new value is different,
  //   set property, set changed flag
  //   compute change in raw bytes size

  (this->mProperties)[ propName ] = propValue;
  bChanged = true;
  iChangeInRawBytesCount = (unsigned int)(strlen( propValue.c_str() ) - strlen( strOldValue.c_str() ));

#ifdef _DEBUG
  printf( "old value: %s\n", strOldValue.c_str() );
  printf( "new value: %s\n", (this->mProperties)[ propName ].c_str() );
  printf( "size change: %i\n", iChangeInRawBytesCount );
#endif

  return true;
}


bool areSame( DBPF_CPFitemType & a, DBPF_CPFitemType & b)
{
  if( a.miType == b.miType )
  {
    if( CPF_STRING == b.miType )
     if( 0 == strcmp( a.mstrValue.c_str(), b.mstrValue.c_str() ) )
       return true;

    if( CPF_INT == b.miType || CPF_INT2 == b.miType )
      if( a.miValue == b.miValue )
        return true;

    if( CPF_FLOAT == b.miType )
      if( a.mfValue == b.mfValue )
        return true;

    if( CPF_BOOL == b.miType )
      if( a.mbValue == b.mbValue )
        return true;
  }

  return false;
}


// str must be preallocated
void DBPF_CPFitemType::typeToString( char * str )
{
  if( CPF_STRING == this->miType )
    strcpy( str, "string" );

  else if( CPF_INT == this->miType )
    strcpy( str, "uint" );

  else if( CPF_INT2 == this->miType )
    strcpy( str, "int" );

  else if( CPF_FLOAT == this->miType )
    strcpy( str, "float" );

  else if( CPF_BOOL == this->miType )
    strcpy( str, "bool" );

  else
    sprintf( str, "0x%x", this->miType );
}


// str must be preallocated
void DBPF_CPFitemType::valueToString( char * str )
{
  if( CPF_STRING == this->miType )
    strcpy( str, this->mstrValue.c_str() );

  else if( CPF_INT == this->miType || CPF_INT2 == this->miType )
    sprintf( str, "%i", this->miValue );

  else if( CPF_FLOAT == this->miType )
    sprintf( str, "%g", this->mfValue );

  else if( CPF_BOOL == this->miType )
  {
    if( true == this->mbValue )  strcpy( str, "true" );
    else                         strcpy( str, "false" );
  }

  else
    strcpy( str, "*** UNKNOWN TYPE ***" );
}


/**
<pre>
 * input:   propName - property name, must match an existing property name
 *          propValue - new value for the property
 * output:  bChanged - true if a property was changed
 *          iChangeInRawBytesCount - just what it says, only changes for strings
 * returns: true - property was set
 *          false - property was not set because there is no such property
 *
 * purpose: set a property, if the new property value is different from the old one
</pre>
**/
bool DBPF_CPFtype::setPropertyValue( const string propName, DBPF_CPFitemType & propValue,
                                     bool & bChanged, int & iChangeInRawBytesCount )
{
  bChanged = false;
  iChangeInRawBytesCount = 0;

  // does this property exist?

  DBPF_CPFitemType oldValue;
  if( false == this->getPropertyValue( propName, oldValue ) )
    return false;

  // is new value of the correct type?

  if( oldValue.miType != propValue.miType )
  { fprintf( stderr, "ERROR: DBPF_CPFtype.setPropertyValue, new value is of wrong type\n" );
    return false;
  }

  // is the new value the same as the old value?

  if( true == areSame( oldValue, propValue ) )
    return true;

  // new value is different,
  //   set property, set changed flag
  //   compute change in raw bytes size

  (this->mProperties)[ propName ] = propValue;
  bChanged = true;
  if( CPF_STRING == propValue.miType )
    iChangeInRawBytesCount = (unsigned int)(strlen( propValue.mstrValue.c_str() ) - strlen( oldValue.mstrValue.c_str() ));

#ifdef _DEBUG
  printf( "CPF property: %s\n", propName.c_str() );
  char foo[256], foot[20];
  oldValue.typeToString( foot );
  oldValue.valueToString( foo );
  printf( "old value: (%s) %s\n", foot, foo );
  (this->mProperties)[ propName ].typeToString( foot );
  (this->mProperties)[ propName ].valueToString( foo );
  printf( "new value: (%s) %s\n", foot, foo );
  printf( "size change: %i\n", iChangeInRawBytesCount );
#endif

  return true;
}


/**
 * input:   data - byte stream to read from
 * output:  restOfData - points to first byte after CPF data
 * returns: success/failure
**/
bool DBPF_CPFtype::initFromByteStream( unsigned char * data,
                                       unsigned char * & restOfData )
{
  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_CPFtype.initFromByteStream, null data\n" );
    return false;
  }

  // reset / clear all values
  this->clear();

  // read CPF data
  // =============

  // CPF type ID, should always be 0xCBE7505E
  unsigned int u = 0;
  readByteStream_uint( data, 4, u );
  this->muTypeID = u;
//  printf( "CPF type ID: %x\n", u ); // DEBUG

/*
  // there may be several different valid CPF type IDs
  if( u != 0xCBE7505E
   && u != 0xCBE750E0
    )
  { fprintf( stderr, "ERROR: DBPF_CPFtype.initFromByteStream, bad type ID %x\n", u );
    restOfData = data - 4; // back up to before this number
    return false;
  }
*/

  // CPF version
  unsigned short s = 0;
  readByteStream_ushort( data, s );
  this->muVersion = s;
//  printf( "CPF version: %i\n", s ); // DEBUG

  // number of items
  unsigned int itemCount = 0;
  readByteStream_uint( data, 4, itemCount );
//  printf( "number of items: %i\n", itemCount ); // DEBUG

  // read items
  char str[256];
  unsigned int strLength = 0;
  string key;
  DBPF_CPFitemType value;
  unsigned int ind = 0;

  for( ind = 0; ind < itemCount; ++ind )
  {
    // value data type
    readByteStream_uint( data, 4, u );
    value.miType = u;
//    printf( "------------\ntype: %x\n", u ); // DEBUG

    // key: property name
    readByteStream_str2( data, strLength, str );
    key.assign( str );
//    printf( "key str length: %i\n", strLength ); // DEBUG
//    printf( "key: %s\n", str );                  // DEBUG

    // value data
    // ----------

    // boolean value
    if( CPF_BOOL == value.miType )
    {
      readByteStream_uint( data, 1, u );
      if( 0 == u )  value.mbValue = false;
      else          value.mbValue = true;
//      printf( "(bool) " ); if( 0 == u ) printf( "false\n" ); else printf( "true\n" ); // DEBUG
    }

    // int value
    else if( CPF_INT == value.miType || CPF_INT2 == value.miType )
    {
      readByteStream_uint( data, 4, u );
      value.miValue = u;
//      if( CPF_INT == value.miType ) printf( "(uint) " ); else printf( "(int) " );  // DEBUG
//      printf( "0x%x, %i\n", u, u );                                                // DEBUG
    }

    // float value
    else if( CPF_FLOAT == value.miType )
    {
      float foo = 0;
      readByteStream_floatBigEndian( data, foo );
      value.mfValue = foo;
//      printf( "(float) %g\n", foo );                  // DEBUG
    }

    // string value
    else if( CPF_STRING == value.miType )
    {
      readByteStream_str2( data, strLength, str );
      value.mstrValue.assign( str );
//      printf( "(str) %s\n", str );                    // DEBUG
    }

    // unknown type
    else
    { fprintf( stderr, "ERROR: DBPF_CPFtype.initFromByteStream, unknown CPF item data type %x\n", value.miType );
      restOfData = data;
      return false;
    }

    // add key to hash table
    // ---------------------

    this->addPair( key, value );
  }
  if( ind != itemCount )
    fprintf( stderr, "ERROR: DBPF_CPFtype.initFromByteStream, read in CPF items, there were non-unique keys\n" );


  // point to first byte after CPF
  restOfData = data;

  return true;
}


bool DBPF_CPFtype::writeToByteStream( unsigned char * & bytes )
{
  // sanity check
  if( NULL == bytes )
  { fprintf( stderr, "ERROR: DBPF_CPFtype::writeToByteStream, null bytes\n" );
    return false;
  }

  // write stuff
  // --------------------

  // CPF typeID

  unsigned int u = this->muTypeID;
  writeByteStream_uint( bytes, u );

  // CPF version

  writeByteStream_ushort( bytes, this->muVersion );

  // item count

  writeByteStream_uint( bytes, this->muPropertyCount );

  // write all items

  string key;
  DBPF_CPFitemType item;
  unsigned char b = 0;
  for( unsigned int i = 0; i < this->muPropertyCount; ++i )
  {
    // get i'th item
    this->getPairAt( i, key, item );

    // item data type
    writeByteStream_uint( bytes, item.miType );

    // key name
    writeByteStream_str2( bytes, key.c_str() );

    // item value
    if( CPF_INT == item.miType || CPF_INT2 == item.miType )
      writeByteStream_uint( bytes, item.miValue );
    else if( CPF_FLOAT == item.miType )
      writeByteStream_floatBigEndian( bytes, item.mfValue );
    else if( CPF_STRING == item.miType )
      writeByteStream_str2( bytes, item.mstrValue.c_str() );
    else if( CPF_BOOL == item.miType )
    {
      if( true == item.mbValue )      b = 1;
      else                            b = 0;
      bytes[0] = b; // write one byte
      ++bytes;      // advance one byte
    }
  }

  return true;
}
