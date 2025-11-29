#include "functions/IdentFunction.h"

IdentFunction::IdentFunction()
    : TFunction(
          [](double x) { return x; },
          [](double) { return 1.0; },
          "x") {}

