#include <cstdlib>
#include <cstdio>
#include <DBPF_types.h>
#include <DBPF_byteStreamFunctions.h>
#include <DBPF_CPFresource.h>


DBPF_CPFresourceType::DBPF_CPFresourceType()
{
  clear();
}


DBPF_CPFresourceType::~DBPF_CPFresourceType()
{
  clear();
}


void DBPF_CPFresourceType::clear()
{
  DBPF_resourceType::clear();
}


#ifdef _DEBUG
void DBPF_CPFresourceType::dump( FILE * f ) const
{
  DBPF_resourceType::dump( f );

  fprintf( f, "-- CPF data --\n" );
  if( this->mpCPF != NULL )
    this->mpCPF->dump( f );
  else
    fprintf( f, "null cpf\n" );

  fprintf( f, "\n" );
}
#endif


bool DBPF_CPFresourceType::initFromByteStream( const DBPFindexType & entry, unsigned char * data, 
                                        const unsigned int byteCountToRead )
{
  // sanity check
  if( NULL == data )
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.initFromByteStream, null data\n" );
    return false;
  }

  // not yet initialized
  this->mbInitialized = false;
  this->clear();

  // index entry
  this->initIndexEntry( entry );

  // raw bytes
  this->muRawBytesCount = byteCountToRead;
  this->mpRawBytes = data;

  // ---------------------------------------------------------

  // allocate CPF if needed
  if( NULL == this->mpCPF )
  { this->mpCPF = new DBPF_CPFtype();
    if( NULL == this->mpCPF )
    { fprintf( stderr, "ERROR: DBPF_CPFresourceType.initFromByteStream, failed to allocate CPF\n" );
      return false;
    }
  }

  // clear CPF
  this->mpCPF->clear();

  // init CPF
  unsigned char * bytes2 = NULL;
  if( false == this->mpCPF->initFromByteStream( data, bytes2 ) )
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.initFromByteStream, failed to init CPF\n" );
    return false;
  }
  data = bytes2;

  // get resource name from CPF
  DBPF_CPFitemType nameVal;
  if( true == this->getPropertyValue( "name", nameVal ) )
    this->mstrName.assign( nameVal.mstrValue.c_str() );

  // --------------------------------------
  // done initializing
  this->mbInitialized = true;

  return true;
}


bool DBPF_CPFresourceType::updateRawBytes()
{
 // sanity check
  if( this->mbInitialized == false )
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.updateRawBytes, can't update, not initialized yet\n" );
    return false;
  }

  if( NULL == this->mpRawBytes )
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.updateRawBytes, can't update, null byte array\n" );
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
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.updateRawBytes, failed to allocate memory\n" );
    return false;
  }

  // =================================================================

  // update bytes
  // ------------

  unsigned char * ptrWrite = newBytes;

  if( false == this->mpCPF->writeToByteStream( ptrWrite ) )
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.updateRawBytes, failed to write CPF to byte stream\n" );
    return false;
  }

  // =================================================================

  // did we write the correct amount of bytes?

  unsigned int sizeWritten = (unsigned int)(ptrWrite - newBytes);
  if( sizeWritten != newSize )
  { fprintf( stderr, "ERROR: DBPF_CPFresourceType.updateRawBytes, something went wrong, bytes written %u, expected new size %u\n",
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
}
