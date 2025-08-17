/*
 * bodyShopReadProcess.cpp :
 * Reads a .package file and returns a subset of the body shop information in it.
 */

#include <cstring>
#include <iostream>
#include <string>
 
#include "../../CatOfEvilGenius/library/DBPF.h"
#include "../../CatOfEvilGenius/library/DBPF_types.h"
#include "../../CatOfEvilGenius/library/DBPF_BINX.h"
#include "../../CatOfEvilGenius/library/DBPF_CPF.h"
#include "../../CatOfEvilGenius/library/DBPF_GZPS.h"
#include "../../CatOfEvilGenius/library/DBPF_STR.h"

// Helper function, not exported to shared library for use in Python
int getColorFromSortindex(int sortindex) {
	// Color lives in hexits yy of 0x0000yy00
	return (sortindex % (16*16*16*16)) / (16*16);
}

// Helper function, not exported to shared library for use in Python
int getColorlessSortindex(int sortindex) {
	int color = getColorFromSortindex(sortindex);
	int colorless_sortindex = sortindex - (color * 16*16);

	return colorless_sortindex;
}

// Helper function, not exported to shared library for use in Python
int addColorToSortindex(int colorless_sortindex, int color) {
	return colorless_sortindex + (color * 16*16);
}

struct resourceInfo {
	unsigned int group;
	unsigned int type;
	unsigned int instance;
	unsigned int instance2;
	const char* key;
	const char* value;
};

extern "C" // for exporting to shared library for use in Python
resourceInfo* readProcess(const char* filename, int* infos_length) {
  DBPFtype package;
  vector<DBPF_resourceType*> resources;

  // Types that should be decompressed and loaded when opening the file.
  vector<unsigned int> typesToInit;
  typesToInit.push_back(DBPF_BINX);
  typesToInit.push_back(DBPF_GZPS);

  // Open package file and read/populate chosen (typesToInit) resources.
  if(!readPackage(filename, package, typesToInit, resources)) {
    cerr << "Opening and reading from " << filename << " failed. Reading aborted." << endl;
    return nullptr;
  }

  // Count the number of GZPS and BINX resources
  int item_count = resources.size();
  DBPF_resourceType* pResource = NULL;
  *infos_length = 0;

  for (int i = 0; i < item_count; i++) {
	  pResource = resources[i];

	  if (NULL == pResource) {
		  continue;
	  }

	  if (DBPF_BINX == pResource->getType()) {
		  // We save one key-value pair from the BINX
		  // Note: *infos_length++ and (*infos_length)++ are not the same!
		  (*infos_length) += 1;
	  }

	  if (DBPF_GZPS == pResource->getType()) {
		  // We save two key-value pairs from the GZPS
		  (*infos_length) += 2;
	  }
  }

  //clog << "Found " << *infos_length << " pieces of data." << endl;

  // Allocate memory to hold infos
  resourceInfo* infos = new resourceInfo[*infos_length];
  int infos_count = 0;

  // Populate infos
  for (int i = 0; i < item_count; i++) {
	  pResource = resources[i];

	  if (NULL == pResource) {
		  continue;
	  }

	  if (DBPF_BINX == pResource->getType()) {
		  DBPF_CPFitemType sortIndex_item;
		  pResource->getPropertyValue("sortindex", sortIndex_item);
		  string sortIndex_string = to_string(sortIndex_item.miValue);
		  char* sortIndex_allocated = new char[sortIndex_string.length() + 1];
		  strcpy(sortIndex_allocated, sortIndex_string.c_str());

		  resourceInfo sortindexInfo = { 
			  .group = pResource->getGroup(),
			  .type = DBPF_BINX,
			  .instance = pResource->getInstance(),
			  .instance2 = pResource->getInstance2(),
			  .key = "sortindex",
			  .value = sortIndex_allocated,
		  };
		  infos[infos_count] = sortindexInfo;
		  infos_count++;

		  //clog << "BINX: sortindex is " << infos[infos_count - 1].value << endl;
	  }

	  if (DBPF_GZPS == pResource->getType()) {
		  DBPF_CPFitemType product_item;
		  pResource->getPropertyValue("product", product_item);
		  string product_string = to_string(product_item.miValue);
		  char* product_allocated = new char[product_string.length() + 1];
		  strcpy(product_allocated, product_string.c_str());

		  resourceInfo productInfo = {
			  .group = pResource->getGroup(),
			  .type = DBPF_GZPS,
			  .instance = pResource->getInstance(),
			  .instance2 = pResource->getInstance2(),
			  .key = "product",
			  .value = product_allocated
		  };
		  infos[infos_count] = productInfo;
		  infos_count++;
		  
		  //clog << "GZPS: product is " << infos[infos_count - 1].value << endl;

		  DBPF_CPFitemType name_item;
		  pResource->getPropertyValue("name", name_item);
		  char* name_allocated = new char[name_item.mstrValue.length() + 1];
		  strcpy(name_allocated, name_item.mstrValue.c_str());

		  resourceInfo nameInfo = {
			  .group = pResource->getGroup(),
			  .type = DBPF_GZPS,
			  .instance = pResource->getInstance(),
			  .instance2 = pResource->getInstance2(),
			  .key = "name",
			  .value = name_allocated
		  };
		  infos[infos_count] = nameInfo;
		  infos_count++;

		  //clog << "GZPS: name is " << infos[infos_count - 1].value << endl;
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

  return infos;
}

struct updateMapPair {
	unsigned int old_val;
	unsigned int new_val;
};

extern "C" // for exporting to shared library for use in Python
bool updateItemProcess(const char* filename, const int group, const int instance, const int instance2, const updateMapPair* color_map, const int color_map_length) {
	// Probably not worth translating any maps to things with faster lookup --
	// I'm operating under the assumption that they're going to be tens of entries,
	// not thousands.

	DBPFtype package;
	vector<DBPF_resourceType*> resources;

	// Types that should be decompressed and loaded when opening the file.
	vector<unsigned int> typesToInit;
	typesToInit.push_back(DBPF_BINX);
	
	// Open package file and read/populate chosen (typesToInit) resources.
	if(!readPackage(filename, package, typesToInit, resources)) {
		cerr << "Opening and reading from " << filename << " failed. Updating aborted." << endl;
		return false;
	}

	int item_count = resources.size();
	DBPF_resourceType* pResource = NULL;
	bool file_updated = false;

	for(int i = 0; i < item_count; i++) {
		pResource = resources[i];

		if (NULL == pResource) {
			continue;
		}

		if (DBPF_BINX == pResource->getType() && group == pResource->getGroup() && instance == pResource->getInstance() && instance2 == pResource->getInstance2()) {
			// Get current sortindex
			DBPF_CPFitemType sortindex;
			pResource->getPropertyValue("sortindex", sortindex);
			int old_sortindex = sortindex.miValue;
			int new_sortindex = old_sortindex;

			// Calculate current color value, which is in hexits 0x0000yy00
			int old_color = getColorFromSortindex(old_sortindex);
			int colorless_sortindex = getColorlessSortindex(old_sortindex);

			//clog << "Sortindex was: " << old_sortindex << endl;
			//clog << "Old color was: " << old_color << endl;
			//clog << "Colorless sortindex is: " << colorless_sortindex << endl;

			// Update desired sortindex value if the old color can be found in the map
			for (int j = 0; j < color_map_length; j++) {
				if(color_map[j].old_val == old_color) {
					new_sortindex = addColorToSortindex(colorless_sortindex, color_map[j].new_val);

					//clog << "Found old color " << color_map[j].old_val << " in map. New color is " << color_map[j].new_val << endl;
					//clog << "New sortindex is: " << new_sortindex << endl;
				}
			}

			// Update actual sortindex value
			if(new_sortindex != old_sortindex) {
				sortindex.miValue = new_sortindex;
				pResource->setPropertyValue("sortindex", sortindex);
				file_updated = true;
			}

		}
	}

	bool write_success = false;
	if (file_updated) {
		write_success = writeCompressedPackage(filename, package, resources);
		if (!write_success) {
			cerr << "Writing to file " << filename << " failed. File may be corrupted... " <<
					"or you may have the file open somewhere else (SimPE, maybe?). " <<
					"If so, close the file elsewhere and try again." << endl;
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

	return file_updated && write_success;
}

extern "C" // for exporting to shared library for use in Python
bool namesyncProcess(const char* filename, const char* creator, const char* setname, const char* colorname, const char* meshname, const char* color_lowercase, const unsigned int product_id, const int sortindex) {

	DBPFtype package;
	vector<DBPF_resourceType*> resources;

	// Types that should be decompressed and loaded when opening the file.
	vector<unsigned int> typesToInit;
	typesToInit.push_back(DBPF_BINX);
	typesToInit.push_back(DBPF_GZPS);
	typesToInit.push_back(DBPF_STR);
	typesToInit.push_back(DBPF_TXMT);
	
	// Open package file and read/populate chosen (typesToInit) resources.
	if(!readPackage(filename, package, typesToInit, resources)) {
		cerr << "Opening and reading from " << filename << " failed. Namesyncing aborted." << endl;
		return false;
	}

  int item_count = resources.size();
  DBPF_resourceType* pResource = NULL;

  for (int i = 0; i < item_count; i++) {
	  pResource = resources[i];

	  if (NULL == pResource) {
		  continue;
	  }

	  if (DBPF_STR == pResource->getType()) {
		  int text_count = ((DBPF_STRtype*)pResource)->getTextItemCount();
		  
		  // Add or assign the first text item to be creator_setname_colorname.
		  DBPF_textType tooltip;
		  tooltip.mLanguageCode = static_cast<char>(1); // English
		  tooltip.mstrValueText = creator + string("_") + setname + string("_") + colorname;

		  if (text_count < 1) {
			  ((DBPF_STRtype*)pResource)->addTextItem(tooltip);
		  }
		  else {
			  ((DBPF_STRtype*)pResource)->setTextItem(0, tooltip);
		  }
	  }

	  if (DBPF_GZPS == pResource->getType()) {
		  // Set product to product_id
		  DBPF_CPFitemType product;
		  product.miType = CPF_INT;
		  product.miValue = product_id;
		  ((DBPF_GZPStype*)pResource)->setPropertyValue("product", product);

		  // Set creator to "00000000-0000-0000-0000-000000000000" to enable custom product id
		  // to influence sorting.
		  DBPF_CPFitemType creator_key;
		  creator_key.miType = CPF_STRING;
		  creator_key.mstrValue = "00000000-0000-0000-0000-000000000000";
		  ((DBPF_GZPStype*)pResource)->setPropertyValue("creator", creator_key);

		  // Set name to meshname_lowercasecolorname
		  DBPF_CPFitemType name;
		  name.miType = CPF_STRING;
		  name.mstrValue = meshname + string("_") + color_lowercase;
		  ((DBPF_GZPStype*)pResource)->setPropertyValue("name", name);
	  }

	  if (DBPF_BINX == pResource->getType()) {		  
		  // Set sortindex with new color and palette
		  DBPF_CPFitemType sortindex_item;
		  sortindex_item.miType = CPF_INT2;
		  sortindex_item.miValue = sortindex;
		  ((DBPF_BINXtype*)pResource)->setPropertyValue("sortindex", sortindex_item);
	  }

	  if (DBPF_TXMT == pResource->getType()) {

	  }
  }

  // Write back to file
  bool write_success = writeCompressedPackage(filename, package, resources);
  if (!write_success) {
    cerr << "Writing to file " << filename << " failed. File may be corrupted... " <<
            "or you may have the file open somewhere else (SimPE, maybe?). " <<
            "If so, close the file elsewhere and try again." << endl;
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

  return write_success;
}

extern "C" // for exporting to shared library for use in Python
void freeResourceInfos(resourceInfo* infos, int infos_length) {
	// .value was new'd; delete it
	for(int i = 0; i < infos_length; i++) {
		delete infos[i].value;
	}
	// delete the whole array
	delete[] infos;
	infos = nullptr;
}
