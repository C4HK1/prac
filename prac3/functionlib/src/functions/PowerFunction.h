#pragma once

#include "functionlib/TFunction.h"

class PowerFunction : public TFunction {
public:
    explicit PowerFunction(double exponent);

private:
    double exponent_;
};

