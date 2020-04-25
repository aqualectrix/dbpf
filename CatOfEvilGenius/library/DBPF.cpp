/**
 * file: DBPF.cpp
 * author: CatOfEvilGenius
 *
 * class DBPFtype - package files, header, index table, various resources
 * class DBPF_DIRtype - DIR resource, directory of compressed resources
 * class DBPFindexType - used by the index table of DBPFtype and also used by the DBPF_DIRtype
**/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>

#include "DBPF.h"
#include "DBPF_types.h"
#include "DBPF_resource.h"


// -------------------------------------------------------------------------


DBPFindexType::DBPFindexType()
: muTypeID( 0 ),
  muGroupID( 0 ),
  muInstanceID( 0 ),
  muInstanceID2( 0 ),
  muLocation( 0 ),
  muSize( 0 )
{}


void DBPFindexType::clear()
{
  muTypeID = 0;
  muGroupID = 0;
  muInstanceID = 0;
  muInstanceID2 = 0;
  muLocation = 0;
  muSize = 0;
}


/**
<pre>
 * input:   f - file pointer, file should already be opened and at correct location to read entry
 *          bRead2ndInstanceID - for index table version 7.0, this should be false,
 *                               for index table version 7.1, this should be true
 *          bReadLocation - if reading index table entry, this should be true
 *                          if reading DIR table entry, this should be false
 * output:  none
 * returns: none
 *
 * purpose: read a DBPF index entry
 *          for index version 7.1, two instance numbers are read,
 *          can also be used to read DIR entries,
 *          for DIR entries, there is no location info,
 *          so bReadLocation should be false
</pre>
**/
void DBPFindexType::read( FILE * f , bool bRead2ndInstanceID, bool bReadLocation )
{
  fread( &muTypeID, sizeof( unsigned int ), 1, f );
  fread( &muGroupID, sizeof( unsigned int ), 1, f );
  fread( &muInstanceID, sizeof( unsigned int ), 1, f );
  if( bRead2ndInstanceID )
    fread( &muInstanceID2, sizeof( unsigned int ), 1, f );
  if( bReadLocation )
    fread( &muLocation, sizeof( unsigned int ), 1, f );
  fread( &muSize, sizeof( unsigned int ), 1, f );
}


#ifdef _DEBUG
// prints type, ie. TXMT or TXTR, no endline at end
void DBPFindexType::dumpTypeOnly( FILE * f ) const
{
  char str[256];
  dbpfResourceTypeToString( this->muTypeID, str );
  fprintf( f, "%s", str );
}
#endif


#ifdef _DEBUG
void DBPFindexType::dumpTableHeader( FILE * f )
{
  fprintf( f, "type       %10s %10s %10s %10s %10s\n", "group", "instance", "instance2", "location", "size" );
}
#endif


#ifdef _DEBUG
void DBPFindexType::dump( FILE * f ) const
{
  char str[30];
  dbpfResourceTypeToString( this->muTypeID, str );
  fprintf( f, "%8s  ", str );
  fprintf( f, "%10x %10x %10x %10x %10x",
      this->muGroupID, this->muInstanceID, this->muInstanceID2, this->muLocation, this->muSize );
  fprintf( f, "\n" );
}
#endif


// ---------------------------------------------------------------------


DBPF_DIRtype::~DBPF_DIRtype()
{
  // clean up memory, compressed index table vector
  if( false == this->mIndexTableCompressed.empty() )
    this->mIndexTableCompressed.clear();
}


/**
<pre>
 * input:   f - file pointer, file should already be open, DBPFtype has this file pointer
 *          myIndexEntry - the index entry for the DIR resource, from DBPFtype index table
 *          bRead2ndInstance - index table version 7.0, false
 *                             index table version 7.1, true
 * output:  none
 * returns: success / failure
 *
 * purpose: read a DIR resource
</pre>
**/
bool DBPF_DIRtype::read( FILE * f, DBPFindexType &myIndexEntry, bool bRead2ndInstance )
{
  // sanity check
  if( NULL == f )
  { fprintf( stderr, "ERROR: DBPF_DIRtype.read, NULL file\n" );
    return false;
  }

  // copy my index entry
  this->mMyIndexEntry = myIndexEntry;

  // how many entries are in the table of compressed things?
  unsigned int entrySize = bRead2ndInstance ? 5 * sizeof( unsigned int ) : 4 * sizeof( unsigned int );
  unsigned int entryCount = this->mMyIndexEntry.muSize / entrySize;
#ifdef _DEBUG
  printf( "\n" );
  printf( "compressed table entry count: %u\n", entryCount );
#endif

  // go to location of DIR in file
  fseek( f, this->mMyIndexEntry.muLocation, SEEK_SET );

  // read index table of compressed stuff (no location data)
  if( this->mIndexTableCompressed.empty() == false )
    this->mIndexTableCompressed.clear();

  DBPFindexType entry;
  const bool bReadLocation = false;
  for( unsigned int i = 0; i < entryCount; ++i )
  {
    entry.read( f, bRead2ndInstance, bReadLocation );
    this->mIndexTableCompressed.push_back( entry );
#ifdef _DEBUG
    entry.dump( stdout );
#endif
  }

  return true;
}


bool DBPF_DIRtype::write( FILE * f, bool bWrite2ndInstance )
{
  return true;
}


#ifdef _DEBUG
void DBPF_DIRtype::dump()
{
}
#endif

unsigned int DBPF_DIRtype::getCompressedItemCount() const
{
  return( (unsigned int)( this->mIndexTableCompressed.size() ) );
}


// does this DIR contain an entry with the type, group, and instance of the given entry?
bool DBPF_DIRtype::isCompressed( const DBPFindexType entry, unsigned int & decmpSize ) const
{
  unsigned int count = this->getCompressedItemCount();

  for( unsigned int i = 0; i < count; ++i )
  {
    // type
    if( entry.muTypeID != (this->mIndexTableCompressed)[i].muTypeID )
      continue;

    // group
    if( entry.muGroupID != (this->mIndexTableCompressed)[i].muGroupID )
      continue;

    // instance
    if( entry.muInstanceID != (this->mIndexTableCompressed)[i].muInstanceID )
      continue;

/* #ifdef _DEBUG
    printf( "this resource is COMPRESSED\n" );
    entry.dump();
    (this->mIndexTableCompressed)[i].dump();
    printf( "\n" );
#endif */

    // type, group, and instance match,
    // get the decompressed size and return true
    decmpSize = (this->mIndexTableCompressed)[i].muSize;
    return true;
  }

/* #ifdef _DEBUG
  printf( "this resource is not compressed\n" );
  entry.dump();
  printf( "\n" );
#endif  */

  return false;
}



// ---------------------------------------------------------------------


DBPFtype::DBPFtype()
: mFile( NULL ),
  muVersionMajor( 0 ),
  muVersionMinor( 0 ),
  muIndexVersionMajor( 0 ),
  muIndexVersionMinor( 0 ),
  muIndexEntryCount( 0 ),
  muIndexOffset( 0 ),
  muIndexSizeInBytes( 0 ),
  muHoleEntryCount( 0 ),
  muHoleOffset( 0 ),
  muHoleSize( 0 ),
  mbDIRexists( false )
{
  strcpy( this->mstrFileName, "" );
}


DBPFtype::~DBPFtype()
{
  // close file, if still open
  if( this->mFile != NULL )
  { fclose( this->mFile );
    this->mFile = NULL;
  }

  // clean up memory, index table vector
  if( false == this->mIndexTable.empty() )
    this->mIndexTable.clear();
}


void DBPFtype::closeFile()
{
  if( this->mFile != NULL )
  {
    fclose( mFile );
    this->mFile = NULL;
  }
}


/**
<pre>
 * mstrFileName must have already been set by read
</pre>
**/
bool DBPFtype::openFile()
{
  if( mFile != NULL )
  { fprintf( stderr, "ERROR: DBPFtype.openFile, mFile is not NULL, maybe file was already opened?  read opens the file\n" );
    return false;
  }

  mFile = fopen( this->mstrFileName, "rb" );
  if( mFile == NULL )
  { fprintf( stderr, "ERROR: DBPFtype.openFile, failed to open %s\n", this->mstrFileName );
    return false;
  }

  return true;
}


/**
<pre>
 * This does NOT read the entire package file.
 * Rather it reads enough to tell you what's in the file and whether it's compressed.
 * Here's what read() does:
 *
 * open the file
 * read the header
 * read the index table
 * read the directory of compressed stuff, if one exists
 * close the file
</pre>
**/
bool DBPFtype::read( const char * fileName,  // IN
                     size_t & fileSize )     // OUT
{
  fileSize = 0;

  // sanity check

  if( NULL == fileName )
  { fprintf( stderr, "ERROR: NULL file name\n" );
    return false;
  }

  // file name

  if( strlen( fileName ) > 1023 )
  { fprintf( stderr, "ERROR: filename too long\n" );
    return false;
  }

  strcpy( this->mstrFileName, fileName );

  // open the file
  if( false == this->openFile() )
    return false;

  // read header

  if( false == this->readHeader() )
    return false;

  // read index table

  if( false == this->readIndexTable() )
    return false;

  // read directory of compressed stuff, if one exists

  if( false == this->readDIR() )
    return false;

  // file size

  fseek( this->mFile, 0, SEEK_END );
  fileSize = ftell( this->mFile );

  // close the file
  this->closeFile();

  return true;
}


/**
 * open the file and read the header
**/
bool DBPFtype::readHeader()
{
  // read the header
  // ===============

  // is it a DBPF file?

  char magic[5];  magic[4] = '\0';
  fread( magic, sizeof( char ), 4, this->mFile );
  if( 0 != strcmp( magic, "DBPF" ) )
  { fprintf( stderr, "ERROR: not a DBPF file, magic is %s\n", magic );
    this->closeFile();
    return false;
  }

  // version

  unsigned int foo;
  fread( &foo, sizeof( unsigned int ), 1, this->mFile );
  // printf( "major %u\n", foo );
  this->muVersionMajor = foo;
  fread( &foo, sizeof( unsigned int ), 1, this->mFile );
  // printf( "minor %u\n", foo - 1 );
  this->muVersionMinor = foo - 1;

  if( this->muVersionMajor != 1
   || this->muVersionMinor < 0
   || this->muVersionMinor > 1 )
  {
    fprintf( stderr, "ERROR: unknown version number %u.%u\n", this->muVersionMajor, this->muVersionMinor );
    return false;
  }

  // unknown stuff (that we may want to write out as is)
  fread( this->muUnknowns, sizeof( unsigned int ), 3, this->mFile );

  // date created and modified
  fread( this->muDates, sizeof( unsigned int ), 2, this->mFile );

  // index info
  // ----------

  // index major version, Sims2 always 7
  // is it a Sims2 file?
  fread( &(this->muIndexVersionMajor), sizeof( unsigned int ), 1, this->mFile );
  if( this->muIndexVersionMajor != 7 )
  { fprintf( stderr, "ERROR: not a Sims2 file (index major version is not 7, is %u)\n", this->muIndexVersionMajor );
    return false;
  }

  // index entry count
  fread( &(this->muIndexEntryCount), sizeof( unsigned int ), 1, this->mFile );
  // printf( "index entry count: %u\n", this->muIndexEntryCount );

  // index offset
  fread( &(this->muIndexOffset), sizeof( unsigned int ), 1, this->mFile );

  // index size in bytes
  fread( &(this->muIndexSizeInBytes), sizeof( unsigned int ), 1, this->mFile );

  // hole info
  fread( &(this->muHoleEntryCount), sizeof( unsigned int ), 1, this->mFile );
  fread( &(this->muHoleOffset),     sizeof( unsigned int ), 1, this->mFile );
  fread( &(this->muHoleSize),       sizeof( unsigned int ), 1, this->mFile );
  // printf( "hole entry count: %u\n", this->muHoleEntryCount );
  // printf( "hole offset: %u\n", this->muHoleOffset );
  // printf( "hole size: %u\n", this->muHoleSize );

  // index minor version
  fread( &(this->muIndexVersionMinor), sizeof( unsigned int ), 1, this->mFile );
  this->muIndexVersionMinor -= 1;

  // read last unknown
  fread( &(this->muUnknownLast), sizeof( unsigned int ), 1, this->mFile );

  return true;

} // DBPFtype::readHeader


/**
 * read index table,
 * be sure header has already been read before calling this
**/
bool DBPFtype::readIndexTable()
{
  // sanity check
  if( this->mFile == NULL )
  { fprintf( stderr, "ERROR: DBPFtype.readIndexTable, mFile is NULL, has readHeader not been called?\n" );
    return false;
  }

  // read all entries in index table

  if( false == this->mIndexTable.empty() )
    this->mIndexTable.clear();

  DBPFindexType indexEntry;
  bool bRead2ndInstanceID = ( this->muIndexVersionMinor == 1 ) ? true : false;
  bool bReadLocation = true;

  // printf( "index offset: %u\n", this->muIndexOffset );
  int offset = (int)(this->muIndexOffset );
  if( offset < 0 )
  { fprintf( stderr, "ERROR: DBPFtype.readIndexTable, negative offset %i\n", offset );
    return false;
  }
  fseek( this->mFile, offset, SEEK_SET );

#ifdef _DEBUG
  printf( "\n    " );
  DBPFindexType::dumpTableHeader( stdout );
#endif
  for( unsigned int i = 0; i < this->muIndexEntryCount; ++i )
  {
    indexEntry.read( this->mFile, bRead2ndInstanceID, bReadLocation );
    this->mIndexTable.push_back( indexEntry );
#ifdef _DEBUG
    printf( "%3u ", i );
    (this->mIndexTable)[i].dump( stdout );
#endif
  }

  return true;
}


/**
<pre>
 * returns true if either of these happened:
 *   DIR exists and was read successfully
 *   DIR does not exist
 * returns false if DIR exists but something went wrong while reading it
 *
 * make sure readIndexTable has been called before calling this
</pre>
**/
bool DBPFtype::readDIR()
{
  bool bRead2ndInstance = false;
  if( this->muIndexVersionMinor == 1 )
    bRead2ndInstance = true;

  this->mbDIRexists = false;
  for( unsigned int i = 0; i < this->mIndexTable.size(); ++i )
  {
    if( (this->mIndexTable)[i].muTypeID == DBPF_DIR )
    {
      this->mbDIRexists = true;
      if( false == this->mDIR.read( this->mFile, (this->mIndexTable)[i], bRead2ndInstance ) )
        return false;
      break;
    }
  }

  return true;
}


/**
<pre>
 * get the index table entry for the k'th resource in the DBPF file,
 * file must have been opened, and index table read
</pre>
**/
bool DBPFtype::getIndexEntry( const unsigned int k, DBPFindexType & entry ) const
{
  if( k < this->muIndexEntryCount )
  {
    entry = (this->mIndexTable)[k];
    return true;
  }

  fprintf( stderr, "ERROR: DBPFtype.getIndexEntry, k out of bounds, is %u, must be < %u\n", k, this->muIndexEntryCount );
  return false;
}


/**
 * Does the package contain at least one resource of a given type?
**/
bool DBPFtype::contains( const unsigned int type ) const
{
  if( false == this->mIndexTable.empty() )
  {
   for( size_t i = 0; i < this->mIndexTable.size(); ++i )
   {
    if( type == (this->mIndexTable)[i].muTypeID )
      return true;
   }
  }

  return false;
}


/**
 * How many resource of a given type does the package contain?
**/
unsigned int DBPFtype::howMany( const unsigned int type ) const
{
  unsigned int count = 0;

  if( false == this->mIndexTable.empty() )
  {
   for( size_t i = 0; i < this->mIndexTable.size(); ++i )
   {
    if( type == (this->mIndexTable)[i].muTypeID )
      ++count;
   }
  }

  return count;
}


/**
<pre>
 * check whether the resource with the given index table entry is compressed or not,
 * it is assumed that "entry" is a valid index entry obtained with DBPFtype::getIndexEntry
 * returns true if compressed, and gives *decompressed* size in decmpSize
 * false if not compressed, decmpSize is meaningless in this case
</pre>
**/
bool DBPFtype::isCompressed( const DBPFindexType entry, unsigned int & decmpSize ) const
{
  // does DIR contain an entry matching the given entry?
  // if so, the data for this resource is compressed,
  // if not, it is not compressed

  if( this->mbDIRexists )
    return( this->mDIR.isCompressed( entry, decmpSize ) );

  return false; // no DIR, nothing is compressed
}


/**
<pre>
 * input:   entry - index entry from index table, such as that returned by getIndexEntry
 * output:  bytes - byte array, data for that resource, read from the file,
 *                  this gets allocated here with new, pass in an *unallocated* array,
 *                  use delete [] when done with the data
 *          byteCount - size of the byte array
 * returns: success / failure
 *
 * get the raw data (byte array) for the resource entry with the given index entry
 * opens the file, reads the data, closes the file
</pre>
**/
bool DBPFtype::getData( const DBPFindexType entry, unsigned char * & bytes, unsigned int & byteCount )
{
  // open file
  if( this->openFile() == false )
    return false;

  // allocate memory
  bytes = new unsigned char[ entry.muSize ];
  if( NULL == bytes )
  { fprintf( stderr, "ERROR: DBPFtype.getData, failed to allocate memory\n" );
    return false;
  }

  // read data
  fseek( this->mFile, entry.muLocation, SEEK_SET );
  size_t bytesRead = fread( bytes, 1, entry.muSize, this->mFile );

  // how many bytes were read?
  byteCount = (unsigned int)bytesRead;
  if( byteCount != entry.muSize )
  { fprintf( stderr, "ERROR: DBPFtype.getData, tried to read %u bytes, read %u bytes\n", entry.muSize, byteCount );
    return false;
  }

  // close file
  this->closeFile();

  return true;
}


/**
<pre>
 * Write a new package file, given an old package with its old header,
 * and a list of new and likely updated resources.
</pre>
**/
bool DBPFtype::write( const char * fileName,                      // IN
                      vector< DBPF_resourceType * > & resources,  // IN
                      size_t & fileSize )                         // OUT
{
  fileSize = 0;

  // sanity check

  if( NULL == fileName )
  { fprintf( stderr, "ERROR: DBPFtype.write, null fileName\n" );
    return false;
  }

  if( 0 == strcmp( this->mstrFileName, "" ) )  // no input file was read
  { fprintf( stderr, "ERROR: DBPFtype.write, this package object has not been initialized with read\n" );
    return false;
  }

  if( 0 == resources.size() )
  { fprintf( stderr, "ERROR: DBPFtype.write, no resources in resource list\n" );
    return false;
  }


  // open file for binary writing
  FILE * f = fopen( fileName, "wb" );
  if( NULL == f )
  { fprintf( stderr, "ERROR: DBPFtype.write, failed to open %s for writing\n", fileName );
    return false;
  }


  // write the file

  this->writeHeader( f, resources );
  this->writeResources( f, resources );
  this->writeDIR( f, resources );
  this->writeIndexTable( f, resources );

  // # of bytes written
  fseek( f, 0, SEEK_END );
  fileSize = ftell( f );

  // close file
  fclose( f );


  return true;
} // write


bool DBPFtype::writeHeader( FILE * f, vector< DBPF_resourceType * > & resources )
{
  // magic, DBPF

  char magic[5];
  strcpy( magic, "DBPF" );
  fwrite( magic, 1, 4, f );

  // major version,
  // minor version

  unsigned int foo = this->muVersionMajor;
  fwrite( &foo, sizeof( unsigned int ), 1, f );
  foo = this->muVersionMinor + 1;
  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // 3 unknowns

  fwrite( this->muUnknowns, sizeof( unsigned int ), 3, f );

  // date created,
  // date modified (should update this)

  fwrite( this->muDates, sizeof( unsigned int ), 2, f );

  // index major version

  foo = this->muIndexVersionMajor;
  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // index entry count
  //   number of resources (currently not including DIR)

  foo = 0;
  for( unsigned int i = 0; i < resources.size(); ++i )
  {
    if( resources[i] != NULL )
      ++foo;
  }

  foo += 1;  // hack, add one for DIR, not a proper resource subclass yet, and should check if we have at least on compressed resource...

  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // offset of first index entry
  // size of index table in bytes
  //   WRITE THIS IN LATER, WHEN WRITING INDEX TABLE
  //   put zeros for now

  foo = 0;
  fwrite( &foo, sizeof( unsigned int ), 1, f );
  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // hole entry count,
  // hole offset,
  // hole size
  //  put zeros for all of these

  foo = 0;
  fwrite( &foo, sizeof( unsigned int ), 1, f );
  fwrite( &foo, sizeof( unsigned int ), 1, f );
  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // index minor version

  foo = this->muIndexVersionMinor + 1;
  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // this value not used in Sims2, put zero

  foo = 0;
  fwrite( &foo, sizeof( unsigned int ), 1, f );

  // last unknown

  fwrite( &(this->muUnknownLast), sizeof(unsigned int), 1, f );

  // 24 bytes reserved by EA, probably not used, put zeros

  unsigned char zeros[24];
  for( int i = 0; i < 24; ++i )
    zeros[i] = 0;
  fwrite( zeros, sizeof(unsigned char), 24, f );

  return true;
}


bool DBPFtype::writeResources( FILE * f, vector< DBPF_resourceType * > & resources )
{
  // resource loop

  DBPF_resourceType * pResource = NULL;

  unsigned int byteCount = 0;
  const unsigned char * bytes = NULL;

  for( size_t i = 0; i < resources.size(); ++i )
  {
    pResource = resources[i];

    // deleted resource
    if( NULL == pResource )
      continue;

    // skip DIR resource
    if( pResource->getType() == DBPF_DIR )
      continue;

    // remember this resource's offset
    size_t offset = ftell( f );
    pResource->setLocation( (unsigned int)offset );

    // write resource
    byteCount = pResource->getRawByteCount();
    bytes = pResource->getRawBytes();
    fwrite( bytes, 1, byteCount, f );
  }

  return true;
}


static unsigned int gOffsetOfNewDIR = 0;
static unsigned int gEntryCountOfNewDIR = 0;

bool DBPFtype::writeDIR( FILE * f, vector< DBPF_resourceType * > & resources )
{
  // offset of the DIR resource
  size_t offset = ftell( f );

  gOffsetOfNewDIR = (unsigned int)offset; // temporary hack, until I make DIR a proper resource subclass...
  gEntryCountOfNewDIR = 0;


  DBPF_resourceType * pResource = NULL;
  unsigned int foo = 0, sizeUnc = 0, sizeCmp = 0;

  for( size_t i = 0; i < resources.size(); ++i )
  {
    pResource = resources[i];

    // skip deleted resources
    if( NULL == pResource )
      continue;

    // old DIR resource, save the new DIR offset
    // (actually, this won't work right now, since I don't have a DIR resource subclass... so save in gOffset...)

    if( DBPF_DIR == pResource->getType() )
    {
      pResource->setLocation( (unsigned int)offset );
      continue;
    }

    // non-DIR resource, if compressed, write its DIR entry

    if( true == pResource->isCompressed( sizeCmp, sizeUnc ) )
    {
      foo = pResource->getType();     fwrite( &foo, sizeof(unsigned int), 1, f );
      foo = pResource->getGroup();    fwrite( &foo, sizeof(unsigned int), 1, f );
      foo = pResource->getInstance(); fwrite( &foo, sizeof(unsigned int), 1, f );
      if( 1 == this->muIndexVersionMinor )
      { foo = pResource->getInstance2(); fwrite( &foo, sizeof(unsigned int), 1, f ); }
      fwrite( &sizeUnc, sizeof(unsigned int), 1, f );

      // temporary hack until I make DIR a proper subclass of resource
      ++gEntryCountOfNewDIR;
    }
  }

  return true;
}


bool DBPFtype::writeIndexTable( FILE * f, vector< DBPF_resourceType * > & resources )
{
  // remember index table offset

  unsigned int offset = (unsigned int)ftell( f );


  // write index table

  DBPF_resourceType * pResource = NULL;
  unsigned int foo = 0;
  unsigned int indexEntryCount = 0;

  for( size_t i = 0; i < resources.size(); ++i )
  {
    pResource = resources[i];

    // deleted resource

    if( NULL == pResource )
      continue;

    // DIR, skip (actually, we don't have a DIR subclass of resource yet, if we did, we wouldn't want to skip it...)
    if( DBPF_DIR == pResource->getType() )
      continue;

    // valid resource

    ++indexEntryCount;

    foo = pResource->getType();     fwrite( &foo, sizeof(unsigned int), 1, f );
    foo = pResource->getGroup();    fwrite( &foo, sizeof(unsigned int), 1, f );
    foo = pResource->getInstance(); fwrite( &foo, sizeof(unsigned int), 1, f );
    if( 1 == this->muIndexVersionMinor )
    { foo = pResource->getInstance2(); fwrite( &foo, sizeof(unsigned int), 1, f ); }
    foo = pResource->getLocation(); fwrite( &foo, sizeof(unsigned int), 1, f );
    foo = pResource->getRawByteCount(); fwrite( &foo, sizeof(unsigned int), 1, f );
  }

  // hack - last entry, for DIR

  // DIR TGI
  foo = this->mDIR.mMyIndexEntry.muTypeID;
  fwrite( &foo, sizeof(unsigned int), 1, f );
  foo = this->mDIR.mMyIndexEntry.muGroupID;
  fwrite( &foo, sizeof(unsigned int), 1, f );
  foo = this->mDIR.mMyIndexEntry.muInstanceID;
  fwrite( &foo, sizeof(unsigned int), 1, f );
  if( 1 == this->muIndexVersionMinor )
  {
    foo = this->mDIR.mMyIndexEntry.muInstanceID2;
    fwrite( &foo, sizeof(unsigned int), 1, f );
  }

  // DIR location
  foo = gOffsetOfNewDIR;
  fwrite( &foo, sizeof(unsigned int), 1, f );

  // DIR size
  unsigned int DIRentrySize = 16;
  if( 1 == this->muIndexVersionMinor )
    DIRentrySize = 20;
  foo = gEntryCountOfNewDIR * DIRentrySize;
  fwrite( &foo, sizeof(unsigned int), 1, f );

  // increment resource count, DIR is a resource
  ++indexEntryCount;



  // go back to header, write index table offset

  fseek( f, 40, SEEK_SET );
  fwrite( &offset, sizeof(unsigned int), 1, f );

  // back in header, write size of index table in bytes

  unsigned int entrySize = 20;
  if( 1 == this->muIndexVersionMinor )
    entrySize = 24;
  unsigned int indexSize = indexEntryCount * entrySize;
  fwrite( &indexSize, sizeof(unsigned int), 1, f );


  return true;
}
