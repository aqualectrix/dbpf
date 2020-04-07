/**
 * file: DBPF_TXTR.cpp
 * author: CatOfEvilGenius
**/

#include <cstdlib>
#include <cstdio>
#include "DBPF_types.h"
#include "DBPF_byteStreamFunctions.h"
#include "DBPF_TXTR.h"


DBPF_TXTRtype::DBPF_TXTRtype()
{
  this->mpRawBytes = NULL;
  this->mpImageData = NULL;
  clear();
}


DBPF_TXTRtype::~DBPF_TXTRtype()
{
  clear();
}


void DBPF_TXTRtype::clear()
{
  DBPF_resourceType::clear();

  mRCOL.clear();

  mstrBlockName.assign( "" );
  muBlockID = 0;
  muBlockVersion = 0;

  muWidth = 0;
  muHeight = 0;
  muFormatCode = 0;
  muMipmapCode = 0;

  mfPurpose = 0.0f;

  muOuterLoopCount = 0;
  muInnerLoopCount = 0;

  if( mpImageData != NULL )
    delete [] mpImageData;
  mpImageData = NULL;
  muImageDataSize = 0;
}


#ifdef _DEBUG
void DBPF_TXTRtype::dump( FILE * f ) const
{
  DBPF_resourceType::dump( f );

  fprintf( f, "-- RCOL --\n" );
  this->mRCOL.dump( f );
  fprintf( f, "\n" );

  fprintf( f, "-- TXTR --\n" );
  fprintf( f, "\n" );

  fprintf( f, "block name: %s\n", this->mstrBlockName.c_str() );
  char str[256];
  dbpfResourceTypeToString( this->muBlockID, str );
  fprintf( f, "block ID: %s\n", str );
  fprintf( f, "block version: %u\n", this->muBlockVersion );
  fprintf( f, "\n" );

  fprintf( f, "width:  %5u\n", this->muWidth );
  fprintf( f, "height: %5u\n", this->muHeight );
  fprintf( f, "format code: %u ", this->muFormatCode );
  switch( this->muFormatCode )
  {
    case 1: printf( "Raw ARGB\n" ); break;
    case 2: printf( "Raw RGB\n" ); break;
    case 3: printf( "8bit alpha\n" ); break;
    case 4: printf( "DXT1 RGB\n" ); break;
    case 5: printf( "DXT3 ARGB\n" ); break;
    case 6: printf( "Raw Greyscale\n" ); break;
    case 7: printf( "32bitAlt\n" ); break;
    case 8: printf( "DXT5\n" ); break;
    case 9: printf( "24bitAlt\n" ); break;
    default: printf( "???\n" ); break;
  };
  fprintf( f, "mipmap code: %u\n", this->muMipmapCode );
  fprintf( f, "purpose: %.1f ", this->mfPurpose );
  if( 1.0f == this->mfPurpose )      printf( "object\n" );
  else if( 2.0f == this->mfPurpose ) printf( "outfit\n" );
  else if( 3.0f == this->mfPurpose ) printf( "interface\n" );
  else                               printf( "???\n" );
  fprintf( f, "outer loop count: %u\n", this->muOuterLoopCount );
  fprintf( f, "image data byte count: %u\n", this->muImageDataSize );
  fprintf( f, "image data address: 0x%x\n", (size_t)(&((this->mpImageData)[0])) );

  fprintf( f, "\n" );
}
#endif


/**
 * reads a TXTR, including RCOL at beginning
**/
bool DBPF_TXTRtype::initFromByteStream( const DBPFindexType & entry,
             unsigned char * data,  const unsigned int byteCountToRead )
{
  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_TXTRtype.initFromByteStream, null data\n" );
    return false;
  }

  // not yet initialized
  this->mbInitialized = false;
  clear();

  // index entry
  this->initIndexEntry( entry );

  // raw bytes
  this->muRawBytesCount = byteCountToRead;
  this->mpRawBytes = data;

  // ---------------------------------------------------------

  // RCOL header
  unsigned char * bytes = data, * bytes2 = NULL;
  this->mRCOL.initFromByteStream( bytes, bytes2 );
  bytes = bytes2;

  // RCOL header should have had one index entry, and the RCOL ID should be TXTR

  if( this->mRCOL.getIndexItemCount() != 1 )
  { fprintf( stderr, "ERROR: TXTR rcol header, index item count is not 1, can't handle this case\n" );
    return false;
  }

  if( this->mRCOL.getIndexItem( 0 ) != DBPF_TXTR )
  { fprintf( stderr, "ERROR: TXTR rcol header, index item 0 is not TXTR, this is unexpected, can't handle this case\n" );
    return false;
  }


  // block name
  char str[256]; str[0] = '\0';
  unsigned char strLength = 0;
  readByteStream_str( bytes, strLength, str );
  if( 0 != strcmp( str, "cImageData" ) )
  { fprintf( stderr, "ERROR: DBPF_TXTRtype.initFromByteStream, block name, expected cImageData, got %s\n", str );
    return false;
  }
  this->mstrBlockName.assign( str );

  // block ID
  readByteStream_uint( bytes, 4, this->muBlockID );
  if( this->muBlockID != DBPF_TXTR )
  { fprintf( stderr, "ERROR: DBPF_TXTRtype.initFromByteStream, block ID, expected 0x%x, got 0x%x\n", DBPF_TXTR, this->muBlockID );
    return false;
  }

  // block version
  readByteStream_uint( bytes, 4, this->muBlockVersion );

  // texture name (cSGResource)
  readByteStream_cSGResource( bytes, strLength, str );
  this->mstrName.assign( str );

  // texture width
  // texture height
  readByteStream_uint( bytes, 4, this->muWidth );
  readByteStream_uint( bytes, 4, this->muHeight );

  // format code
  readByteStream_uint( bytes, 4, this->muFormatCode );

  // mipmap code
  readByteStream_uint( bytes, 4, this->muMipmapCode );

  // purpose
  unsigned int foo = 0;
  readByteStream_uint( bytes, 4, foo );
  memcpy( &(this->mfPurpose), &foo, 4 );

  // outer loop count
  readByteStream_uint( bytes, 4, this->muOuterLoopCount );

  // skip an int (always 0)
  bytes += 4;

#ifdef _DEBUG
/*
  for( unsigned int b = 0; b < 50; ++b )
    printf( "%c", bytes[b] );
  printf( "\n" );
*/
#endif

  // file description, like file name w/o _txtr
  if( 9 == this->muBlockVersion )
  {
    readByteStream_str( bytes, strLength, str );
    this->mstrDesc.assign( str );
  }


  // copy image data, raw bytes

  this->muImageDataSize = byteCountToRead - ( bytes - data );
  this->mpImageData = new unsigned char[ this->muImageDataSize ];
  if( NULL == this->mpImageData )
  { fprintf( stderr, "ERROR: TXTR couldn't allocate memory for raw image data\n" );
    return false;
  }
  memcpy( this->mpImageData, bytes, this->muImageDataSize );

  // --------------------------------------
  // done initializing
  this->mbInitialized = true;

  return true;
}


// TXTR currently has no "set" functions, can't be changed, so no need to update bytes yet, this always returns true
bool DBPF_TXTRtype::updateRawBytes()
{
  return true;
}


/**
<pre>
 * Compares the image data of two textures to see if they contain the same image.
 * Returns false in the following cases:
 * + different format code
 * + different width and/or height
 * + different muImageDataSize
 * + different mpImageData (should do a checksum comparison here)
</pre>
**/
bool DBPF_TXTRtype::isImageSameAs( const DBPF_TXTRtype * pOther ) const
{
  // format code
  if( this->muFormatCode != pOther->muFormatCode )
    return false;

  // width and height
  if( this->muWidth  != pOther->muWidth
   || this->muHeight != pOther->muHeight )
    return false;

  // image data size
  if( this->muImageDataSize != pOther->muImageDataSize )
    return false;

  // image data

  if( 0 != memcmp( this->mpImageData, pOther->mpImageData, this->muImageDataSize ) )
    return false;

  return true;
}
