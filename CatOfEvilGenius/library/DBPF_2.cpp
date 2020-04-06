// DBPF_2.cpp
// function declarations in DBPF.h

#include <cstdlib>
#include <cstdio>

#include <DBPF.h>
#include <DBPFcompress.h>
#include <DBPF_types.h>
#include <DBPF_3IDR.h>
#include <DBPF_GZPS.h>
#include <DBPF_XHTN.h>
#include <DBPF_TXMT.h>
#include <DBPF_TXTR.h>
#include <DBPF_STR.h>


/**
<pre>
 * input:   strIn - original filename
 *          strAppend - some text to append to the original filename, before .package
 * output:  strOut - will be orignal filename (without .package), then strAppend, then .package
 * returns: (none)
</pre>
**/
void makeOutputFileName( string & strOut, const char * strIn, const char * strAppend )
{
  strOut.assign( strIn, strlen( strIn ) - 8 );
  strOut = strOut + string( strAppend );
  strOut = strOut + string(".package");
}


// given a filepath in str, gives you just the filename in strOut
bool justTheFilename( const char * str, string strOut )
{
  if( NULL == str )
    return false;

  size_t len = strlen( str );
  if( len < 2 )
    return false;

  const char * ptr = NULL;
  for( size_t i = len - 2; i != 0; --i )
  {
    ptr = str + i;
    if( '/' == *ptr
     || '\\' == *ptr )
    {
      ptr = ptr+1;
    }
  }

  char foo[512];
  strcpy( foo, ptr );
  strOut.assign( foo );

  return true;
}


/**
 * reads a package file,
 * gives a list of resources in that package,
 *   of type DBPF_resourceType (and appropriate subtype),
 * resources whose types are give in typesToInit
 *   will be decompressed and initialized,
 * all others will be compressed and of type DBPF_undecodedType
**/
bool readPackage( const char * filename,                         // IN
                  DBPFtype & package,                            // IN/OUT
                  vector< unsigned int > & typesToInit,          // IN
                  vector< DBPF_resourceType * > & resources )    // OUT
{
  // open DBPF package
  // ------------------------
  // read DBPF header, index table, and directory of compressed stuff, if it exists

  size_t fileSizeIn = 0;
  size_t fileSizeOut = 0;

  if( false == package.read( filename, fileSizeIn ) )
    return false;

  fileSizeOut = fileSizeIn; // if file does not change, this will be true


  // resources loop
  // ------------------------
  // get resources, put them in a vector,
  // uncompress and init the ones we're interested in,
  // compress / don't init the others

  const unsigned int itemCount = package.getItemCount();
  DBPFindexType entry;

  unsigned char * bytes = NULL;
  unsigned int byteCount = 0;
  unsigned char * cmprBytes = NULL;
  unsigned int cmprByteCount = 0;
  unsigned int decmpByteCount = 0;
  bool bInitThis = false;
  DBPF_resourceType * pResource = NULL;

  for( unsigned int k = 0; k < itemCount; ++k )
  {
    // package index entry

    package.getIndexEntry( k, entry );

    // is it a DIR?  don't need it, package already has it, and its decoded

    if( DBPF_DIR == entry.muTypeID )
      continue;

    // get data for a resource

    if( false == package.getData( entry, bytes, byteCount ) )
      return false;

    // is this resource of a type we want?

    bInitThis = false;

    for( size_t tt = 0; tt < typesToInit.size(); ++tt )
    {
      if( entry.muTypeID == typesToInit[tt] )
      {
        bInitThis = true;
        tt = typesToInit.size();
      }
    }

    // if resource is a type we want, uncompress it,
    // otherwise, leave it compressed, we won't be decoding it anyway

    bool bCompressed = false; // this may or may not be true, checked later

    if( true == bInitThis )
    {
     if( package.isCompressed( entry, decmpByteCount ) )
     {
      bCompressed = true;

      cmprBytes = bytes;
      cmprByteCount = byteCount;
      bytes = NULL;
      byteCount = decmpByteCount;

      if( false == dbpfDecompress( cmprBytes, cmprByteCount, bytes, byteCount ) )
        return false;

      bCompressed = false;

      delete [] cmprBytes;
      cmprBytes = NULL;
      cmprByteCount = 0;
     }
    }

    // if resource is not a type we want,
    // and if NOT compressed, then attempt to compress it,
    // we won't need to decode it or use it for anything else
    else
    {
      bCompressed = true; // assume it is compressed, then check

      if( false == package.isCompressed( entry, decmpByteCount ) )
      {
        cmprBytes = NULL;
        cmprByteCount = 0;

        // attempt compression
        bool bCompressed = dbpfCompress( bytes, byteCount, cmprBytes, cmprByteCount );

        // if it worked, move cmprBytes to bytes, delete old uncompressed data
        if( bCompressed )
        {
          delete [] bytes;
          bytes = cmprBytes;
          byteCount = cmprByteCount;
          cmprByteCount = 0;
        }
      }
    }


    // if resource is a type we want, make pointer of appropriate type, so it can be decoded later,
    // otherwise, just store it undecoded type resource

    if( true == bInitThis )
    {
      if( DBPF_TXMT == entry.muTypeID )
      {
        DBPF_TXMTtype * pFooRes = new DBPF_TXMTtype();
        pResource = pFooRes;
      }
      else if( DBPF_GZPS == entry.muTypeID )
      {
        DBPF_GZPStype * pFooRes = new DBPF_GZPStype();
        pResource = pFooRes;
      }
      else if( DBPF_3IDR == entry.muTypeID )
      {
        DBPF_3IDRtype * pFooRes = new DBPF_3IDRtype();
        pResource = pFooRes;
      }
      else if( DBPF_TXTR == entry.muTypeID )
      {
        DBPF_TXTRtype * pFooRes = new DBPF_TXTRtype();
        pResource = pFooRes;
      }
      else if( DBPF_STR == entry.muTypeID )
      {
        DBPF_STRtype * pFooRes = new DBPF_STRtype();
        pResource = pFooRes;
      }
      else if( DBPF_XHTN == entry.muTypeID )
      {
        DBPF_XHTNtype * pFooRes = new DBPF_XHTNtype();
        pResource = pFooRes;
      }
      else
      {
        fprintf( stderr, "ERROR: readPackage, need to construct resource to init, but type no in if/else chain, %x\n", entry.muTypeID );
        return false;
      }
    }
    else
    {
      DBPF_undecodedType * pUndec = new DBPF_undecodedType();
      pResource = pUndec;
    }

    // initialize resource from byte stream

    if( false == pResource->initFromByteStream( entry, bytes, byteCount ) )
      return false;

#ifdef _DEBUG
    pResource->dump( stdout );
#endif

    // add resource to resource vector

    resources.push_back( pResource );
    pResource = NULL;


    // do NOT delete bytes, they were given to a resource and the resource will delete them
    bytes = NULL;
    byteCount = 0;

  } // resources loop

  return true;
} // readPackage


/**
 * Write out package file.
 * First, compresses all resources.
**/
bool writeCompressedPackage( const char * filename,              // IN
                   DBPFtype & package,                           // IN
                   vector< DBPF_resourceType * > & resources )   // IN
{
  unsigned int cmprByteCount = 0, uncByteCount = 0;
  for( size_t k = 0; k < resources.size(); ++k )
  {
    resources[k]->updateRawBytes();
    if( false == resources[k]->isCompressed( cmprByteCount, uncByteCount ) )
      resources[k]->compressRawBytes();
  }

  size_t fileSizeOut = 0;
  return( package.write( filename, resources, fileSizeOut ) );
}

