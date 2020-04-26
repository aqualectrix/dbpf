/*
 * hideProcess.cpp :
 * Zeroes out most properties to hide the named clothing.
 */

#include <iostream>
#include <string>
#include <vector>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_GZPS.h"

using namespace std;

bool hideProcess(const char* skinfile, const char* hidefile, int namecount, const char* names[]) {
  DBPFtype skinpackage;
  vector<DBPF_resourceType*> resources;

  // Types that should be decompressed and loaded when opening the file.
  vector<unsigned int> typesToInit;
  typesToInit.push_back(DBPF_GZPS);

  // Open package file and read/populate chosen resources.
  if(!readPackage(skinfile, skinpackage, typesToInit, resources)) {
    cerr << "Opening and reading from " << skinfile << " failed." << endl;
    return false;
  }

  // Clean up
  if (!resources.empty()) {
    size_t vec_size = resources.size();
    for (size_t i = 0; i < vec_size; i++) {
      if (resources[i] != NULL) {
        delete resources[i];
        resources[i] = NULL;
      }
      resources.clear();
    }
  }

  return true;
}
