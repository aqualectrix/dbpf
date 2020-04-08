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

extern bool sortProcess (const char* filename, string sortindex);

int main (int argc, char * argv[]) {

  // debugging output
  clog << "argc: " << argc << endl;
  for (int i = 0; i < argc; i++) {
    clog << "argv[" << i << "]: " << argv[i] << endl;
  }

  ostringstream usage;
  usage << endl << "usage: " << argv[0] << " filename hex_index";

  // check parameter count
  if (argc != 3) {
    cerr << usage.str() << endl;
    return 1;
  }

  // check hex_index for validity; each character should be a valid hex digit
  string hex_index = string(argv[2]);
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

  // process file
  sortProcess(argv[1], hex_index);

  return 0;
}
