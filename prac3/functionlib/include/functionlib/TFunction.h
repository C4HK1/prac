#pragma once

#include "functionlib/functionlib_export.h"

#include <functional>
#include <memory>
#include <string>

class FUNCTIONLIB_EXPORT TFunction {
public:
    using FuncType = std::function<double(double)>;

    TFunction() = default;
    TFunction(FuncType func, FuncType deriv, std::string representation);
    virtual ~TFunction() = default;

    double operator()(double x) const;
    double GetDeriv(double x) const;
    virtual std::string ToString() const;

protected:
    FuncType func_;
    FuncType deriv_;
    std::string str_;
};

using TFunctionPtr = std::shared_ptr<TFunction>;
