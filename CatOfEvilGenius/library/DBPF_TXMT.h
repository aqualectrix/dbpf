/**
 * file: DBPF_TXMT.h
 * author: CatOfEvilGenius
**/

#ifndef DBPF_TXMT_H_CATOFEVILGENIUS
#define DBPF_TXMT_H_CATOFEVILGENIUS

#include <string>  // <string> class
#include <vector>

#include "DBPF_resource.h"
#include "DBPF_RCOL.h"

using namespace std;
#ifdef __GNUC__
using namespace __gnu_cxx;
#else
using namespace stdext;
#endif



class DBPF_TXMTtype : public DBPF_resourceType
{
public:
  DBPF_TXMTtype();
  ~DBPF_TXMTtype();

  void clear();
  bool initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead );
  bool updateRawBytes();

  bool getSubsetName( string & subsetName );

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

private:
  DBPF_RCOLtype mRCOL;

  string mstrBlockName; // this should be cMaterialDefinition
  unsigned int muBlockID; // this should always be 0x49596978 or DBPF_TXMT
  unsigned int muBlockVersion; // could be 8, 9, 10, or 11

  string mstrMaterialType;

  // this is only used if block version > 8
  unsigned int muTextureNameCount;
  /**
   * These are not the necessarily the names of the textures this TXMT uses,
   * especially in the case of CC recolors, rather, these are the names
   * of Maxis textures to use if the expected textures are not available, I think
   *
   * only used if block version > 8
   **/
  vector< string > mTextureNames;
};


// DBPF_TXMT_H_CATOFEVILGENIUS
#endif
