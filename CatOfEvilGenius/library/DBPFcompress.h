/**
<pre>
 * file: DBPFcompress.h
 * author: CatOfEvilGenius
 *
 * compress / uncompress for Sims2 *.package file resources (QFC compression)
 * based on Delphy's pseudocode, which was based on dmchess's perl script
 * http://www.modthesims2.com/wiki.php?title=DBPF_Compression
</pre>
**/

// QFC compression
bool dbpfCompress( const unsigned char * data, const unsigned int dataByteCount,
                   unsigned char * & dataCmpr, unsigned int & dataCmprByteCount );

// QFC decompression
bool dbpfDecompress( const unsigned char * data, const unsigned int dataByteCount,
                     unsigned char * & dataUnc, unsigned int & dataUncByteCount );

// given raw byte data, check for a QFC 9 byte header
bool dbpfGetCompressedHeader( const unsigned char * data,
                              unsigned int & compressedSize,
                              unsigned int & compressionID,
                              unsigned int & uncompressedSize );

