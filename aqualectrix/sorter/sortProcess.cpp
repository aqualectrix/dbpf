/*
 * sortProcess.cpp :
 * Changes all sortindexes in the file to the given index.
 */

#include <iostream>
#include <string>
#include <vector>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"

using namespace std;

bool sortProcess(const char* filename, const string hex_index) {
 clog << "Sorting " << filename << " into index 0x" << hex_index << "..." << endl;

 DBPFtype package;
 vector<DBPF_resourceType*> resources;

 // Types that should be decompressed and loaded when opening the file.
 vector<unsigned int> typesToInit;
 typesToInit.push_back(DBPF_BINX);

 // Open package file and read/populate chosen (typesToInit) resources.
 if(!readPackage(filename, package, typesToInit, resources)) {
   clog << "Opening and reading from " << filename << " failed. Sorting aborted." << endl;
   return false;
 }

 clog << "Sorting complete!";
 return true;
}
