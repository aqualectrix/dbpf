/**
<pre>
 * file: DBPFcompress.cpp
 * author: CatOfEvilGenius
 *
 * compress / uncompress for Sims2 *.package file resources
 * based on Delphy's pseudocode, which was based on dmchess's perl script
 * http://www.modthesims2.com/wiki.php?title=DBPF_Compression
 *
 * QFS Compression
</pre>
**/

#include <cstdlib>
#include <cstdio>
#include <vector>
#include "DBPF_byteStreamFunctions.h"
using namespace std;


// ================================================================================

#define DBPF_COMPRESSION_QFS 0xFB10

/**
<pre>
 * intput:  data - DBPF compressed data, should have a 9 byte header
 * output:  compressedSize - size in bytes of compressed data
 *          uncompressedsize - size in bytes of uncompressed data
 * returns: true - compression id is QFS
 *          false - compression id is something else (not valid DBPF data)
 *
 * purpose: given DBPF compressed data, decode the 9 byte header to find the compressed / uncompressed size
</pre>
**/
bool dbpfGetCompressedHeader( const unsigned char * data,
                              unsigned int & compressedSize,
                              unsigned int & compressionID,
                              unsigned int & uncompressedSize )
{
  bytes2uint( data, 4, compressedSize );
  bytes2uint( data+4, 2, compressionID );
  bytes2uintBigEndian( data+6, 3, uncompressedSize );

#ifdef _DEBUG
/*
  printf( "\n" );
  for( int i = 0; i < 9; ++i )
    printf( "%2x ", *(data+i) );
  printf( "\n" );
  printf( "cmpr size: %10x: %10u bytes, %10u kb, %10u Mb\n",
    compressedSize, compressedSize, compressedSize/1024, (compressedSize/1024)/1024 );
  printf( "uncm size: %10x: %10u bytes, %10u kb, %10u Mb\n",
    uncompressedSize, uncompressedSize, uncompressedSize/1024, (uncompressedSize/1024)/1024 );
  printf( "cmpr ID:   %10x\n", compressionID );
  printf( "QFC ID:    %10x\n", DBPF_COMPRESSION_QFS );
  printf( "\n" );
*/
#endif

  if( compressionID != DBPF_COMPRESSION_QFS )
    return false;
  return true;
}


typedef unsigned char byte;
extern byte * compress( const byte* src, const byte* srcend, byte* dst, byte* dstend, bool pad );
extern bool decompress( const byte* src, int compressed_size, byte* dst, int uncompressed_size, bool truncate );


/**
<pre>
 * input:   data, dataByteCount - uncompressed data and size
 * output:  dataCmpr, dataCmprByteCount - compressed data and compressed size,
 *                         dataCmpr should be NULL when passed in, will be allocated here
 * returns: success / failure
 *
 * purpose: compress data compressed with QFC compression
 *          calls benrq compress routine
</pre>
**/
bool dbpfCompress( const unsigned char * data, const unsigned int dataByteCount,  // IN
                   unsigned char * & dataCmpr, unsigned int & dataCmprByteCount ) // OUT
{
  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: dbpfCompress, null data\n" );
    return false;
  }

  // allocate memory for compressed data (same size array as uncompressed data, though hopefully it won't all be used)

  dataCmpr = new unsigned char[ dataByteCount ];
  if( NULL == dataCmpr )
  { fprintf( stderr, "ERROR: dbpfCompress, failed to allocate memory\n" );
    return false;
  }

  // try to compress

  const bool bPad = false;
  unsigned char * dataCmprEnd = compress( data, data + dataByteCount,
                                          dataCmpr, dataCmpr + dataByteCount, bPad );

  // was compression unsuccessful?
  // if so, nuke attempted compressed data

  if( NULL == dataCmprEnd )
  { fprintf( stderr, "WARNING: dbpfCompress, failed to compress, overran output buffer\n" );
    delete [] dataCmpr;
    dataCmpr = NULL;
    dataCmprByteCount = 0;
    return false;
  }

  // compression worked,
  // what's the size of the compressed data?

  dataCmprByteCount = (unsigned int)(dataCmprEnd - dataCmpr);

  // if compressed data is same size as original, little point in compressing it,
  // this should almost never happen

  if( !(dataCmprByteCount < dataByteCount) )
  {
    delete [] dataCmpr;
    dataCmpr = NULL;
    dataCmprByteCount = 0;
    fprintf( stderr, "WARNING: dbpfCompress, failed to compress, compressed data same size as original\n" );
    return false;
  }

  // remake the compressed data array without extra unused space at the end

  unsigned char * tmp = dataCmpr;
  dataCmpr = new unsigned char[ dataCmprByteCount ];
  if( NULL == dataCmpr )
  {
    dataCmpr = tmp;
    fprintf( stderr, "WARNING: dbpfCompress, compressed data is stored in an array with extra unused space at the end, failed to allocate smaller array\n" );
  }
  else
  {
    memcpy( dataCmpr, tmp, dataCmprByteCount );
    delete [] tmp;
    tmp = NULL;
  }

  // all done!

  return true;
} // compress


/**
<pre>
 * input:   data, dataByteCount - compressed data with 9 byte header containing size, size of data
 * output:  dataUnc, dataUncByteCount - uncompressed data and size,
 *                         dataUnc should be NULL when passed in, will be allocated here
 * returns: success / failure
 *
 * purpose: decompress data compressed with QFC compression
 *          calls benrq's decompress routine
</pre>
**/
bool dbpfDecompress( const unsigned char * data, const unsigned int dataByteCount, // IN
                     unsigned char * & dataUnc, unsigned int & dataUncByteCount )  // OUT
{
  // header - first 9 bytes
  // ----------------------

  unsigned int hdrCompressedSize, hdrCompressionID, hdrUncompressedSize;
  if( false == dbpfGetCompressedHeader( data, hdrCompressedSize, hdrCompressionID, hdrUncompressedSize ) )
  { fprintf( stderr, "ERRROR: dbpfDecompress, compression ID is not QFC\n" );
    return false;
  }

  if( dataUncByteCount != hdrUncompressedSize )
  { fprintf( stderr, "ERROR: dbpfDecompress, expected uncompressed size does not match size in 9 byte header\n" );
    return false;
  }

  // allocate memory for uncompressed data
  dataUnc = new unsigned char[ dataUncByteCount ];
  if( NULL == dataUnc )
  { fprintf( stderr, "ERRROR: dbpfDecompress, failed to allocate memory\n" );
    return false;
  }

  bool bSuccess = decompress( data, dataByteCount, dataUnc, dataUncByteCount, false );

  return bSuccess;
} // uncompress
