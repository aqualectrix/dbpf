#ifndef DBPF_GZPS2_H_CATOFEVILGENIUS
#define DBPF_GZPS2_H_CATOFEVILGENIUS

#include "DBPF_resource.h"
#include "DBPF_CPFresource.h"

class DBPF_GZPStype : public DBPF_CPFresourceType
{
public:
  DBPF_GZPStype() {}
  ~DBPF_GZPStype() {}

  unsigned int getAge() const;
  string getFamily() const;

  bool setFamily( string family );
  bool setHairtone( string hairtone );
  bool setHairColor( int color );
};


// DBPF_GZPS2_H_CATOFEVILGENIUS
#endif
