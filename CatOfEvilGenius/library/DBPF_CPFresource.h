#ifndef DBPF_GZPS_H_CATOFEVILGENIUS
#define DBPF_GZPS_H_CATOFEVILGENIUS

#include "DBPF_resource.h"
#include "DBPF_CPF.h"

// these are used by GZPS, XHTN
#define HAIR_BLACK     1
#define HAIR_BROWN     2
#define HAIR_BLOND     3
#define HAIR_RED       4
#define HAIR_GREY      5


// base class for resources that have CPF data (GZPS, XHTN)
class DBPF_CPFresourceType : public DBPF_resourceType
{
public:
  DBPF_CPFresourceType();
  virtual ~DBPF_CPFresourceType();

  virtual void clear();
  virtual bool initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead );
  virtual bool updateRawBytes();

#ifdef _DEBUG
  virtual void dump( FILE * f ) const;
#endif

};


// DBPF_GZPS_H_CATOFEVILGENIUS
#endif
