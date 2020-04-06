/**
 * file:   DBPF_STR.h
 * author: CatOfEvilGenius
 *
 * The STR class can be used for DBPF resources of type STR#, CATS, CTSS, and TTA.
**/

#ifndef H_DBPF_STR_CATOFEVILGENIUS
#define H_DBPF_STR_CATOFEVILGENIUS

#include <string>
#include <vector>
#include <DBPF_resource.h>

using namespace std;


/**
 * helper type for DBPF_STRtype
**/
typedef struct DBPF_textStruct
{
  unsigned char mLanguageCode;
  string mstrValueText;
  string mstrDescText;
} DBPF_textType;


/**
 * The STR class can be used for DBPF resources of type STR#, CATS, CTSS, and TTA.
**/
class DBPF_STRtype : public DBPF_resourceType
{
public:
  DBPF_STRtype() { clear(); }
  ~DBPF_STRtype() { clear(); }

  void clear();
  bool initFromByteStream( const DBPFindexType & entry, unsigned char * data, const unsigned int byteCountToRead );
  bool updateRawBytes();

  unsigned short getFormatCode() const { return msFormatCode; }
  unsigned short getTextItemCount() const { return msTextItemCount; }
  bool getTextItem( const unsigned short index, DBPF_textType & text );

  // change value of existing text item
  bool setTextItem( const unsigned short index, const DBPF_textType & text );

  // add text item to end of list
  void addTextItem( const DBPF_textType & text );

  // remove a text item (and compact list to close hole)
  bool removeTextItem( const unsigned short index );


#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

protected:
  unsigned short msFormatCode;
  unsigned short msTextItemCount;
  vector< DBPF_textType > mTextItems;
};


// H_DBPF_STR_CATOFEVILGENIUS
#endif
