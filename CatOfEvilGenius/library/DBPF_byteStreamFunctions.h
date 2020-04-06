/**
 * file: DBPF_byteStreamFunctions.h
 * author: CatOfEvilGenius
 *
 * functions for getting data from byte arrays, or converting data to byte array
**/

#ifndef DBPF_BYTE_STREAM_H_CATOFEVILGENIUS
#define DBPF_BYTE_STREAM_H_CATOFEVILGENIUS

#include <string>
using namespace std;


// least significant byte first
void uint2bytes( const unsigned int foo, unsigned char * bytes );
// least significant byte first
void bytes2uint( const unsigned char * bytes, const unsigned int byteCount, unsigned int & foo );

// most significant byte first
void uint2bytesBigEndian( const unsigned int foo, unsigned char * bytes );
// most significant byte first
void bytes2uintBigEndian( const unsigned char * bytes, const unsigned int byteCount, unsigned int & foo );

// least significant byte first
void ushort2bytes( const unsigned short foo, unsigned char * bytes );
// least significant byte first
void bytes2ushort( const unsigned char * bytes, unsigned short & foo );

// least significant byte first
void float2bytes( const float foo, unsigned char * bytes );
void float2bytesBigEndian( const float foo, unsigned char * bytes );
// least significant byte first
void bytes2float( const unsigned char * bytes, float & foo );
void bytes2floatBigEndian( const unsigned char * bytes, float & foo );

// reads byteCount sized uint from byte stream, stores in 4 byte uint, advances bytes pointer by byteCount
void readByteStream_uint(  unsigned char * & bytes, const unsigned int byteCount, unsigned int & foo );
// writes a byteCount sized int to bytes, advances bytes pointer by 4, always
void writeByteStream_uint( unsigned char * & bytes, const unsigned int foo );

// reads unsigned short (2 bytes) from bytes stream, stores in foo, advances bytes pointer by 2
void readByteStream_ushort(  unsigned char * & bytes, unsigned short & foo );
// writes unsigned short to bytes tream, advances bytes pointer by 2
void writeByteStream_ushort( unsigned char * & bytes, const unsigned short foo );

// reads float from bytes stream, stores in foo, advances bytes pointer by 4
void readByteStream_float( unsigned char * & bytes, float & foo );
void readByteStream_floatBigEndian( unsigned char * & bytes, float & foo );
// writes float to bytes stream, advances bytes pointer by 4
void writeByteStream_float( unsigned char * & bytes, const float foo );
void writeByteStream_floatBigEndian( unsigned char * & bytes, const float foo );

// reads string from byte stream, advances bytes by string size, and length at start, 1 byte
void readByteStream_str(  unsigned char * & bytes, unsigned char & strLength, char * str );
// writes string to byte stream, advanced bytes
void writeByteStream_str( unsigned char * & bytes, const unsigned char strLength, const char * str );
// writes string class object to byte stream, advanced bytes
void writeByteStream_str( unsigned char * & bytes, const string & str );

// read string from byte stream, advanced by bytes by string size, and length at start, 1 unsigned int (4 bytes)
void readByteStream_str2( unsigned char * & bytes, unsigned int & strLength, char * str );
// write string to byte stream, advanced by bytes by string size, and length at start, 1 unsigned int (4 bytes)
void writeByteStream_str2( unsigned char * & bytes, const char * str );


// read null terminated string, advance bytes pointer
void readByteStream_strNullTerminated( unsigned char * & bytes, string & str );
// write null terminated string, advance bytes pointer
void writeByteStream_strNullTerminated( unsigned char * & bytes, const string & str );

// read cSGResource, advancy bytes pointer by 21+strLength
void readByteStream_cSGResource(  unsigned char * & bytes, unsigned char & strLength, char * str );
// write cSGResource, advance bytes pointer by 21+strLength
void writeByteStream_cSGResource( unsigned char * & bytes, const unsigned char strLength, const char * str );
// write cSGResource, advance bytes pointer by 21+strLength
void writeByteStream_cSGResource( unsigned char * & bytes, const string & str );


// DBPF_BYTE_STREAM_H_CATOFEVILGENIUS
#endif
