#include <cstdlib>
#include <cstdio>
#include <string>

#include "DBPF_BINX.h"

bool DBPF_BINXtype::setSortIndex( int index )
{
  DBPF_CPFitemType item;
  item.miType = CPF_INT2;
  item.miValue = index;
  return( this->setPropertyValue( "sortindex", item ) );
}
