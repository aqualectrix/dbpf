/**
 * file:   DBPF_3IDR.h
 * author: CatOfEvilGenius
**/

#ifndef H_DBPF_3IDR_CATOFEVILGENIUS
#define H_DBPF_3IDR_CATOFEVILGENIUS

#include <cstdlib>
#include <vector>
#include <DBPF_resource.h>
#include <DBPF_types.h>       // TGIR type

using namespace std;


class DBPF_3IDRtype : public DBPF_resourceType
{
public:
  DBPF_3IDRtype();
  ~DBPF_3IDRtype();

  void clear();
  bool initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead );
  bool updateRawBytes();

  unsigned int get3IDR_ID() const { return this->mu3IDR_ID; }
  unsigned int getIndexType() const { return this->muIndexType; }
  unsigned int getTGIRcount() const { return this->muTGIRcount; }
  bool getTGIRentry( const unsigned int index, DBPF_TGIRtype & TGIR ) const;

  // change value of existing TGIR entry
  bool setTGIRentry( const unsigned int index, const DBPF_TGIRtype & TGIR );
  // add a new TGIR entry to end of TGIR entries
  void addTGIRentry( const DBPF_TGIRtype & TGIR );
  // remove an existing TGIR entry
  bool removeTGIRentry( const unsigned index );

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

protected:
  // should be 0xDEADBEEF
  unsigned int mu3IDR_ID;
  // 1 or 2, 1: 12 bytes (no resource), 2: 16 bytes (has resource)
  unsigned int muIndexType;

  // TGIR entries
  unsigned int muTGIRcount;
  vector< DBPF_TGIRtype > mTGIRs;
};


// H_DBPF_3IDR_CATOFEVILGENIUS
#endif
