#ifndef TERMINAL_REPORT_HPP_INC
#define TERMINAL_REPORT_HPP_INC

#include <iostream>
#include "abstractreport.hpp"
#include "checkfile.hpp"

class TerminalReport: public AbstractReport {
public:
  TerminalReport(CheckFile& file);

  virtual std::ostream& produceReport(std::ostream& out);

private:
  CheckFile& file;
}

#endif
