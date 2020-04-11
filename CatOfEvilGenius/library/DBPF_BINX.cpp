#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <string>

#include "DBPF_BINX.h"

bool DBPF_BINXtype::setSortIndex( int index )
{
  string prop_name = "sortindex";
  DBPF_CPFitemType item;
  item.miValue = index;

  // Very old hair files have sortindex with type UINT
  // instead of INT. We need to check what the value type is
  // so we sent the right type and don't have the change rejected.
  DBPF_CPFitemType old_value;
  if (false == this->getPropertyValue(prop_name, old_value))
  {
    cerr << "setSortIndex: Failed to get old property value for " << prop_name <<
            ". This property may not exist in this resource." << endl;
    return false;
  }
  int old_type = old_value.miType;

  if (CPF_INT != old_type && CPF_INT2 != old_type) {
    char val_type[20];
    old_value.typeToString(val_type);
    cerr << "setSortIndex: " << val_type << " is not a supported type for " << prop_name <<
            ". The file you are processing is borked and you will need to " <<
            "open it in SimPE to change the type of " << prop_name <<
            "to dtInteger." << endl;
  }

  // Okay, now we know what the old type was and that it's valid.
  item.miType = old_type;

  return( this->setPropertyValue(prop_name, item ) );
}
