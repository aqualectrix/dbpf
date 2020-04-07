/**
 * file: DBPF.h
 * author: CatOfEvilGenius
 *
 * class DBPFtype - package files, header, index table, various resources
 * class DBPF_DIRtype - DIR resource, directory of compressed resources
 * class DBPFindexType - used by the index table of DBPFtype and also used by the DBPF_DIRtype
**/

#ifndef DBPF_H_CATOFEVILGENIUS
#define DBPF_H_CATOFEVILGENIUS

#include <string>
#include <vector>

using namespace std;

// forward declaration
class DBPF_resourceType;

/**
<pre>
 * helper class of DBPFtype
 * this type is for an individual entry in the index table of a DBPF package file,
 * there are several of these in the index table
 *
 * you can check the type of a resource by looking at muTypeID in its index entry
 * compare to predefined constants such as DBPF_BHAV, DBPF_TXTR, etc.
</pre>
**/
class DBPFindexType
{
public:
  // instance2 is instance (high)
  // instance  is instance (low)
  unsigned int muTypeID, muGroupID, muInstanceID, muInstanceID2, muLocation, muSize;

  DBPFindexType();
  ~DBPFindexType() {};
  void clear();
  void read( FILE * f, bool bRead2ndInstanceID, bool bReadLocation );
#ifdef _DEBUG
  static void dumpTableHeader( FILE * f ); // prints a header for a table of dump lines
  void dumpTypeOnly( FILE * f ) const;
  void dump( FILE * f ) const;
#endif
};


/**
<pre>
 * DIR resource type, directory of compressed files
 * used to figure out which resources are compressed,
 * stores the *decompressed* size
</pre>
**/
class DBPF_DIRtype
{
public:
  // copy of the index entry for the DIR resource from the package's index table
  DBPFindexType mMyIndexEntry;

  // directory of compressed resources
  vector< DBPFindexType > mIndexTableCompressed;

public:
  DBPF_DIRtype() {}
  ~DBPF_DIRtype();

  bool read( FILE * f, DBPFindexType & myIndexEntry, bool bRead2ndInstance );
  bool write( FILE * f, bool bWrite2ndInstance );
  unsigned int getCompressedItemCount() const;
  bool isCompressed( const DBPFindexType entry, unsigned int & decmpSize ) const;
#ifdef _DEBUG
  void dump();
#endif
};


/**
<pre>
 * DBPF package file class
 * =======================
 *
 * DBPF file contains a header, index entries, various records (TXMT, TXTR, BINX, etc.)
 * and usually a DIR resource that says what's compressed
 *
 * - use the read function first, this reads the header, index table, and DIR, if it exists
 * - use getItemCount to find out how many resource items there are in the package
 * - to get a resource item from the package
 * --- get the item's index entry with getIndexEntry (see DBPFindexType)
 * --- check if the item is compressed with isCompressed
 * --- get the the item's data with getData
 * --- if it was compressed, use the global uncompress function
</pre>
**/
class DBPFtype
{
private:
  char mstrFileName[1024];
  FILE * mFile;
  unsigned int muVersionMajor, muVersionMinor;
  unsigned int muIndexVersionMajor, muIndexVersionMinor,
        muIndexEntryCount, muIndexOffset, muIndexSizeInBytes;
  unsigned int muUnknowns[3], muUnknownLast;
  unsigned int muDates[2]; // date created, date modified
  unsigned int muHoleEntryCount, muHoleOffset, muHoleSize;

  vector< DBPFindexType > mIndexTable;

  bool mbDIRexists;
  DBPF_DIRtype mDIR;

public:
  DBPFtype();
  ~DBPFtype();

  bool read( const char * fileName, size_t & fileSize );
  bool directoryOfCompressedStuffExists() const { return mbDIRexists; }

  unsigned int getItemCount() const { return muIndexEntryCount; }
  bool contains( const unsigned int type ) const;
  unsigned int howMany( const unsigned int type ) const;
  bool getIndexEntry( const unsigned int k, DBPFindexType & entry ) const;
  bool isCompressed( const DBPFindexType indexEntry, unsigned int & decmpSize ) const;
  bool getData( const DBPFindexType indexEntry, unsigned char * & bytes, unsigned int & byteCount );

  bool write( const char * fileName, vector< DBPF_resourceType * > & resources, size_t & fileSize );

private:
  bool readHeader();
  bool readIndexTable();
  bool readDIR();
  bool openFile();
  void closeFile();
  bool writeHeader( FILE * f, vector< DBPF_resourceType * > & resources );
  bool writeResources( FILE * f, vector< DBPF_resourceType * > & resources );
  bool writeDIR( FILE * f, vector< DBPF_resourceType * > & resources );
  bool writeIndexTable( FILE * f, vector< DBPF_resourceType * > & resources );
};


// --------------------------------------------------------------------------------------
// file stuff, in DBPF_2.cpp

// given a filepath in str, gives you just the filename in strOut
bool justTheFilename( const char * str, string strOut );

// output file name is input filename (- .package) + strAppend + .package
void makeOutputFileName( string & strOut, const char * strIn, const char * strAppend );

// reads package file, gives set of resources, specified types will be uncompressed and initialized
bool readPackage( const char * filename,                         // IN
                  DBPFtype & package,                            // IN/OUT
                  vector< unsigned int > & typesToInit,          // IN
                  vector< DBPF_resourceType * > & resources );   // OUT

// writes package file (first, compresses all resources)
bool writeCompressedPackage( const char * filename,              // IN
                   DBPFtype & package,                           // IN
                   vector< DBPF_resourceType * > & resources );  // IN



// DBPF_H_CATOFEVILGENIUS
#endif
