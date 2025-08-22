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
        cout << filenames[f] << endl;
    }

    return extractedResources.size();
}