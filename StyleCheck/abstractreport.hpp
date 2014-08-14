#ifndef ABSTRACT_REPORT_HPP
#define ABSTRACT_REPORT_HPP

#include <iostream>

class AbstractReport {
public:
  virtual std::ostream& produceReport(std::ostream& out) = delete;
}

#endif
