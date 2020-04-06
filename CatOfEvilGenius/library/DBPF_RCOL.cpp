/**
 * file: DBPF_RCOL.cpp
 * author: CatOfEvilGenius
**/

#include <cstdlib>
#include <cstdio>
#include <DBPF_RCOL.h>
#include <DBPF_byteStreamFunctions.h>
#include <DBPF_types.h> // type to string functions


DBPF_RCOLtype::DBPF_RCOLtype()
: mbInitialized( false ),
  mbLinksHaveResourceID( false ),
  muLinkCount( 0 ),
  muIndexItemCount( 0 )
{}


DBPF_RCOLtype::~DBPF_RCOLtype()
{
  clear();
}


void DBPF_RCOLtype::clear()
{
  this->mbInitialized = false;
  this->mbChanged = false;

  this->muRawBytesCount = 0;

  this->mbLinksHaveResourceID = false;
  this->muLinkCount = 0;
  this->muIndexItemCount = 0;

  if( false == this->mLinks.empty() )
    this->mLinks.clear();
  if( false == this->mIndex.empty() )
    this->mIndex.clear();
}


#ifdef _DEBUG
void DBPF_RCOLtype::dump( FILE * f ) const
{
  fprintf( f, "mbInitialized: " );
  if( this->mbInitialized ) fprintf( f, "TRUE\n" );
  else                      fprintf( f, "false\n" );
  fprintf( f, "mbChanged: " );
  if( this->mbChanged ) fprintf( f, "TRUE\n" );
  else                  fprintf( f, "false\n" );
  fprintf( f, "raw byte count: %u\n", this->muRawBytesCount );
  fprintf( f, "mbLinksHaveResourceID: " );
  if( this->mbLinksHaveResourceID ) fprintf( f, "TRUE\n" );
  else                              fprintf( f, "false\n" );
  fprintf( f, "muLinkCount: %u\n", this->muLinkCount );
  char str[256];
  DBPF_TGIRtype link;
  for( unsigned int i = 0; i < this->muLinkCount; ++i )
  {
    link = (this->mLinks)[i];
    fprintf( f, "  %u: group 0x%x\n", i, link.muGroupID );
    fprintf( f, "  %u: inst  0x%x\n", i, link.muInstanceID );
    if( this->mbLinksHaveResourceID )
      fprintf( f, "  %u: resrc 0x%x\n", i, link.muResourceID );
    dbpfResourceTypeToString( link.muTypeID, str );
    fprintf( f, "  %u: type  %s\n", i, str );
  }
  fprintf( f, "muIndexItemCount: %u\n", this->muIndexItemCount );
  for( unsigned int i = 0; i < this->muIndexItemCount; ++i )
  {
    dbpfResourceTypeToString( (this->mIndex)[i], str );
    fprintf( f, "  %u: %s\n", i, str );
  }
} // dump
#endif


bool DBPF_RCOLtype::initFromByteStream( unsigned char * data,    // IN - data, rcol header at beginning
                                        unsigned char * & restOfData ) // OUT - points to first byte after header
{
  // set uninitialized, remove any old data
  this->clear();

  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_RCOLtype.initFromByteStream, null data\n" );
    return false;
  }

  // is first DWORD 0x0100FFFF? (not uint, but raw bytes!)
  // if so, we have resource IDs in links,
  // if not, no resource IDs in links

  unsigned char * bytes = data;

  this->mbLinksHaveResourceID = false;
  if( data[0] == 1  &&  data[1] == 0  &&  data[2] == 0xFF  &&  data[3] == 0xFF )
  {
    this->mbLinksHaveResourceID = true;
    bytes += 4;
  }

  // link count

  bytes2uint( bytes, 4, this->muLinkCount );
  bytes += 4;

#ifdef _DEBUG
//  printf( "link count %u\n", this->muLinkCount );
#endif


  // links

  DBPF_TGIRtype link;

  for( unsigned int i = 0; i < this->muLinkCount; ++i )
  {
    // group
    bytes2uint( bytes, 4, link.muGroupID );
    bytes += 4;

    // instance
    bytes2uint( bytes, 4, link.muInstanceID );
    bytes += 4;

    // resource (not always there)
    if( this->mbLinksHaveResourceID )
    {
      bytes2uint( bytes, 4, link.muResourceID );
      bytes += 4;
    }

    // type
    bytes2uint( bytes, 4, link.muTypeID );
    bytes += 4;

#ifdef _DEBUG
/*    printf( "link\n" );
    printf( "  0x%10x\n  0x%10x\n  0x%10x\n  0x%10x\n",
      link.muGroupID, link.muInstanceID, link.muResourceID, link.muTypeID );
    printf( "\n" );  */
#endif

    // add to list of links
    this->mLinks.push_back( link );
  }


  // index item count

  bytes2uint( bytes, 4, this->muIndexItemCount );
  bytes += 4;

#ifdef _DEBUG
//  printf( "index item count %u\n", this->muIndexItemCount );
#endif


  // index(es)

  unsigned int rcolID = 0;
  for( unsigned int i = 0; i < this->muIndexItemCount; ++i )
  {
    bytes2uint( bytes, 4, rcolID );
    bytes += 4;

#ifdef _DEBUG
/*    printf( "rcol ID: " );
    dbpfPrintResourceType( rcolID );
    printf( "\n" );  */
#endif

    this->mIndex.push_back( rcolID );
  }
  

  // finished initializing
  this->mbInitialized = true;

  // rest of the data, after the header, starts here
  restOfData = bytes;

  // how big is the RCOL in bytes?
  this->muRawBytesCount = (unsigned int)(restOfData - data);

  return true;
} // initFromByteStream


/**
<pre>
 * in/out: bytes - ptr to byte array, will get advanced
 * returns: success / failure
 *
 * purpose: write out the RCOL are raw bytes to the given byte array,
 *          advances the bytes pointer
 *
 * returns false if RCOL has not been initialized
</pre>
**/
bool DBPF_RCOLtype::writeToByteStream( unsigned char * & bytes )
{
  // sanity check

  if( false == this->mbInitialized )
    return false;

  if( NULL == bytes )
  { fprintf( stderr, "ERROR: DBPF_RCOLtype.writeToByteStream, null bytes given\n" );
    return false;
  }


  // write RCOL's raw bytes
  // ----------------------

  // version mark
  if( this->mbLinksHaveResourceID )
  {
    // write raw bytes 0x0100FFFF
    bytes[0] = 1;
    bytes[1] = 0;
    bytes[2] = 0xFF;
    bytes[3] = 0xFF;

    bytes += 4;
  }

  // link count
  writeByteStream_uint( bytes, this->muLinkCount );

  for( unsigned int i = 0; i < this->muLinkCount; ++i )
  {
    // group
    writeByteStream_uint( bytes, this->mLinks[i].muGroupID );

    // instance
    writeByteStream_uint( bytes, this->mLinks[i].muInstanceID );

    // resource
    if( this->mbLinksHaveResourceID )
      writeByteStream_uint( bytes, this->mLinks[i].muResourceID );

    // type
    writeByteStream_uint( bytes, this->mLinks[i].muTypeID );
  }

  // item count
  writeByteStream_uint( bytes, this->muIndexItemCount );

  // index(es)
  for( unsigned int i = 0; i < this->muIndexItemCount; ++i )
    writeByteStream_uint( bytes, this->mIndex[i] );


  return true;
}