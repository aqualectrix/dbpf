#ifndef DBPF_BINX_H_AQUALECTRIX
#define DBPF_BINX_H_AQUALECTRIX

#include "DBPF_resource.h"
#include "DBPF_CPFresource.h"

class DBPF_BINXtype : public DBPF_CPFresourceType
{
public:
  DBPF_BINXtype() {}
  ~DBPF_BINXtype() {}

  bool setSortIndex(int index);
};

// DBPF_BINX_H_AQUALECTRIX
#endif
