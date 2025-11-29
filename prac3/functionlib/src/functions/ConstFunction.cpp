#include "functions/ConstFunction.h"

#include <sstream>

ConstFunction::ConstFunction(double value)
    : TFunction(
          [value](double) { return value; },
          [](double) { return 0.0; },
          ""),
      value_(value) {
    std::ostringstream oss;
    oss << value_;
    str_ = oss.str();
}

