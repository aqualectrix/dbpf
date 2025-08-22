/*
 * txtrExtractProcess.cpp :
 * Extracts all TXTR references from the given files and saves
 * them to a new file with the given savefile name.
 */

#include <string>
#include <vector>
#include <iostream>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_TXTR.h"

using namespace std;

// Yoinked from CatOfEvilGenius's hairProcess.cpp
void removeNullResources( vector< DBPF_resourceType * > & resources )
{
  if( true == resources.empty() )
    return;

  vector< DBPF_resourceType * > tmp;
  DBPF_resourceType * pResource = NULL;

  int nullCount = 0;
  for( int i = 0; i < (int)resources.size(); ++i )
  {
    pResource = resources[i];
    if( pResource != NULL )
    {
      tmp.push_back( pResource );
    }
    else
      ++nullCount;
  }

  resources.clear();

  for( int i = 0; i < (int)tmp.size(); ++i )
    resources.push_back( tmp[i] );

  clog << "Removed " << nullCount << " TXTR resources." << endl;
}

extern "C" // for exporting to shared library for use in Python
int txtrExtractProcess(const char** filenames, const int num_filenames, const char* savefile_name) {
    vector<DBPF_resourceType*> extractedResources;

    for(int f = 0; f < num_filenames; f++) {
        DBPFtype package;
        vector<DBPF_resourceType*> resources;

        // Types that should be decompressed and loaded when opening the file.
        vector<unsigned int> typesToInit;
        typesToInit.push_back(DBPF_TXTR);

        // Open package file and read/populate chosen (typesToInit) resources.
        if(!readPackage(filenames[f], package, typesToInit, resources)) {
            cerr << "Opening and reading from " << filenames[f] << " failed. Extracting aborted." << endl;
            return false;
        }

        // Find, copy, and null out TXTRs
        int item_count = resources.size();
        DBPF_resourceType* pResource = NULL;

        for(int i = 0; i < item_count; i++) {
            pResource = resources[i];

            if (NULL == pResource) {
                continue;
            }

            if (DBPF_TXTR == pResource->getType()) {
                extractedResources.push_back(new DBPF_TXTRtype(*((DBPF_TXTRtype*)pResource)));

                // We've copied it elsewhere; we don't need this resource anymore.
                delete pResource;
                pResource = NULL;
                resources[i] = NULL;
            }
        }

        removeNullResources(resources);
    }

    return extractedResources.size();
}