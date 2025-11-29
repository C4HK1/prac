#pragma once

#include "functionlib/TFunction.h"

class ConstFunction : public TFunction {
public:
    explicit ConstFunction(double value);

private:
    double value_;
};

