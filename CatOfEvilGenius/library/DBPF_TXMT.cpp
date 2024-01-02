/**
 * file: DBPF_TXMT.cpp
 * author: CatOfEvilGenius
 *
 * material definition data, TXMT class
**/

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include "DBPF_types.h"
#include "DBPF_TXMT.h"
#include "DBPF_byteStreamFunctions.h"



DBPF_TXMTtype::DBPF_TXMTtype()
{
  this->mpRawBytes = NULL;
  clear();
}


DBPF_TXMTtype::~DBPF_TXMTtype()
{
  clear();
}


void DBPF_TXMTtype::clear()
{
  DBPF_resourceType::clear();

  mRCOL.clear();

  mstrBlockName.assign( "" );
  muBlockID = 0;
  muBlockVersion = 0;

  mstrMaterialType.assign( "" );

  muTextureNameCount = 0;
  if( this->mTextureNames.empty() == false )
    this->mTextureNames.clear();
}

bool DBPF_TXMTtype::getSubsetName( string & subsetName )
{
    string texName;
    if ( this->getPropertyValue( "stdMatBaseTextureName", texName ) )
    {
        // remove ##0xabcdef01! from the front
        texName.erase( 0, 13 );
        // remove ~stdMatBaseTextureName from the end
        texName.erase( texName.length() - 22, texName.length() );

        subsetName = texName;
        return true;
    }

    return false;
}


#ifdef _DEBUG
void DBPF_TXMTtype::dump( FILE * f ) const
{
  DBPF_resourceType::dump( f );

  fprintf( f, "-- RCOL --\n" );
  this->mRCOL.dump( f );
  fprintf( f, "\n" );

  fprintf( f, "-- TXMT --\n" );
  fprintf( f, "\n" );

  fprintf( f, "block name: %s\n", this->mstrBlockName.c_str() );
  char str[256];
  dbpfResourceTypeToString( this->muBlockID, str );
  fprintf( f, "block ID: %s\n", str );
  fprintf( f, "block version: %u\n", this->muBlockVersion );
  fprintf( f, "\n" );

  fprintf( f, "material type: %s\n", this->mstrMaterialType.c_str() );
  fprintf( f, "\n" );

  this->mpProperties->dump( f );
  fprintf( f, "\n" );

  fprintf( f, "textures: %u\n", this->muTextureNameCount );
  for( unsigned int i = 0; i < this->muTextureNameCount; ++i )
    fprintf( f, "  %u: %s\n", i, (this->mTextureNames)[i].c_str() );

  fprintf( f, "\n" );
}
#endif


/**
 * reads a TXMT (including an RCOL header at the beginning)
**/
bool DBPF_TXMTtype::initFromByteStream( const DBPFindexType & entry,
                             unsigned char * data, const unsigned int byteCountToRead )
{
  clear();

  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_TXMTtype.initFromByteStream, null data\n" );
    return false;
  }

  // index entry
  this->initIndexEntry( entry );

  // raw bytes
  this->muRawBytesCount = byteCountToRead;
  this->mpRawBytes = data;

  // ---------------------------------------

  // read RCOL header first
  unsigned char * bytes = data, * bytes2 = NULL;
  this->mRCOL.initFromByteStream( bytes, bytes2 );
  bytes = bytes2;

  // RCOL header should have had one index entry, and the RCOL ID should be TXMT

  if( this->mRCOL.getIndexItemCount() != 1 )
  { fprintf( stderr, "ERROR: TXMT rcol header, index item count is not 1, can't handle this case\n" );
    return false;
  }

  if( this->mRCOL.getIndexItem( 0 ) != DBPF_TXMT )
  { fprintf( stderr, "ERROR: TXMT rcol header, index item 0 is not TXMT, this is unexpected, can't handle this case\n" );
    return false;
  }


  // block name
  char str[256]; str[0] = '\0';
  unsigned char strLength = 0;
  readByteStream_str( bytes, strLength, str );
  this->mstrBlockName.assign( string( str ) );
  if( 0 != strcmp( this->mstrBlockName.c_str(), "cMaterialDefinition" ) )
  { fprintf( stderr, "ERROR: expected cMaterialDefinition, got %s\n", this->mstrBlockName );
    return false;
  }
//  printf( "block name: %s\n", str );

  // block ID
  readByteStream_uint( bytes, 4, this->muBlockID );
  if( this->muBlockID != DBPF_TXMT )
  { fprintf( stderr, "ERROR: expected TXMT value, 0x%x, got 0x%x\n", DBPF_TXMT, this->muBlockID );
    return false;
  }

  // block version
  readByteStream_uint( bytes, 4, this->muBlockVersion );

  // material name in cSGResource
  readByteStream_cSGResource( bytes, strLength, str );
  this->mstrName.assign( (string)( str ) );
//  printf( "name: %s\n", this->mstrName.c_str() );

  // material description
  readByteStream_str( bytes, strLength, str );
  this->mstrDesc.assign( str );
//  printf( "desc: %s\n", this->mstrDesc.c_str() );

  // material type
  readByteStream_str( bytes, strLength, str );
  this->mstrMaterialType.assign( str );
//  printf( "mat type: %s\n", this->mstrMaterialType.c_str() );

  // properties
  // ----------

  if( NULL == this->mpProperties )
  {
    this->mpProperties = new DBPF_propertiesType();
    if( NULL == this->mpProperties )
    { fprintf( stderr, "ERROR: DBPF_TXMTtype, initFromByteStream, failed to allocate properties\n" );
      return false;
    }
  }

  // property count
  unsigned int val = 0;
  readByteStream_uint( bytes, 4, val );
//  printf( "prop count: %u\n", val );

  // properties

  this->mpProperties->clear();

  char str2[256]; str2[0] = '\0';
  for( unsigned int i = 0; i < val; ++i )
  {
    // property name, property value
    readByteStream_str( bytes, strLength, str );
    readByteStream_str( bytes, strLength, str2 );
    this->mpProperties->addPair( str, str2 );
#ifdef _DEBUG
//    printf( "%s: %s\n", str, str2 );
#endif
  } // properties loop


  // texture names

  if( this->muBlockVersion > 8 )
  {
    // texture name count
    readByteStream_uint( bytes, 4, this->muTextureNameCount );
//    printf( "texture name count: %u\n", this->muTextureNameCount );

    // texture names
    for( unsigned int i = 0; i < this->muTextureNameCount; ++i )
    {
      readByteStream_str( bytes, strLength, str );
      this->mTextureNames.push_back( string( str ) );
//      printf( "  %s\n", str );
    }
  }

  // -------------------------------------

  // done initializing
  this->mbInitialized = true;

  return true;
}


/**
<pre>
 * Call this if the TXMT was changed, use isChanged to check first.
 * It will make the raw byte array and byte count reflect the actual data of the TXMT.
 * Creates *uncompressed* bytes if it does an update.
 *
 * returns: success / failure (if no changes were needed, returns true)
</pre>
**/
bool DBPF_TXMTtype::updateRawBytes()
{
  // sanity check
  if( this->mbInitialized == false )
  { fprintf( stderr, "ERROR: DBPF_TXMTtype.updateRawBytes, can't update, not initialized yet\n" );
    return false;
  }

  if( NULL == this->mpRawBytes )
  { fprintf( stderr, "ERROR: DBPF_TXMTtype.updateRawBytes, can't update, null byte array\n" );
    return false;
  }

  // no change needed
  if( this->mbChanged == false )
    return true;


  // changes needed
  // --------------

  // new byte array size
  // - - - - - - - - - -

  // use this formula if raw byte are currently uncompressed
  unsigned int newSize = (unsigned int)((int)(this->muRawBytesCount) + this->miChangeInRawBytesCount);

  // use this formula if raw bytes are currently compressed,
  // need to get the uncompressed size from them
  unsigned int fooCmprSize, barUncSize;
  bool bCompressed = this->isCompressed( fooCmprSize, barUncSize );
  if( bCompressed )
    newSize = (unsigned int)((int)(barUncSize) + this->miChangeInRawBytesCount);


  // allocate new byte array
  // - - - - - - - - - - - -

  unsigned char * newBytes = new unsigned char[ newSize ];
  if( NULL == newBytes )
  { fprintf( stderr, "ERROR: DBPF_TXMTtype.updateRawBytes, failed to allocate memory\n" );
    return false;
  }

  // =====================================================

  // update bytes
  // ------------

  unsigned char * ptrWrite = newBytes;

  // RCOL header
  this->mRCOL.writeToByteStream( ptrWrite ); // this will advance the write pointer

  // block name
  writeByteStream_str( ptrWrite, this->mstrBlockName );

  // block ID
  writeByteStream_uint( ptrWrite, this->muBlockID );

  // block version
  writeByteStream_uint( ptrWrite, this->muBlockVersion );

  // name, in cSGResource
  writeByteStream_cSGResource( ptrWrite, this->mstrName );

  // material description
  writeByteStream_str( ptrWrite, this->mstrDesc );

  // material type
  writeByteStream_str( ptrWrite, this->mstrMaterialType );

  // properties
  // ----------

  // property count
  unsigned int propCount = this->mpProperties->getPropertyCount();
  writeByteStream_uint( ptrWrite, propCount );

  // properties pairs
  string key, val;
  for( unsigned int i = 0; i < propCount; ++i )
  {
    this->mpProperties->getPairAt( i, key, val );
    // property name
    writeByteStream_str( ptrWrite, key );
    // property value
    writeByteStream_str( ptrWrite, val );
  }

  // texture names
  if( this->muBlockVersion > 8 )
  {
    // texture name count
    writeByteStream_uint( ptrWrite, this->muTextureNameCount );

    for( unsigned int i = 0; i < this->muTextureNameCount; ++i )
      writeByteStream_str( ptrWrite, this->mTextureNames[i] );
  }

  // =================================================================

  // did we write the correct amount of bytes?

  unsigned int sizeWritten = (unsigned int)(ptrWrite - newBytes);
  if( sizeWritten != newSize )
  { fprintf( stderr, "ERROR: DBPF_TXMTtype.updateRawBytes, something went wrong, bytes written %u, expected new size %u\n",
           sizeWritten, newSize );
    delete [] newBytes;
    return false;
  }


  // done updating
  // -------------

  // byte count is the new size
  this->muRawBytesCount = newSize;

  // byte array is new byte array, delete old bytes
  if( NULL != this->mpRawBytes )
    delete [] this->mpRawBytes;
  this->mpRawBytes = newBytes;
  newBytes = NULL;

  // set changed flag to false
  this->mbChanged = false;

  // report success!
  return true;

} // DBPF_TXMTtype.updateRawBytes
