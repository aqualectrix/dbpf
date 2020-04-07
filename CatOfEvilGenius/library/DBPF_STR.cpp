/**
 * file:   DBPF_STR.cpp
 * author: CatOfEvilGenius
**/

#include <cstdlib>
#include <cstdio>
#include "DBPF_STR.h"
#include "DBPF_byteStreamFunctions.h"


void readByteStream_DBPFtext( unsigned char * & bytes, DBPF_textType & text )
{
  // language code, 1 byte
  text.mLanguageCode = bytes[0];
  bytes += 1;

  // value text
  readByteStream_strNullTerminated( bytes, text.mstrValueText );

  // description text
  readByteStream_strNullTerminated( bytes, text.mstrDescText );
}


void writeByteStream_DBPFtext( unsigned char * & bytes, const DBPF_textType & text )
{
  // language code, 1 byte
  bytes[0] = text.mLanguageCode;
  bytes += 1;

  // value text
  writeByteStream_strNullTerminated( bytes, text.mstrValueText );

  // description text
  writeByteStream_strNullTerminated( bytes, text.mstrDescText );
}


void DBPF_STRtype::clear()
{
  DBPF_resourceType::clear();

  this->msFormatCode = 0;
  this->msTextItemCount = 0;
  if( false == this->mTextItems.empty() )
    this->mTextItems.clear();
}


bool DBPF_STRtype::initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead )
{
  clear();

  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_STRtype.initFromByteStream, null data\n" );
    return false;
  }

  // index entry
  this->initIndexEntry( entry );

  // raw bytes
  this->muRawBytesCount = byteCountToRead;
  this->mpRawBytes = data;

  // -----------------------------------

  char str[256];
  unsigned char * bytes = data;

  // file name, always 64 bytes, null terminated

  memcpy( str, bytes, 64 );
  str[64] = '\0';
  this->mstrName.assign( str );
  bytes += 64;

  // format code
  readByteStream_ushort( bytes, this->msFormatCode );

  // number of text items
  readByteStream_ushort( bytes, this->msTextItemCount );

  // text items

  DBPF_textType text;
  for( unsigned short i = 0; i < this->msTextItemCount; ++i )
  {
    readByteStream_DBPFtext( bytes, text );
    this->mTextItems.push_back( text );
  }

  // -----------------------------------

  // done initializing
  this->mbInitialized = true;

  return true;
}


bool DBPF_STRtype::updateRawBytes()
{
  // no change needed?
  if( this->mbChanged == false )
    return true;

  // allocate new bytes

  unsigned int newByteCount = (unsigned int)((int)(this->muRawBytesCount) + this->miChangeInRawBytesCount);
  unsigned char * bytes = new unsigned char[ newByteCount ];
  if( NULL == bytes )
    return false;
  unsigned char * bytesStart = bytes;


  // update bytes
  // ------------

  // file name, 64 bytes
  for( int i = 0; i < 64; ++i )
    bytes[i] = 0;
  unsigned char * bytes2 = bytes;
  writeByteStream_strNullTerminated( bytes2, this->mstrName ); // ptr won't advance 64, only advance length of string
  bytes += 64;

  // format code
  writeByteStream_ushort( bytes, this->msFormatCode );

  // text item count
  writeByteStream_ushort( bytes, this->msTextItemCount );

  // text items
  for( size_t i = 0; i < this->mTextItems.size(); ++i )
    writeByteStream_DBPFtext( bytes, (this->mTextItems)[i] );

#ifdef _DEBUG
  size_t bytesWrittenCount = bytes - bytesStart;
  if( bytesWrittenCount != (size_t)newByteCount )
  { fprintf( stderr, "ERROR: DBPF_STRtype.updateRawBytes, bad array size\n" );
    return false;
  }
#endif


  // done updating
  // -------------

  // new raw bytes
  if( this->mpRawBytes != NULL )
    delete [] this->mpRawBytes;
  this->mpRawBytes = bytesStart;
  bytes = NULL;

  // new byte count
  this->muRawBytesCount = newByteCount;

  // zero change in size
  this->miChangeInRawBytesCount = 0;

  // unset changed flag
  this->mbChanged = false;

  return true;
}


#ifdef _DEBUG
void DBPF_STRtype::dump( FILE * f ) const
{
  DBPF_resourceType::dump( f );

  fprintf( f, "-- STR --\n\n" );

  fprintf( f, "name: %s\n", this->mstrName.c_str() );
  fprintf( f, "format: %i\n", this->msFormatCode );
  fprintf( f, "text item count: %i\n", this->msTextItemCount );

  for( unsigned short i = 0; i < this->msTextItemCount; ++i )
  {
    fprintf( f, "  language: %i\n", (this->mTextItems)[i].mLanguageCode );
    fprintf( f, "  value text: %s\n", (this->mTextItems)[i].mstrValueText.c_str() );
    fprintf( f, "  desc text:  %s\n", (this->mTextItems)[i].mstrDescText.c_str() );
  }

  fprintf( f, "\n" );
}
#endif


bool DBPF_STRtype::getTextItem( const unsigned short index, DBPF_textType & text )
{
  if( (size_t)index >= this->mTextItems.size() )
  { fprintf( stderr, "ERROR: DBPF_STRtype.getTextItem, index out of bounds\n" );
    return false;
  }

  text.mLanguageCode = (this->mTextItems)[index].mLanguageCode;
  text.mstrValueText = (this->mTextItems)[index].mstrValueText;
  text.mstrDescText  = (this->mTextItems)[index].mstrDescText;

  return true;
}


// change value of existing text item
bool DBPF_STRtype::setTextItem( const unsigned short index, const DBPF_textType & text )
{
  if( (size_t)index >= this->mTextItems.size() )
  { fprintf( stderr, "ERROR: DBPF_STRtype.setTextItem, index out of bounds\n" );
    return false;
  }

  size_t oldLen = 3 + strlen( (this->mTextItems)[index].mstrValueText.c_str() )
                    + strlen( (this->mTextItems)[index].mstrDescText.c_str() );

  size_t newLen = 3 + strlen( text.mstrValueText.c_str() )
                    + strlen( text.mstrDescText.c_str() );

  (this->mTextItems)[index].mLanguageCode = text.mLanguageCode;
  (this->mTextItems)[index].mstrValueText = text.mstrValueText;
  (this->mTextItems)[index].mstrDescText  = text.mstrDescText;

  this->miChangeInRawBytesCount += (int)( newLen - oldLen );
  this->mbChanged = true;

  return true;
}


// add text item to end of list
void DBPF_STRtype::addTextItem( const DBPF_textType & text )
{
  // add item to end of list
  this->mTextItems.push_back( text );

  // increment item count
  this->msTextItemCount += 1;

  // mark as changed
  this->miChangeInRawBytesCount += (int)(3 + strlen( text.mstrValueText.c_str() )
                                           + strlen( text.mstrDescText.c_str() ));
  this->mbChanged = true;
}


// remove a text item (and compact list to close hole)
bool DBPF_STRtype::removeTextItem( const unsigned short index )
{
  // error check
  if( (size_t)index >= this->mTextItems.size() )
  { fprintf( stderr, "ERROR: DBPF_STRtype.removeTextItem, index out of bounds\n" );
    return false;
  }

  // size in bytes of item to remove
  size_t len = 3 + strlen( (this->mTextItems)[index].mstrValueText.c_str() )
                 + strlen( (this->mTextItems)[index].mstrDescText.c_str() );

  // compact list
  for( size_t i = index; i < this->mTextItems.size() - 1; ++i )
  {
    (this->mTextItems)[i] = (this->mTextItems)[i+1];
  }

  // remove item at end of list
  this->mTextItems.pop_back();

  // mark as changed
  this->miChangeInRawBytesCount -= (int)len;
  this->mbChanged = true;

  return true;
}
