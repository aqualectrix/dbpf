/*
 * sortProcess.cpp :
 * Changes all sortindexes in the file to the given index.
 */

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_BINX.h"
#include "../../CatOfEvilGenius/library/DBPF_GZPS.h"

using namespace std;

extern "C" // for exporting to shared library for use in Python
bool sortProcess(const char* filename, const int index, const bool geneticize_hair) {
  // extra crunchy goodness for restoring state after outputting in hex format
  // ios_base::fmtflags f(cout.flags());
  // clog << endl << "Sorting " << filename << " into index " << hex << index << "..." << endl;
  // cout.flags(f);

  DBPFtype package;
  vector<DBPF_resourceType*> resources;

  // Types that should be decompressed and loaded when opening the file.
  vector<unsigned int> typesToInit;
  typesToInit.push_back(DBPF_BINX);
  typesToInit.push_back(DBPF_GZPS);

  // Open package file and read/populate chosen (typesToInit) resources.
  if(!readPackage(filename, package, typesToInit, resources)) {
    cerr << "Opening and reading from " << filename << " failed. Sorting aborted." << endl;
    return false;
  }

  // Create index-based genetic hairtone (with hex representation of index)
  string hairtone;
  // Create 0-left-padded at-least-length-8 string with hex representation of index
  // (note that setw does not truncate)
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(8) << std::hex << index;
  string index_str = stream.str();
  // Abort if we need to use the string but it is too long
  if (geneticize_hair && index_str.length() > 8) {
      cerr << "The given sortindex " << index << " is too long for this program to use in a genetic hairtone. Please use a sortindex with no more than 8 characters." << endl;
      return false;
  }
  hairtone = index_str + "-4000-0000-0000-000000000000";


  // Set all sortindices (and possibly hairtones)
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

    if (geneticize_hair && DBPF_GZPS == pResource->getType()) {
        // Only set hairtone if it already exists.
        DBPF_CPFitemType item;
        if (((DBPF_GZPStype*)pResource)->getPropertyValue("hairtone", item)) {
            clog << "\t" << "Found hairtone, setting to " + hairtone + "." << endl;
            ((DBPF_GZPStype*)pResource)->setHairtone(hairtone);
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
