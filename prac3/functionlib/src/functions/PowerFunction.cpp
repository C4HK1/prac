#include "functions/PowerFunction.h"

#include <cmath>
#include <sstream>

PowerFunction::PowerFunction(double exponent)
    : TFunction(
          [exponent](double x) {
              if (exponent == 0.0) {
                  return 1.0;
              }
              return std::pow(x, exponent);
          },
          [exponent](double x) {
              if (exponent == 0.0) {
                  return 0.0;
              }
              if (exponent == 1.0) {
                  return 1.0;
              }
              return exponent * std::pow(x, exponent - 1.0);
          },
          ""),
      exponent_(exponent) {
    std::ostringstream oss;
    if (exponent_ == 0.0) {
        str_ = "1";
    } else if (exponent_ == 1.0) {
        str_ = "x";
    } else {
        oss << "x^" << exponent_;
        str_ = oss.str();
    }
}

