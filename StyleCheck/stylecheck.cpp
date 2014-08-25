#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>
#include <cstring>
#include <vector>
#include <boost/regex.hpp>
#include <boost/throw_exception.hpp>
#include <algorithm>

#include "issue.hpp"
#include "severity.hpp"
#include "varcheck.hpp"

using namespace std;

const string USAGE_MESSAGE = "usage:\nstylecheck [-h] filename";

namespace boost {
void throw_exception(std::exception const & e) {
  cout << "THINGS BROKE" << endl;
}
}

bool linesort(Issue a, Issue b){
  return a.getLine() < b.getLine();
}


int main(int argc, char** argv) {

  //We need at least 2 arguments to work
  if (argc < 2) {
    cout << USAGE_MESSAGE << endl;
    return 1;
  }

  bool HTMLoutput = false;
  //Parse the arguments
  int i = 0;
  for(; i < argc-1; ++i) {
    if (strcmp(argv[i], "-h") == 0) {
      HTMLoutput = true;
    }
  }

  //Get the filename
  string filename{argv[i]};

  std::vector<Issue> lineIssues;
  std::vector<Issue> fileIssues;

  //Open the file
  fstream fileStream{filename};

  //TODO: Check the style here

  boost::regex hppRegex{"\\.hpp"};

  boost::regex privateRegex{"-private.hpp"};

  //Do all line based regex style stuff
  int lineNo = 1;
  string line;
  while( getline(fileStream, line) ){

    boost::regex derefRegex{"\\( *\\* *(.*?) *\\)\\..*"};

    boost::smatch m;

    if (boost::regex_search(line, m, derefRegex)) {
      lineIssues.push_back(Issue(lineNo, m.position(int(0)),
      "Unneeded Dereference", "It is better to use the -> operator instead of (*).",
       WARNING));
    }

    boost::regex thisRegex{"this->.*"};

    if (boost::regex_search(line, m, thisRegex)) {
      lineIssues.push_back(Issue(lineNo, m.position(int(0)),
      "Unneeded this", "Member functions you don't need to use this",
      WARNING));
    }

    ++lineNo;
  }


  //Check for include guards if it is an hpp
  if (boost::regex_search(filename, hppRegex)){
    //Rewind the file for whole file regex's
    fileStream.clear();
    fileStream.seekg(0, ios::beg);
    string fileContents{istreambuf_iterator<char>{fileStream},
                        istreambuf_iterator<char>{}};


    if(!boost::regex_search(filename, privateRegex)){
      boost::regex incGuardRegex{"#ifndef *([A-Z_]+).*\n#define *([A-Z_]+)"};

      if(!boost::regex_search(fileContents, incGuardRegex)){
        fileIssues.push_back(Issue("Missing include guards",
        "hpp files should contain include guards to prevent double includes",
        ERROR));
      }
    }

    boost::regex usingRegex{"using namespace .*"};

    if(boost::regex_search(fileContents, usingRegex)){
      fileIssues.push_back(Issue("using directive in header",
      "Using directives should not appear in header files",
      ERROR));
    }
  }

  //Use clang to check variable names
  checkVariables(filename, lineIssues);

  //Sort the issues by line number
  sort(lineIssues.begin(), lineIssues.end(), linesort);

  //Produce either terminal or HTML output
  if (!HTMLoutput) {
    for (auto issue:fileIssues) {
      if (issue.getSeverity() == WARNING) {
        cout << "[WARNING]: ";
      }else {
        cout << "[ERROR]: ";
      }
      cout << issue.getTitle() << endl << issue.getMessage() << endl;
    }

    for (auto issue:lineIssues) {
      if (issue.getSeverity() == WARNING) {
        cout << "[WARNING]: (" << issue.getLine() << "," << issue.getColumn() << ") ";
      }else {
        cout << "[ERROR]: (" << issue.getLine() << "," << issue.getColumn() << ") ";
      }
      cout << issue.getTitle() << endl << issue.getMessage() << endl;
    }
  } else {
    for (auto issue:fileIssues) {
      if (issue.getSeverity() == WARNING) {
        cout << "<h3><span style=\"color:yellow\">[WARNING]</span>: ";
      }else {
        cout << "<h3><span style=\"color:red\">[ERROR]</span>: ";
      }
      cout << issue.getTitle() << "</h3>"<< endl;
      cout << "<pre>" << issue.getMessage() << "</pre>" << endl;
    }

    for (auto issue:lineIssues) {
      if (issue.getSeverity() == WARNING) {
        cout << "<h3><span style=\"color:yellow\">[WARNING]</span>: (";
        cout << issue.getLine() << "," << issue.getColumn() << ") ";
      }else {
        cout << "<h3><span style=\"color:red\">[ERROR]</span>: (";
        cout << issue.getLine() << "," << issue.getColumn() << ") ";
      }
      cout << issue.getTitle() << "</h3>"<< endl;
      cout << "<pre>" << issue.getMessage() << "</pre>" << endl;
    }
  }

  return 0;
}
