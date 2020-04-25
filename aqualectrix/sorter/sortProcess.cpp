/*
 * sortProcess.cpp :
 * Changes all sortindexes in the file to the given index.
 */

#include <iostream>
#include <string>
#include <vector>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_BINX.h"

using namespace std;

extern "C" // for exporting to shared library for use in Python
bool sortProcess(const char* filename, const int index) {
  // extra crunchy goodness for restoring state after outputting in hex format
  // ios_base::fmtflags f(cout.flags());
  // clog << endl << "Sorting " << filename << " into index " << hex << index << "..." << endl;
  // cout.flags(f);

  DBPFtype package;
  vector<DBPF_resourceType*> resources;

  // Types that should be decompressed and loaded when opening the file.
  vector<unsigned int> typesToInit;
  typesToInit.push_back(DBPF_BINX);

  // Open package file and read/populate chosen (typesToInit) resources.
  if(!readPackage(filename, package, typesToInit, resources)) {
    cerr << "Opening and reading from " << filename << " failed. Sorting aborted." << endl;
    return false;
  }

  // Set all sortindices
  int item_count = resources.size();
  DBPF_resourceType* pResource = NULL;

  for (int i = 0; i < item_count; i++) {
    pResource = resources[i];

    if (NULL == pResource) {
      continue;
    }

    if (DBPF_BINX == pResource->getType()) {
      if (((DBPF_BINXtype*)pResource)->setSortIndex(index)) {
        // clog << "\t" << "Set BINX resource " << i << "." << endl;
      }
    }
  }

  // clog << "Sorting complete!" << endl;

  // Write back to file
  // clog << endl << "Overwriting file " << filename << "..." << endl;
  bool write_success = writeCompressedPackage(filename, package, resources);
  if (!write_success) {
    cerr << "Writing to file " << filename << " failed. File may be corrupted... " <<
            "or you may have the file open somewhere else (SimPE, maybe?). " <<
            "If so, close the file elsewhere and try again." << endl;
  }
  // else {
  //   clog << "File written!" << endl;
  // }

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

  return write_success;
}
