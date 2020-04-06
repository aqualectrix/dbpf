/**
 * file: DBPF_resource.cpp
 * author: CatOfEvilGenius
 *
 * abstract base class for other DBPF resources
**/

#include <DBPF_resource.h>
#include <DBPF_types.h>
#include <DBPFcompress.h>

// -------------------------------------------------


DBPF_resourceType::DBPF_resourceType()
{
  this->mpRawBytes = NULL;
  this->mpProperties = NULL;
  this->mpCPF = NULL;
  clear();
}


DBPF_resourceType::~DBPF_resourceType()
{
  clear();

  if( this->mpProperties != NULL )
  {
    delete this->mpProperties;
    this->mpProperties = NULL;
  }

  if( this->mpCPF != NULL )
  {
    delete this->mpCPF;
    this->mpCPF = NULL;
  }
}


void DBPF_resourceType::clear()
{
  this->mbInitialized = false;
  this->mbChanged = false;
  this->miChangeInRawBytesCount = 0;

  this->mMyIndexEntry.clear();

  this->muRawBytesCount = 0;
  if( this->mpRawBytes != NULL )
    delete [] this->mpRawBytes;
  this->mpRawBytes = NULL;

  if( this->mpProperties != NULL )
    mpProperties->clear();

  if( this->mpCPF != NULL )
    mpCPF->clear();

  this->mstrName.assign( "" );
  this->mstrDesc.assign( "" );
}


void DBPF_resourceType::initIndexEntry( const DBPFindexType & entry )
{
  this->mMyIndexEntry = entry;
}


void DBPF_resourceType::getTGIR( DBPF_TGIRtype & tgir ) const
{
  tgir.muTypeID = this->getType();
  tgir.muGroupID = this->getGroup();
  // instance2 is instance (high)
  tgir.muInstanceID = this->getInstance();
  tgir.muResourceID = this->getInstance2();
}



#ifdef _DEBUG
void DBPF_resourceType::dump( FILE * f ) const
{
  char strType[256];
  dbpfResourceTypeToString( this->mMyIndexEntry.muTypeID, strType );

  fprintf( f, "\n-------------------------------------------------\n" );
  fprintf( f, "%s %s", strType, this->mstrName.c_str() );
  fprintf( f, "\n-------------------------------------------------\n" );
  fprintf( f, "\n" );

  fprintf( f, "initialized: " );
  if( this->mbInitialized ) fprintf( f, "TRUE\n" );
  else                      fprintf( f, "false\n" );
  fprintf( f, "changed: " );
  if( this->mbChanged ) fprintf( f, "TRUE\n" );
  else                  fprintf( f, "false\n" );
  fprintf( f, "\n" );

  fprintf( f, "-- index entry --\n" );
  this->mMyIndexEntry.dump( f );
  fprintf( f, "\n" );

  fprintf( f, "-- raw bytes --\n" );
  fprintf( f, "size: %u\n", this->muRawBytesCount );
  fprintf( f, "address: 0x%x\n", (size_t)(this->mpRawBytes) );
  fprintf( f, "\n" );

  fprintf( f, "name: %s\n", this->mstrName.c_str() );
  fprintf( f, "desc: %s\n", this->mstrDesc.c_str() );
  fprintf( f, "\n" );
} // dump
#endif


/**
<pre>
 * input: none
 *
 * output: uncompressed byte count - only valid if true returned
 *         compressed byte count - only valid if true returned
 *
 * returns: true - if this resource is compressed (and initialized and unchanged)
 *          false - if this resource in not compressed,
 *            or if it is not initialized or if it changed.
 * 
 * purpose: Check if this resource is compressed.  If so, get the uncompressed and compressed sizes.
</pre>
**/
bool DBPF_resourceType::isCompressed( unsigned int & cmprByteCount, unsigned int & uncByteCount ) const
{
  if( this->mbInitialized == false )
    return false;

  if( this->mbChanged == true )
    return false;

  if( NULL == this->mpRawBytes )
  { fprintf( stderr, "ERROR: DBPF_resourceType.isCompressed, raw bytes should not be null, something has gone wrong\n" );
    return false;
  }

  unsigned int cmprID = 0;
  return( dbpfGetCompressedHeader( this->mpRawBytes, cmprByteCount, cmprID, uncByteCount ) );
}


/**
<pre>
 * returns true if compression was successful, raw bytes are now compressed, use isCompressed to get compressed size
 * returns false if compression was unsuccessful, raw bytes remain uncompressed
 * also returns false if resource is not initialized or is changed, should call updateRawBytes first if it is changed
</pre>
**/
bool DBPF_resourceType::compressRawBytes()
{
  // sanity check

  // not initialized?
  if( false == this->isInitialized() )
  { fprintf( stderr, "ERROR: DBPF_resourceType, can't compress this resource, it hasn't been initialized yet\n" );
    return false;
  }

  // changed?
  if( true == this->isChanged() )
  { fprintf( stderr, "WARNING: DBPF_resourceType, won't compress, this resource is changed, call updateRawBytes first, then compress\n" );
    return false;
  }

  // already compressed?
  unsigned int cmpSize, uncSize;
  if( true == this->isCompressed( cmpSize, uncSize ) )
  { fprintf( stderr, "WARNING: DBPF_resourceType, won't compress, resource is already compressed\n" );
    return false;
  }

  // null bytes?
  if( NULL == this->mpRawBytes )
  { fprintf( stderr, "ERROR: DBPF_resourceType, can't compress, null bytes, something has gone very wrong\n" );
    return false;
  }


  // attempt to compress

  unsigned char * cmprBytes = NULL;
  unsigned int cmprByteCount = 0;
  if( false == dbpfCompress( this->mpRawBytes, this->muRawBytesCount, cmprBytes, cmprByteCount ) )
    return false;

  // delete old bytes, save new compressed bytes

  if( this->mpRawBytes != NULL )
    delete [] this->mpRawBytes;
  this->mpRawBytes = cmprBytes;

  // remember compressed size

  this->muRawBytesCount = cmprByteCount;

  return true;
}


bool DBPF_resourceType::getPropertyValue( const string propName, string & propValue ) const
{
  if( NULL == this->mpProperties )
  {
    fprintf( stderr, "WARNING: DBPF_resourceType.getPropertyValue, NULL mpProperties\n" );
    return false;
  }

  return( this->mpProperties->getPropertyValue( propName, propValue ) );
}


bool DBPF_resourceType::getPropertyValue(const std::string propName, DBPF_CPFitemType &propValue) const
{
  if( NULL == this->mpCPF )
  {
    fprintf( stderr, "WARNING: DBPF_resourceType.getPropertyValue, NULL mpCPF\n" );
    return false;
  }

  return( this->mpCPF->getPropertyValue( propName, propValue ) );
}


// this sets the mbChanged flag if the new propValue is different, and adjusts change in raw byte count
bool DBPF_resourceType::setPropertyValue( const string propName, string & propValue )
{
  if( NULL == this->mpProperties )
  {
    fprintf( stderr, "WARNING: DBPF_resourceType.setPropertyValue, NULL mpProperties\n" );
    return false;
  }

  bool bChanged = false;
  int iChangeBytes = 0;
  if( true == this->mpProperties->setPropertyValue( propName, propValue, bChanged, iChangeBytes ) )
  {
    if( bChanged )
    {
      this->miChangeInRawBytesCount += iChangeBytes;
      this->mbChanged = true;
    }
    return true;
  }

  return false;
}


// this sets the mbChanged flag if the new propValue is different, and adjusts change in raw byte count
bool DBPF_resourceType::setPropertyValue( const string propName, DBPF_CPFitemType & propValue )
{
  if( NULL == this->mpCPF )
  {
    fprintf( stderr, "WARNING: DBPF_resourceType.setPropertyValue, NULL mpCPF\n" );
    return false;
  }

  bool bChanged = false;
  int iChangeBytes = 0;
  if( true == this->mpCPF->setPropertyValue( propName, propValue, bChanged, iChangeBytes ) )
  {
    if( bChanged )
    {
      this->miChangeInRawBytesCount += iChangeBytes;
      this->mbChanged = true;
    }

    // if property is "name", change the resource name
    if( 0 == strcmp( "name", propName.c_str() )
     && CPF_STRING == propValue.miType )
      this->mstrName.assign( propValue.mstrValue.c_str() );

    return true;
  }

  return false;
}


// -----------------------------------------------


DBPF_undecodedType::DBPF_undecodedType()
{
  this->mpRawBytes = NULL;
  clear();
}


DBPF_undecodedType::~DBPF_undecodedType()
{
  clear();
}


bool DBPF_undecodedType::initFromByteStream(
  const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead )
{
  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_undecodedType.initFromByteStream, null data\n" );
    return false;
  }

  // simple init, just save the raw bytes, no decoding

  this->mbInitialized = false;
  clear();

  this->initIndexEntry( entry );

  this->muRawBytesCount = byteCountToRead;
  this->mpRawBytes = data;

  this->mbInitialized = true;

  return true;
}


/**
 * raw bytes were never decoded, nothing can be changed,
 * so no update needed, this leaves raw bytes as they are and returns true
**/
bool DBPF_undecodedType::updateRawBytes()
{
  // don't need to set mbChanged to false here, because it never gets set to true
  return true;
}


#ifdef _DEBUG
void DBPF_undecodedType::dump( FILE * f ) const
{
  DBPF_resourceType::dump( f );

  fprintf( f, "this resource has has not been decoded\n" );
  fprintf( f, "that's why the name and description appear blank\n" );
  fprintf( f, "\n" );
}
#endif
