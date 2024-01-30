/**
 * file: DBPF_TXTR.h
 * author: CatOfEvilGenius
**/

#ifndef DBPF_TXTR_H_CATOFEVILGENIUS
#define DBPF_TXTR_H_CATOFEVILGENIUS

#include "DBPF_resource.h"
#include "DBPF_RCOL.h"


class DBPF_TXTRtype : public DBPF_resourceType
{
public:
  DBPF_TXTRtype();
  ~DBPF_TXTRtype();

  void clear();
  bool initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead );
  bool updateRawBytes();

  string getTextureType();
  string getSubsetName();
  bool isImageSameAs( const DBPF_TXTRtype * pOther ) const;

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

private:
  DBPF_RCOLtype mRCOL;

  string mstrBlockName; // this should be cImageData
  unsigned int muBlockID; // this should always be 0x0x1c4a276c or DBPF_TXTR
  unsigned int muBlockVersion; // could be 7, 8, or 9

  unsigned int muWidth,
               muHeight,
               muFormatCode,
               muMipmapCode;

  // 1 object, 2 outfit, 3 UI
  float mfPurpose;

  unsigned int muOuterLoopCount;
  unsigned int muInnerLoopCount;

  // image, raw byte data, everything after "filename repeat"
  unsigned char * mpImageData;
  // image data size in bytes
  unsigned int muImageDataSize;

}; // TXTR class


// DBPF_TXTR_H_CATOFEVILGENIUS
#endif
