#pragma once

#include "functionlib/TFunction.h"

#include <string>
#include <vector>

class FUNCTIONLIB_EXPORT FunctionFactory {
public:
    TFunctionPtr Create(const std::string& type) const;
    TFunctionPtr Create(const std::string& type, double param) const;
    TFunctionPtr Create(const std::string& type, const std::vector<double>& params) const;
};

