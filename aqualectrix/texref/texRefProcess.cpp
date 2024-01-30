/*
 * texRefProcess.cpp :
 * Changes TXMTs with the given subset to point to the given texture ID. 
 * TXTRs with the given subset are removed.
 */

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_TXMT.h"
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
bool texRefProcess(const char* filename, const char* texId, const char* subsetToReplace, bool replaceBumpmap) {
  clog << "Texture referencing " << filename << " to the " << texId << " reference." << endl;

  DBPFtype package;
  vector<DBPF_resourceType*> resources;

  // Types that should be decompressed and loaded when opening the file.
  vector<unsigned int> typesToInit;
  typesToInit.push_back(DBPF_TXMT);
  typesToInit.push_back(DBPF_TXTR);

  // Open package file and read/populate chosen (typesToInit) resources.
  if(!readPackage(filename, package, typesToInit, resources)) {
    cerr << "Opening and reading from " << filename << " failed. Referencing aborted." << endl;
    return false;
  }

  // Set TXMTs to point to referenced textures.
  int item_count = resources.size();
  DBPF_resourceType* pResource = NULL;

  for (int i = 0; i < item_count; i++) {
    pResource = resources[i];

    if (NULL == pResource) {
      continue;
    }

    // Point TXMTs at reference textures
    if (DBPF_TXMT == pResource->getType()) {
      string subsetName;
      if (((DBPF_TXMTtype*)pResource)->getSubsetName(subsetName)) {
          clog << "\t" << "Found subset " << subsetName << " in TXMT." << endl;

          if (0 == subsetName.compare(subsetToReplace)) {
              // Replace "Base" texture
              clog << "\t\t" << "Replacing with " << texId << "." << endl;
              string texIdStr = texId;
              string referencedTextureName = "##0x" + texIdStr + "!" + subsetName + "~stdMatBaseTextureName";
              clog << "\t\t" << "New value is " << referencedTextureName << endl;
              ((DBPF_TXMTtype*)pResource)->setPropertyValue("stdMatBaseTextureName", referencedTextureName);

              // If bumpmap exists and we should replace it, do so
              if (replaceBumpmap) {
                  string bumpmapName;
                  if (((DBPF_TXMTtype*)pResource)->getPropertyValue("stdMatNormalMapTextureName", bumpmapName)) {          
                      clog << "\t\t" << "Bumpmap exists, replacing it too." << endl;
                      string referencedBumpMapTextureName = "##0x" + texIdStr + "!" + subsetName + "~stdMatNormalMapTextureName";
                      clog << "\t\t" << "New value is " << referencedBumpMapTextureName << endl;
                      ((DBPF_TXMTtype*)pResource)->setPropertyValue("stdMatNormalMapTextureName", referencedBumpMapTextureName);
                  }
              }
          }
      } else {
          cerr << filename << " is missing the stdMatBaseTextureName property for the given subset in at least one of its TXMTs. Skipping texture referencing.";
          return false;
      }
    }

    // Null out TXTRs that we'll no longer be using
    if (DBPF_TXTR == pResource->getType()) {
        string subsetName = ((DBPF_TXTRtype*)pResource)->getSubsetName();
        clog << "\t" << "Found subset " << subsetName << " in TXTR." << endl;

        if (0 == subsetName.compare(subsetToReplace)) {
            string textureType = ((DBPF_TXTRtype*)pResource)->getTextureType();

            if (0 == textureType.compare("Base") || ( replaceBumpmap && 0 == textureType.compare("NormalMap") )) {
                clog << "\t\t" << "Found stdMat" << textureType << "TextureName TXTR with subset " << subsetName << ", which will no longer be needed once the corresponding TXMT is texture referenced. Nulling the TXTR." << endl;
                delete pResource;
                pResource = NULL;
                resources[i] = NULL;
            }
        }
    }
  }

  clog << "Referencing complete!" << endl;

  // Remove nulled-out resources
  removeNullResources(resources);

  // Write back to file
  clog << endl << "Overwriting file " << filename << "..." << endl;
  bool write_success = writeCompressedPackage(filename, package, resources);
  if (!write_success) {
    cerr << "Writing to file " << filename << " failed. File may be corrupted... " <<
            "or you may have the file open somewhere else (SimPE, maybe?). " <<
            "If so, close the file elsewhere and try again." << endl;
  }
  else {
    clog << "File written!" << endl;
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
