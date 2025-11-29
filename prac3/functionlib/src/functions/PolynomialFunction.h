#pragma once

#include "functionlib/TFunction.h"

#include <vector>

class PolynomialFunction : public TFunction {
public:
    explicit PolynomialFunction(const std::vector<double>& coefficients);

private:
    std::vector<double> coefficients_;
};

