/**
 * file:   DBPF_CPF.h
 * author: CatOfEvilGenius
 *
 * CPF is a data format used in several DBPF resource types.
 * This file also has the properties class (key/value, string values only).
**/

#ifndef H_DBPF_CPF_CATOFEVILGENIUS
#define H_DBPF_CPF_CATOFEVILGENIUS

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>

using namespace std;
#ifdef __GNUC__
using namespace __gnu_cxx;
#else
using namespace stdext;
#endif


#define CPF_BOOL   0xCBA908E1
#define CPF_INT    0xEB61E4F7 // uint
#define CPF_INT2   0x0C264712 // int
#define CPF_FLOAT  0xABC78708
#define CPF_STRING 0x0B8BEA18

class DBPF_CPFitemType
{
public:
  DBPF_CPFitemType() {}
  ~DBPF_CPFitemType() {}

  unsigned int miType;  // indicates if value is a bool, int, float, or string
  bool mbValue;         // bool value
  unsigned int miValue; // int value
  float mfValue;        // float value
  string mstrValue;     // string value

  void typeToString( char * str );  // converts item type to a string
  void valueToString( char * str );      // converts item value to a string

};

bool areSame( DBPF_CPFitemType & a, DBPF_CPFitemType & b );


/**
 * helper class for DBPF_resourceType,
 * for resources that have CPF data (key/value pairs, values can be non-strings)
**/
class DBPF_CPFtype
{
public:
  DBPF_CPFtype();
  ~DBPF_CPFtype();

  void clear();

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

  bool initFromByteStream( unsigned char * data, unsigned char * & restOfData );
  bool writeToByteStream( unsigned char * & bytes );

  unsigned int getPropertyCount() const { return muPropertyCount; }
  bool getKeyAt( const unsigned int i, string & key ) const;
  bool getPairAt( const unsigned int i, string & key, DBPF_CPFitemType & val ) const;

  bool getPropertyValue( const string propName, DBPF_CPFitemType & propValue ) const;
  bool setPropertyValue( const string propName, DBPF_CPFitemType & propValue, bool & bChanged,
                                      int & iChangeInRawBytesCount );
  bool addPair( string propName, DBPF_CPFitemType & propValue );

protected:
  unsigned int muTypeID;
  unsigned short muVersion;

  // number of properties
  unsigned int muPropertyCount;
  // names of all properties
  vector< string > mPropertyKeys;
  // hashmap of property name and property value pairs, get names from mPropertyKeys
  unordered_map< string, DBPF_CPFitemType > mProperties;
};



/**
 * helper class for DBPF_resourceType, for those resources that have propery name/value pairs (values are srings only),
 * the TXMT class uses this
**/
class DBPF_propertiesType
{
public:
  DBPF_propertiesType();
  ~DBPF_propertiesType();

  void clear();

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

  unsigned int getPropertyCount() const { return muPropertyCount; }
  bool getKeyAt( const unsigned int i, string & key ) const;
  bool getPairAt( const unsigned int i, string & key, string & val ) const;

  bool getPropertyValue( const string propName, string & propValue ) const;
  bool setPropertyValue( const string propName, const string & propValue, bool & bChanged,
                                      int & iChangeInRawBytesCount );
  bool addPair( string propName, string propValue );

protected:
  // number of properties
  unsigned int muPropertyCount;
  // names of all properties
  vector< string > mPropertyKeys;
  // hashmap of property name and property value pairs, get names from mPropertyKeys
  unordered_map< string, string > mProperties;

};


// H_DBPF_CPF_CATOFEVILGENIUS
#endif
