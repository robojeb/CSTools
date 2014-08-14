#ifndef CHECKFILE_HPP_INC
#define CHECKFILE_HPP_INC

#include <string>

class CheckFile{
public:
  CheckFile(std::string fileName);

  addLineIssue(int line, int col,
                std::string title, std::string message, severity_t severity);

  addFileIssue(std::string title, std::string message, severity_t severity);
};

#endif
