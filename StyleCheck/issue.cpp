#include "issue.hpp"

using namespace std;

Issue::Issue(int line, int col, string title, string message,
              severity_t severity):
  line_{line},
  column_{col},
  title_{title},
  message_{message},
  severity_{severity}
{
  //Nothing to do
}

int Issue::getLine() const
{
  return (*this).line_;
}

int Issue::getColumn() const
{
  return column_;
}

std::string Issue::getTitle() const
{
  return title_;
}

std::string Issue::getMessage() const
{
  return message_;
}

severity_t Issue::getSeverity() const
{
  return severity_;
}
