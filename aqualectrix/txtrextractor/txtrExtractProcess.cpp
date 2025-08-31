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
    extractedResources.clear();

    // Extract TXTR resources and remove them from the original packages
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

        // Find and copy TXTRs
        int item_count = resources.size();
        DBPF_resourceType* pResource = NULL;

        for(int i = 0; i < item_count; i++) {
            pResource = resources[i];

            if (NULL == pResource) {
                continue;
            }

            if (DBPF_TXTR == pResource->getType()) {
                extractedResources.push_back((DBPF_TXTRtype*)pResource);
            }
        }
    }

    // Add TXTR resources to the provided file.
    DBPFtype package;
    vector<DBPF_resourceType*> resources;

    // We don't need to init any resources -- we're not interested in the current contents of
    // the (empty) file.
    vector<unsigned int> typesToInit;

    if (!readPackage(savefile_name, package, typesToInit, resources)) {
        cerr << "Opening and reading from " << savefile_name << " failed. Extracting aborted." << endl;
        return false;
    }

    if (!resources.empty()) {
        cerr << "New file " << savefile_name << " already contains resources. Aborting instead of overwriting." << endl;
        return false;
    }

    // Replace the contents of the file with the extracted resources
    bool write_success = writeCompressedPackage(savefile_name, package, extractedResources);
    if (!write_success) {
        cerr << "Writing to file " << savefile_name << " failed. File may be corrupted... " <<
                "or you may have the file open somewhere else (SimPE, maybe?). " <<
                "If so, close the file elsewhere and try again." << endl;
    }
    else {
        clog << "File written!" << endl;
    }

    return extractedResources.size();
}