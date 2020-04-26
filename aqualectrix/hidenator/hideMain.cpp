#include <iostream>
#include <string>
#include <vector>

using namespace std;

extern bool hideProcess( const char* skinfile, const char* hidefile, vector<string> names);

int main(int argc, char* argv[]) {
  clog << "argc: " << argc << endl;
  for (int ii = 0; ii < argc; ++ii) {
    clog << "argv[" << ii << "]: " << argv[ii] << endl;
  }

  // incorrect parameters
  if (argc < 4) {
    cerr << "usage: " << argv[0] << " skinfile hidefile name" << endl;
    return 1;
  }

  // vectorize names
  vector<string> names;
  for (int jj = 3; jj < argc; ++jj) {
    names.push_back(argv[jj]);
  }

  return hideProcess(argv[1], argv[2], names);
}
