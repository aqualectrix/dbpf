/*
 * sorterMain.cpp :
 * Defines the entry point for the console application.
 */

#include <cstdio>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

extern bool sortProcess (const char* filename, const int sortindex);

int main (int argc, char * argv[]) {

  // debugging output
  clog << "argc: " << argc << endl;
  for (int i = 0; i < argc; i++) {
    clog << "argv[" << i << "]: " << argv[i] << endl;
  }

  ostringstream usage;
  usage << endl << "usage: " << argv[0] << " hex_index filename(s)";

  // check parameter count
  if (argc < 3) {
    cerr << usage.str() << endl;
    return 1;
  }

  // check hex_index for validity; each character should be a valid hex digit
  string hex_index = string(argv[1]);
  for (int i = 0; i < hex_index.length(); i++) {
    if (!isxdigit(hex_index[i])) {
      cerr << usage.str() <<
           endl <<
           "\tError: hex_index \"" << hex_index << "\" includes non-hexadecimal characters." <<
           endl <<
           "\tOnly numbers 0-9 and letters a-f or A-F are valid characters." <<
           endl;

      return 1;
    }
  }


  // turn "hex" string into unsigned int
  unsigned int index = stoul(hex_index, NULL, 16);

  // process file(s)
  for (int fi = 2; fi < argc; fi++)
  {
    sortProcess(argv[fi], index);
  }

  return 0;
}
