/*
 * getTexIdProcess.cpp :
 * Reads the first TXMT it finds to get the id-prefix for textures in the file.
 */

 #define _DEBUG

#include <iostream>
#include <string>
#include <vector>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_TXMT.h"

using namespace std;

extern "C" // for exporting to shared library for use in Python
const char* getTexIdProcess(const char* filename) {
  clog << "Reading " << filename << "..." << endl;

  DBPFtype package;
  vector<DBPF_resourceType*> resources;

  // Types that should be decompressed and loaded when opening the file.
  vector<unsigned int> typesToInit;
  typesToInit.push_back(DBPF_TXMT);

  // Open package file and read/populate chosen (typesToInit) resources.
  if(!readPackage(filename, package, typesToInit, resources)) {
    cerr << "Opening and reading from " << filename << " failed. Reference gathering aborted." << endl;
    return "";
  }

  int item_count = resources.size();
  DBPF_resourceType* pResource = NULL;
  string txtName;

  for (int i = 0; i < item_count; i++) {
    pResource = resources[i];

    if (NULL == pResource) {
      continue;
    }

    if (DBPF_TXMT == pResource->getType()) {
      if (((DBPF_TXMTtype*)pResource)->getPropertyValue("stdMatBaseTextureName", txtName)) {
          //clog << "\t" << "Found stdMatBaseTextureName: " << txtName;
          break; // The prefix is typically the same in each TXMT in a file, so we only need the one name.
      }
    }
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

   return txtName.substr(4, 8).c_str(); // from ##0xabcdef01!etc, keep abcdef01
}
