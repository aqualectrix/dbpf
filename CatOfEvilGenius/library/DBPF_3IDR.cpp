/**
 * file:   DBPF_3IDR.cpp
 * author: CatOfEvilGenius
**/

#include <cstdlib>
#include <cstdio>
#include <DBPF_3IDR.h>
#include "DBPF_byteStreamFunctions.h"
#include "DBPF_types.h"



DBPF_3IDRtype::DBPF_3IDRtype()
{
  this->mpRawBytes = NULL;
  clear();
}


DBPF_3IDRtype::~DBPF_3IDRtype()
{
  clear();
}


void DBPF_3IDRtype::clear()
{
  this->mu3IDR_ID = 0;
  this->muIndexType = 0;
  this->muTGIRcount = 0;
  DBPF_resourceType::clear();

  if( this->mTGIRs.empty() == false )
    this->mTGIRs.clear();
}


/**
<pre>
</pre>
**/
bool DBPF_3IDRtype::initFromByteStream(
                  const DBPFindexType & entry,
                  unsigned char * data,
                  const unsigned int byteCountToRead )
{
  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_3IDRtype.initFromByteStream, null data\n" );
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

  // -------------------------------------------------------------

  unsigned char * bytes = data;

  // 3DIR ID, should be 0xDEADBEEF
  readByteStream_uint( bytes, 4, this->mu3IDR_ID );
  if( 0xDEADBEEF != this->mu3IDR_ID )
  { fprintf( stderr, "ERROR: reading 3IDR, expected 0xDEADBEEF, got %x\n", this->mu3IDR_ID );
    return false;
  }

  // index type, should be 1 or 2
  readByteStream_uint( bytes, 4, this->muIndexType );
  if( this->muIndexType < 1 || this->muIndexType > 2 )
  { fprintf( stderr, "ERROR: reading 3IDR, expected index type 1 or 2, got %u\n", this->muIndexType );
    return false;
  }

  // number of TGIR entries
  readByteStream_uint( bytes, 4, this->muTGIRcount );

  // TGIR entries

  DBPF_TGIRtype TGIR;

  for( unsigned int i = 0; i < this->muTGIRcount; ++i )
  {
    // type
    readByteStream_uint( bytes, 4, TGIR.muTypeID );

    // group
    readByteStream_uint( bytes, 4, TGIR.muGroupID );

    // instance
    readByteStream_uint( bytes, 4, TGIR.muInstanceID );

    // resource
    if( 2 == this->muIndexType )
      readByteStream_uint( bytes, 4, TGIR.muResourceID );
    else
      TGIR.muResourceID = 0;

    // add to entries
    this->mTGIRs.push_back( TGIR );
  }

  // -------------------------------------------------------------
  // done initializing

  this->mbInitialized = true;

  return true;
}


#ifdef _DEBUG
void DBPF_3IDRtype::dump( FILE * f ) const
{
  DBPF_resourceType::dump( f );

  fprintf( f, "-- 3IDR --\n\n" );

  fprintf( f, "id: %x\n", this->mu3IDR_ID );
  fprintf( f, "index type: %u\n", this->muIndexType );
  fprintf( f, "# of records: %u\n", this->muTGIRcount );

  if( false == this->mTGIRs.empty() )
  {
    char str[256];
    for( unsigned int i = 0; i < this->muTGIRcount; ++i )
    {
      dbpfResourceTypeToString( this->mTGIRs[i].muTypeID, str );
      fprintf( f, "type:  %s\n", str );
      fprintf( f, "group: %x\n", this->mTGIRs[i].muGroupID );
      fprintf( f, "inst:  %x\n", this->mTGIRs[i].muInstanceID );
      if( 2 == this->muIndexType )
        fprintf( f, "resrc: %x\n", this->mTGIRs[i].muResourceID );
    }
  }

  fprintf( f, "\n" );
}
#endif


// returns false if there are not TGIR entries, or index is out of bounds
bool DBPF_3IDRtype::getTGIRentry( const unsigned int index,        // IN
                                  DBPF_TGIRtype & TGIR ) const     // OUT
{
  if( true == this->mTGIRs.empty() )
    return false;

  if( index >= this->mTGIRs.size() )
    return false;

  // -------------------------

  TGIR = this->mTGIRs[ index ];
  return true;
}


// change value of existing TGIR entry
bool DBPF_3IDRtype::setTGIRentry( const unsigned int index, const DBPF_TGIRtype & TGIR )
{
  if( true == this->mTGIRs.empty() )
    return false;

  if( index >= this->mTGIRs.size() )
    return false;

  // -------------------------

  if( TGIR != this->mTGIRs[ index ] )
  {
    this->mTGIRs[ index ] = TGIR;
    this->mbChanged = true;
  }

  return true;
}


// add a new TGIR entry to end of TGIR entries
void DBPF_3IDRtype::addTGIRentry( const DBPF_TGIRtype & TGIR )
{
  // add new TGIR
  this->mTGIRs.push_back( TGIR );

  // increment TGIR item count
  this->muTGIRcount += 1;

  // mark as changed
  if( 1 == this->muIndexType )
    this->miChangeInRawBytesCount += 12;
  else
    this->miChangeInRawBytesCount += 16;
  this->mbChanged = true;
}


// remove an existing TGIR entry
bool DBPF_3IDRtype::removeTGIRentry( const unsigned index )
{
  if( true == this->mTGIRs.empty() )
    return false;

  if( index >= this->mTGIRs.size() )
    return false;

  // -------------------------

  // move entries after removed entry up one
  if( index < this->mTGIRs.size() - 1 )
  {
    for( size_t i = index; i < this->mTGIRs.size() - 1; ++i )
      this->mTGIRs[i] = this->mTGIRs[i+1];
  }

  // remove the last entry
  this->mTGIRs.pop_back();

  this->muTGIRcount -= 1;
  if( 1 == this->muIndexType )
    this->miChangeInRawBytesCount -= 12;
  else
    this->miChangeInRawBytesCount -= 16;
  this->mbChanged = true;

  return true;
}


bool DBPF_3IDRtype::updateRawBytes()
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


  // "write" new bytes
  // -----------------

  // 3IDR ID
  writeByteStream_uint( bytes, this->mu3IDR_ID );

  // index type
  writeByteStream_uint( bytes, this->muIndexType );

  // TGIR entry count
  writeByteStream_uint( bytes, this->muTGIRcount );

  // TGIR entries
  for( unsigned int i = 0; i < this->muTGIRcount; ++i )
  {
    // type
    writeByteStream_uint( bytes, this->mTGIRs[i].muTypeID );

    // group
    writeByteStream_uint( bytes, this->mTGIRs[i].muGroupID );

    // instance
    writeByteStream_uint( bytes, this->mTGIRs[i].muInstanceID );

    // resource
    if( 2 == this->muIndexType )
      writeByteStream_uint( bytes, this->mTGIRs[i].muResourceID );
  }


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
