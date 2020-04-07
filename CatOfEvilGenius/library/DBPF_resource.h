/**
 * file: DBPF_resource.h
 * author: CatOfEvilGenius
 *
 * DBPF_resource
 * abstract base class for other DBPF resources
 *
 * DBPF_undecoded
 * This class does NOT decode the raw bytes given to it in init,
 * but it is useful.  It's for items / resources you want to read in
 * and write out without changes, except to their index entries (location).
**/


#ifndef DBPF_RESOURCE_H_CATOFEVILGENIUS
#define DBPF_RESOURCE_H_CATOFEVILGENIUS

#include <string>
#include "DBPF.h" // index entry type
#include "DBPF_CPF.h" // properties, set of name/value pairs
#include "DBPF_types.h" // TGIR
using namespace std;



class DBPF_resourceType
{
public:
  // this should call clear, set mpRawBytes to NULL (do this in all subclasses!)
  DBPF_resourceType();

  // this should call clear
  virtual ~DBPF_resourceType();

#ifdef _DEBUG
  /**
   * prints data for debugging or just viewing,
   * can pass in stdout to print to the screen, stderr, or an open text file handle
   * !!! derived classes should call base class dump from their dump !!!
   **/
  virtual void dump( FILE * f ) const;
#endif

  /**
   *!!! subclasses should call base class clear from their clear !!!
   * free any memory held by this object, set all member variables to zero, null, empty string, etc.
  **/
  virtual void clear();

  /**
  <pre>
   * this should set mbInitialized to false, call clear,
   * save data and byteCountToRead and mpRawBytes and muRawBytesCount,
   * call initTGI, do init of other class specific stuff,
   * then set mbInitialized to true
  </pre>
  **/
  virtual bool initFromByteStream(
    const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead ) = 0;

  /**
  <pre>
   * How big is this resource in bytes?
   * This may be compressed or uncompressed size.
   * Use isCompressed to be sure.
   * This number is only valid if the resource is unchanged.
   * Use isChanged to check.
  </pre>
  **/
  unsigned int getRawByteCount() const { return this->muRawBytesCount; }

  /**
  <pre>
   * Get the raw byte data for this resource (not modifiable).
   * To get the size of the byte array, in bytes, use getRawByteCount.
   * The bytes are only valid if the changed flag is false.
   * If the changed flag is true, use updateRawBytes before calling getRawBytes.
  </pre>
  **/
  const unsigned char * getRawBytes() const { return this->mpRawBytes; }

  /**
  <pre>
   * after decoding a file, and making *changes* to it, the mpRawBytes data is no longer valid,
   * call this function to update it, the changed flag will get set from true to false,
   * this creates *uncompressed* data
  </pre>
  **/
  virtual bool updateRawBytes() = 0;

  // If this resource is uncompressed, you can attempt to compress it with compress.
  bool compressRawBytes();

  // check if this resource is compressed, if so, what's the compressed and uncompressed sizes
  bool isCompressed( unsigned int & cmprByteCount, unsigned int & uncByteCount ) const;


  bool isInitialized() const { return this->mbInitialized; }
  bool isChanged() const { return this->mbChanged; }

  unsigned int getType() const { return( this->mMyIndexEntry.muTypeID ); }
  unsigned int getGroup() const { return( this->mMyIndexEntry.muGroupID ); }
  unsigned int getInstance() const { return( this->mMyIndexEntry.muInstanceID ); }
  unsigned int getInstance2() const { return( this->mMyIndexEntry.muInstanceID2 ); }
  void getTGIR( DBPF_TGIRtype & tgir ) const;
  // offset into the file when read/written from/to disk
  unsigned int getLocation() const { return( this->mMyIndexEntry.muLocation ); }
  // this is the offset into the file when read/written, so changing this does not affect the changed flag of the resource
  void setLocation( const unsigned int loc ) { this->mMyIndexEntry.muLocation = loc; }
  string getName() const { return( string( this->mstrName ) ); }
  string getDescription() const { return( string( this->mstrDesc ) ); }

  // use these for TXMT
  // this sets the mbChanged flag if the new propValue is different
  bool setPropertyValue( const string propName, string & propValue );
  bool getPropertyValue( const string propName, string & propValue ) const;

  // use these for GZPS, XHTN
  // this sets the mbChanged flag if the new propValue is different
  bool setPropertyValue( const string propName, DBPF_CPFitemType & propValue );
  bool getPropertyValue( const string propName, DBPF_CPFitemType & propValue ) const;


protected:
  DBPFindexType mMyIndexEntry;
  void initIndexEntry( const DBPFindexType & entry );

protected:
  // if this is true, initFromByteStream has been done
  bool mbInitialized;

  // if this is true, raw bytes are no longer valid, need to be updated with updateRawBytes
  bool mbChanged;
  // change in raw bytes size, new size will be old size + change (must be signed, could be negative)
  int miChangeInRawBytesCount;

  /**
   * size of raw bytes in bytes (this is the *uncompressed* size)
  **/
  unsigned int muRawBytesCount;

  /**
   * raw byte data, got this from the package file, this gets decoded in initFromByteStream,
   * and updated by updateRawBytes
  **/
  unsigned char * mpRawBytes;

  string mstrName;
  string mstrDesc;

public:
  /**
   * set of string key/value pairs, not all resource types have properties, but many do,
   * subclasses that use mpProperties must allocate it with new before doing stuff with it
  **/
  DBPF_propertiesType * mpProperties;

  /**
   * set of string key / various type value pairs,
   * not all resources have this, but many do, such as GZPS and XHTN
  **/
  DBPF_CPFtype * mpCPF;
};


class DBPF_undecodedType : public DBPF_resourceType
{
public:
  DBPF_undecodedType();
  ~DBPF_undecodedType();

  bool initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead );
  bool updateRawBytes();

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif
};


// DBPF_RESOURCE_H_CATOFEVILGENIUS
#endif
