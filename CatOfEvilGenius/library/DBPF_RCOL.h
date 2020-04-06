/**
 * file: DBPF_RCOL.h
 * author: CatOfEvilGenius
 *
 * RCOL is used by
 *
    *  FB00791E ANIM filetype
    * 4D51F042 CINE filetype
    * E519C933 CRES filetype
    * AC4F8687 GMDC filetype
    * 7BA3838C GMND filetype
    * C9C81B9B LGHT filetype - Lighting (Ambient Light)
    * C9C81BA3 LGHT filetype - Lighting (Directional Light)
    * C9C81BA9 LGHT filetype - Lighting (Point Light)
    * C9C81BAD LGHT filetype - Lighting (Spot Light)
    * ED534136 LIFO
    * FC6EB1F7 SHPE
    * 49596978 TXMT aka MATD - Material Definition
    * 1C4A276C TXTR 
**/

#ifndef DBPF_RCOL_H_CATOFEVILGENIUS
#define DBPF_RCOL_H_CATOFEVILGENIUS

#include <cstdlib>
#include <vector>
#include <DBPF_types.h> // TGIR type
using namespace std;


/**
<pre>
 * RCOL header data
 * This is used by TXMT, TXTR
</pre>
**/

class DBPF_RCOLtype
{
public:
  DBPF_RCOLtype();
  ~DBPF_RCOLtype();

  bool initFromByteStream( unsigned char * data, unsigned char * & restOfData );
  void clear();

  bool isInitialized() const { return this->mbInitialized; }
  // not likely ;)  no "set" function yet
  bool isChanged() const { return this->mbChanged; }

  // how big is the RCOL in bytes?
  unsigned int getRawByteCount() const { return this->muRawBytesCount; }
  // write out raw byte data of this RCOL to the given byte array
  bool DBPF_RCOLtype::writeToByteStream( unsigned char * & bytes );

#ifdef _DEBUG
  void dump( FILE * f ) const;
#endif

  unsigned int getLinkCount() const { return this->muLinkCount; }
  DBPF_TGIRtype getLink( const unsigned int i ) const
  { return( (this->mLinks)[i] ); }

  unsigned int getIndexItemCount() const { return this->muIndexItemCount; }
  unsigned int getIndexItem( const unsigned int i ) const
  { return( (this->mIndex)[i] ); }

private:
  bool mbInitialized;
  bool mbChanged;

  // how big is the RCOL in bytes?  this could change if the RCOL changed, but since this class has no "set" functions right now, that won't happen
  unsigned int muRawBytesCount;

  bool mbLinksHaveResourceID; // if true, use TGIR, if not, just TGI

  unsigned int muLinkCount;
  vector< DBPF_TGIRtype > mLinks;
  unsigned int muIndexItemCount;
  vector< unsigned int > mIndex; // contains RCOL IDs, these appear in individual data blocks
};


// DBPF_RCOL_H_CATOFEVILGENIUS
#endif
