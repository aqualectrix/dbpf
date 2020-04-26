#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
  clog << "argc: " << argc << endl;
  for (int ii = 0; ii < argc; ii++) {
    clog << "argv[" << ii << "]: " << argv[ii] << endl;
  }

  // incorrect parameters
  if (argc < 4) {
    cerr << "usage: " << argv[0] << " skinfile hidefile name" << endl;
    return 1;
  }

  return 0;
}
