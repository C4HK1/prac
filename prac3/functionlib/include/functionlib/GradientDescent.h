#pragma once

#include "functionlib/TFunction.h"

FUNCTIONLIB_EXPORT double FindRootByGradientDescent(const TFunction& func,
                                                    double initialGuess,
                                                    double learningRate,
                                                    int iterations);

