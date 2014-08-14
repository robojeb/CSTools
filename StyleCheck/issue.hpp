#ifndef ISSUE_HPP_INC
#define ISSUE_HPP_INC

#include "severity.hpp"

class Issue{
public:
  Issue(int line, int col, std::string title,
        std::string message, severity_t severity);

  Issue(std::string title, std::string message, severity_t severity);

  int getLine() const;

  int getColumn() const;

  std::string getTitle() const;

  std::string getMessage() const;

  severity_t getSeverity() const;

private:
  int line_;
  int column_;
  std::string title_;
  std::string message_;
  severity_t severity_;
};

#endif
