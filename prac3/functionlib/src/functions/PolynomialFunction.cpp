#include "functions/PolynomialFunction.h"

#include <cmath>
#include <sstream>

PolynomialFunction::PolynomialFunction(const std::vector<double>& coefficients)
    : TFunction(
          [coefficients](double x) {
              double result = 0.0;
              double x_pow = 1.0;
              for (double coef : coefficients) {
                  result += coef * x_pow;
                  x_pow *= x;
              }
              return result;
          },
          [coefficients](double x) {
              if (coefficients.size() <= 1) {
                  return 0.0;
              }
              double result = 0.0;
              double x_pow = 1.0;
              for (size_t i = 1; i < coefficients.size(); ++i) {
                  result += static_cast<double>(i) * coefficients[i] * x_pow;
                  x_pow *= x;
              }
              return result;
          },
          ""),
      coefficients_(coefficients) {
    std::ostringstream oss;
    bool firstTerm = true;
    for (size_t i = 0; i < coefficients_.size(); ++i) {
        double coef = coefficients_[i];
        if (coef == 0.0) {
            continue;
        }

        if (!firstTerm) {
            oss << (coef >= 0.0 ? " + " : " - ");
        } else if (coef < 0.0) {
            oss << "-";
        }

        double absCoef = std::abs(coef);
        if (i == 0) {
            oss << absCoef;
        } else {
            if (absCoef != 1.0) {
                oss << absCoef << "*";
            }
            oss << "x";
            if (i > 1) {
                oss << "^" << i;
            }
        }

        firstTerm = false;
    }

    if (firstTerm) {
        oss << "0";
    }

    str_ = oss.str();
}

