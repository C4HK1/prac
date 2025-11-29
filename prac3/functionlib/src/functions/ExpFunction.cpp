#include "functions/ExpFunction.h"

#include <cmath>

ExpFunction::ExpFunction()
    : TFunction(
          [](double x) { return std::exp(x); },
          [](double x) { return std::exp(x); },
          "exp(x)") {}

