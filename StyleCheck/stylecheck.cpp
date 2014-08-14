#include <iostream>
#include <string>

using namespace std;

const string USAGE_MESSAGE = "usage:\nstylecheck [-h] filename";

int main(int argc, char** argv) {

  //We need at least 2 arguments to work
  if (argc < 2) {
    cout << USAGE_MESSAGE << endl;
    return 1;
  }

  bool HTMLoutput = false;
  //Parse the arguments
  for(int i = 0; i < argc; ++i) {
    if (argc[i] == "-h") {
      HTMLoutput = true;
    }
  }

  //TODO: Check the style here

  //TODO: Produce output based on the issues we found

  return 0;
}
