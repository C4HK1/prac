#include "functionlib/GradientDescent.h"

double FindRootByGradientDescent(const TFunction& func,
                                 double initialGuess,
                                 double learningRate,
                                 int iterations) {
    double x = initialGuess;
    for (int i = 0; i < iterations; ++i) {
        double value = func(x);
        double gradient = value * func.GetDeriv(x);
        x -= learningRate * gradient;
    }
    return x;
}

