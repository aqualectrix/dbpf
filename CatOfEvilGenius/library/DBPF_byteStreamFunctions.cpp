/**
 * file: DBPF_byteStreamFunctions.cpp
 * author: CatOfEvilGenius
 *
 * functions for getting data from byte arrays, or converting data to byte array
**/

#include <cstdlib>
#include <cstdio>
#include <string>
using namespace std;


/**
<pre>
 * input:   bytes - array of bytes, length 1 to 4
 *          byteCount - array length in bytes, 1 to 4
 * output:  foo - resulting unsigned integer
 * returns: none
 *
 * purpose: convert a byte array to an unsigned int,
 *          if given a length two array with values { 0x0A, 0x0B },
 *          the resulting 4 byte integer will have bytes 0x 00 00 0A 0B
</pre>
**/
void bytes2uintBigEndian( const unsigned char * bytes, const unsigned int byteCount, unsigned int & foo )
{
  foo = 0;
  foo += bytes[0];

  for( unsigned int i = 1; i < byteCount; ++i )
  {
    foo = foo << 8;
    foo += bytes[i];
  }
}


/**
<pre>
 * input:   foo - an unsigned integer (4 bytes)
 * output:  bytes - must be an array of at least size 4, output is in bytes 0 to 3
 * returns: none
 *
 * purpose: convert an unsigned int to a byte array,
 *          input: 0x0A0B0C0D
 *          output: { 0x0A, 0x0B, 0x0C, 0x0D }
</pre>
**/
void uint2bytesBigEndian( const unsigned int _foo, unsigned char * bytes )
{
  unsigned int foo = _foo;
  unsigned int mask = 0x000000FF;

  bytes[3] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[2] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[1] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[0] = (unsigned char)(foo & mask);
}


/**
<pre>
 * input:   bytes - array of bytes, length 1 to 4
 *          byteCount - array length in bytes, 1 to 4
 * output:  foo - resulting unsigned integer
 * returns: none
 *
 * purpose: convert a byte array to an unsigned int,
 *
 *          if given a length two array with values { 0x0A, 0x0B },
 *          the resulting 4 byte integer will have bytes 0x 00 00 0B 0A    REVERSED!!!
 * 
 *          if given a length three array with values { 0x0A, 0x0B, 0x0C },
 *          the resulting 4 byte integer will have bytes 0x 00 0C 0B 0A    REVERSED!!!
 * 
 *          if given a length three array with values { 0x0A, 0x0B, 0x0C, 0x0D },
 *          the resulting 4 byte integer will have bytes 0x 0D 0C 0B 0A    REVERSED!!!
</pre>
**/
void bytes2uint( const unsigned char * bytes, const unsigned int byteCount, unsigned int & foo )
{
  foo = 0;

  unsigned int i = byteCount-1;
  for( unsigned int k = 0; k < byteCount; ++k )
  {
    foo = foo << 8;
    foo += bytes[i];
    --i;
  }
}


/**
<pre>
 * input:   foo - an unsigned integer (4 bytes)
 * output:  bytes - must be an array of at least size 4, output is in bytes 0 to 3
 * returns: none
 *
 * purpose: convert an unsigned int to a byte array,
 *          input: 0x0A0B0C0D
 *          output: { 0x0D, 0x0C, 0x0B, 0x0A }   REVERSED!!!
</pre>
**/
void uint2bytes( const unsigned int _foo, unsigned char * bytes )
{
  unsigned int foo = _foo;
  unsigned int mask = 0x000000FF;

  bytes[0] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[1] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[2] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[3] = (unsigned char)(foo & mask);
}


/**
<pre>
 * input:   bytes - byte array, must be length at least 2
 * output:  foo - unsigned short value (2 bytes)
 * returns: none
 *
 * purpose: Convert a byte array to an unsigned short value.
 *          bytes { 0x0A, 0x0B } -->  foo 0x0B0A   REVERSED!
</pre>
**/
void bytes2ushort( const unsigned char * bytes, unsigned short & foo )
{
  foo = 0;

  foo += bytes[1];  foo = foo << 8;
  foo += bytes[0];
}


/**
<pre>
 * input:   _foo, an unsigned short value, 2 bytes
 * output:  bytes - must be preallocated, and size at least 2
 * returns: none
 *
 * purpose: Convert an unsigned short to a byte array.
 *          input: 0x0A0B
 *          output: { 0x0B, 0x0A }   REVERSED!
</pre>
**/
void ushort2bytes( const unsigned short _foo, unsigned char * bytes )
{
  unsigned short foo = _foo;
  unsigned short mask = 0x00FF;

  bytes[0] = (unsigned char)(foo & mask);  foo = foo >> 8;
  bytes[1] = (unsigned char)(foo & mask);
}


// least significant byte first
void bytes2float( const unsigned char * bytes, float & foo )
{
  float bar = 0;
  unsigned char tmp[4];

  tmp[0] = bytes[3];
  tmp[1] = bytes[2];
  tmp[2] = bytes[1];
  tmp[3] = bytes[0];

  memcpy( &bar, tmp, 4 );
  foo = bar;
}


void bytes2floatBigEndian( const unsigned char * bytes, float & foo )
{
  float bar = 0;
  unsigned char tmp[4];

  tmp[0] = bytes[0];
  tmp[1] = bytes[1];
  tmp[2] = bytes[2];
  tmp[3] = bytes[3];

  memcpy( &bar, tmp, 4 );
  foo = bar;
}


// least significant byte first
void float2bytes( const float _foo, unsigned char * bytes )
{
  float foo = _foo;
  unsigned char tmp[4];
  memcpy( tmp, &foo, 4 );

  bytes[0] = tmp[3];
  bytes[1] = tmp[2];
  bytes[2] = tmp[1];
  bytes[3] = tmp[0];
}


void float2bytesBigEndian( const float _foo, unsigned char * bytes )
{
  float foo = _foo;
  unsigned char tmp[4];
  memcpy( tmp, &foo, 4 );

  bytes[0] = tmp[0];
  bytes[1] = tmp[1];
  bytes[2] = tmp[2];
  bytes[3] = tmp[3];
}


/**
<pre>
 * in/out:  bytes - array of bytes
 * input:   byteCount - how many bytes to read (may be 1 to 4)
 * output:  foo - resulting unsigned integer
 * returns: none
 *
 * purpose: read an unsigned int from a byte array, and ADVANCE THE ARRAY POINTER
 *          to the next byte after what was read
 *
 *          if given a length two array with values { 0x0A, 0x0B },
 *          the resulting 4 byte integer will have bytes 0x 00 00 0B 0A    REVERSED!!!
 * 
 *          if given a length three array with values { 0x0A, 0x0B, 0x0C },
 *          the resulting 4 byte integer will have bytes 0x 00 0C 0B 0A    REVERSED!!!
 * 
 *          if given a length three array with values { 0x0A, 0x0B, 0x0C, 0x0D },
 *          the resulting 4 byte integer will have bytes 0x 0D 0C 0B 0A    REVERSED!!!
</pre>
**/
void readByteStream_uint( unsigned char * & bytes, const unsigned int byteCount, unsigned int & foo )
{
  bytes2uint( bytes, byteCount, foo );
  bytes += byteCount;
}


/**
<pre>
 * input:   foo - an int, 4 bytes long
 * in/out:  bytes - byte array to write int to,
 *                  after int is written, bytes pointer advances by 4, always
 * returns:
</pre>
**/
void writeByteStream_uint( unsigned char * & bytes, const unsigned int foo )
{
  uint2bytes( foo, bytes );
  bytes += 4;
}


/**
<pre>
 * in/out:  bytes - a byte stream (array, really)
 * output:  foo - 2 byte value read from the byte stream
 * returns: none
 *
 * purpose: Read a 2 byte unsigned short value, and advance the bytes array pointer.
 *          Given bytes, 0x0B 0x0A, the value will be 0x0A0B  REVERSED!
</pre>
**/
void readByteStream_ushort( unsigned char * & bytes, unsigned short & foo )
{
  bytes2ushort( bytes, foo );
  bytes += 2;
}


/**
<pre>
 * input:   foo - an unsigned short, 2 bytes long
 * in/out:  bytes - byte array to write short to,
 *                  after short is written, bytes pointer advances by 2
 * returns:
</pre>
**/
void writeByteStream_ushort( unsigned char * & bytes, const unsigned short foo )
{
  ushort2bytes( foo, bytes );
  bytes += 2;
}


void readByteStream_float( unsigned char * & bytes, float & foo )
{
  bytes2float( bytes, foo );
  bytes += 4;
}


void readByteStream_floatBigEndian( unsigned char * & bytes, float & foo )
{
  bytes2floatBigEndian( bytes, foo );
  bytes += 4;
}


void writeByteStream_float( unsigned char * & bytes, const float foo )
{
  float2bytes( foo, bytes );
  bytes += 4;
}


void writeByteStream_floatBigEndian( unsigned char * & bytes, const float foo )
{
  float2bytesBigEndian( foo, bytes );
  bytes += 4;
}


/**
<pre>
 * in/out:  bytes - array of bytes, bytes pointer gets advanced by stringSize
 * output:  strLength - number of non-null bytes in string, not including null at end,
 *                      I append a null at the end in the output
 *          str - text string, must be *preallocated*, 256 bytes will suffice, since max length is 255
 * returns: none
 *
 * purpose: reads string from byte stream, advances bytes by string size
 *          string is length + characters
 *
 *          string encoding: length is in 1st byte
</pre>
**/
void readByteStream_str( unsigned char * & bytes, unsigned char & strLength, char * str )
{
  strLength = (unsigned int)( bytes[0] );
  for( unsigned char i = 0; i < strLength; ++i )
    str[i] = bytes[i+1];

  str[ strLength ] = '\0';

  bytes += strLength + 1;  // +1 is for the byte with the string size
}


// as other version, but length is an unsigned int, not a byte
void readByteStream_str2( unsigned char * & bytes, unsigned int & strLength, char * str )
{
  // read string length and
  //   advance byte stream pointer
  unsigned int foo = 0;
  readByteStream_uint( bytes, 4, foo );
  strLength = foo;

  // read string characters
  for( unsigned char i = 0; i < strLength; ++i )
    str[i] = bytes[i];

  str[ strLength ] = '\0';

  // advancy byte stream pointer
  bytes += strLength;
}


/**
<pre>
 * in/out:  bytes - byte array with a null terminated string
 * output:  str - string object
 * returns: none
 *
 * purpose: Read a null terminated string from a byte stream.
 *          Advance the bytes pointer to after the string.
 *          Max length, 1024, including null.
</pre>
**/
void readByteStream_strNullTerminated( unsigned char * & bytes, string & str )
{
  // determine string length, not including null character
  size_t length = 0;
  while( length < 1024 )
  {
    if( '\0' == bytes[ length ] )
      break;
    ++length;
  }

  // copy string text and null at end
  static char strArray[1024];
  strArray[0] = '\0';
  if( length > 0 )
    memcpy( strArray, bytes, length );
  if( length < 1024 )
    strArray[length] = '\0';
  else
    strArray[1023] = '\0';
  str.assign( strArray );

  // advance bytes pointer
  if( length < 1024 )
    bytes += (length+1);
  else
    bytes += 1024;
}


/**
<pre>
 * in/out:  bytes - a byte array
 * input:   str - null terminated string to write to the byte array
 * returns: none
 *
 * purpose: Write a null terminated string to a byte array, including null at end,
 *          advance bytes pointer past null.
</pre>
**/
void writeByteStream_strNullTerminated( unsigned char * & bytes, const string & str )
{
  // sanity check
  if( NULL == str.c_str() )
    return;

  // copy string and null
  size_t len = strlen( str.c_str() );
  for( size_t i = 0; i < len; ++i )
    bytes[i] = (str.c_str())[i];
  bytes[len] = '\0';
  
  // advance bytes pointer
  bytes += (len+1);
}


/**
<pre>
 * input:   strLength - string length, number of characters in string, not counting any null terminating character
 *          str - a string
 * in/out:  bytes - byte array, must be preallocated, string will be written to it,
 *                  afterwards, bytes will point to the character right after the end of the string,
 *                  so you can continue writing other stuff
 * returns: none
 *
 * purpose: write a string to a byte array, string is written as length (1 char), followed by
 *          all the characters of the string, no null terminating character at end,
 *          After writing the string, bytes will point to the character right after the end of the string,
 *          so you can continue writing other stuff
</pre>
**/
void writeByteStream_str( unsigned char * & bytes, const unsigned char strLength, const char * str )
{
  // write string length (byte)
  bytes[0] = strLength;

  // write string characters
  for( unsigned char i = 0; i < strLength; ++i )
    bytes[i+1] = str[i];

  // advance byte steam pointer
  bytes += strLength + 1; // +1 is for the byte with the string size
}


// as other version, but string length is unsigned int, not byte
void writeByteStream_str2( unsigned char * & bytes, const char * str )
{
  // write string length (unsigned int)
  // and advance string pointer
  unsigned int strLength = strlen( str );
  writeByteStream_uint( bytes, strLength );

  // write string characters
  for( unsigned char i = 0; i < strLength; ++i )
    bytes[i] = str[i];

  // advance byte steam pointer
  bytes += strLength;
}


/**
<pre>
 * read a string (1 byte - length, other bytes - text)
 * convenient wrapper for other version of writeByteStream_str
</pre>
**/
void writeByteStream_str( unsigned char * & bytes, const string & str )
{
  const char * _str = str.c_str();
  unsigned char strLength = (unsigned char)(strlen( _str ));
  writeByteStream_str( bytes, strLength, _str );
}


/**
<pre>
 * in/out:  bytes - array of bytes, bytes pointer gets advanced by stringSize
 * output:  strLength - number of non-null bytes in string, not including null at end,
 *                      I append a null at the end in the output
 *          str - text string, must be *preallocated*, 256 bytes will suffice, since max length is 255
 * returns: none
 *
 * purpose: reads a cSGResource from byte stream, advances bytes by string size + 21
 *          cSGResource is the following
 *            1 byte for length of string, value will be 11
 *            11 bytes of string "cSGResource"
 *            4 byte uint - value 0
 *            4 byte uint - value 2
 *            1 byte length of a string, value n
 *            n bytes - some string
</pre>
**/
void readByteStream_cSGResource( unsigned char * & bytes, unsigned char & strLength, char * str )
{
  bytes += 20; // skip "cSGResource", 0, and 2
  // now read string length and string
  readByteStream_str( bytes, strLength, str );
}


/**
<pre>
 * Writes a DBPF cSGResource.
 * That's a string, "cSGResource", followed by a DWORD 0 and a DWORD 2,
 * and then the user supplied string, usually a file name.
 *
 * bytes pointer advances by ( strLength + 21 ) = 1+11 + 4 + 4 + 1+strLength
</pre>
**/
void writeByteStream_cSGResource( unsigned char * & bytes, const unsigned char strLength, const char * str )
{
  const unsigned char lenCSGR = 11;
  const char strCSGR[12] = "cSGResource"; // 11 non-null characters

  writeByteStream_str( bytes, lenCSGR, strCSGR );
  writeByteStream_uint( bytes, 0 );
  writeByteStream_uint( bytes, 2 );
  writeByteStream_str( bytes, strLength, str ); // user input string
}


/**
 * convenient wrapper for other version of writeByteStream_cSGResource
**/
void writeByteStream_cSGResource( unsigned char * & bytes, const string & str )
{
  const char * _str = str.c_str();
  unsigned char strLength = (unsigned char)(strlen( _str ));
  writeByteStream_cSGResource( bytes, strLength, _str );
}

