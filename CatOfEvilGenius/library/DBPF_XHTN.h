#ifndef DBPF_XHTN_H_CATOFEVILGENIUS
#define DBPF_XHTN_H_CATOFEVILGENIUS

#include <DBPF_resource.h>
#include <DBPF_CPFresource.h>

class DBPF_XHTNtype : public DBPF_CPFresourceType
{
public:
  DBPF_XHTNtype() {}
  ~DBPF_XHTNtype() {}

  bool setFamily( string family );
  bool setHairColor( int color );
};


// DBPF_XHTN_H_CATOFEVILGENIUS
#endif
