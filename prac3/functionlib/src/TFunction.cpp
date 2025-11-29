#include "functionlib/TFunction.h"

#include <stdexcept>

TFunction::TFunction(FuncType func, FuncType deriv, std::string representation)
    : func_(std::move(func)), deriv_(std::move(deriv)), str_(std::move(representation)) {}

double TFunction::operator()(double x) const {
    if (!func_) {
        throw std::logic_error("Function is not defined");
    }
    return func_(x);
}

double TFunction::GetDeriv(double x) const {
    if (!deriv_) {
        throw std::logic_error("Derivative is not defined");
    }
    return deriv_(x);
}

std::string TFunction::ToString() const {
    return str_;
}

