#include "functionlib/FunctionOperators.h"

#include <string>

TFunction operator+(const TFunction& lhs, const TFunction& rhs) {
    auto func = [lhs, rhs](double x) { return lhs(x) + rhs(x); };
    auto deriv = [lhs, rhs](double x) { return lhs.GetDeriv(x) + rhs.GetDeriv(x); };
    std::string repr = "(" + lhs.ToString() + " + " + rhs.ToString() + ")";
    return TFunction(func, deriv, repr);
}

TFunction operator-(const TFunction& lhs, const TFunction& rhs) {
    auto func = [lhs, rhs](double x) { return lhs(x) - rhs(x); };
    auto deriv = [lhs, rhs](double x) { return lhs.GetDeriv(x) - rhs.GetDeriv(x); };
    std::string repr = "(" + lhs.ToString() + " - " + rhs.ToString() + ")";
    return TFunction(func, deriv, repr);
}

TFunction operator*(const TFunction& lhs, const TFunction& rhs) {
    auto func = [lhs, rhs](double x) { return lhs(x) * rhs(x); };
    auto deriv = [lhs, rhs](double x) {
        return lhs.GetDeriv(x) * rhs(x) + lhs(x) * rhs.GetDeriv(x);
    };
    std::string repr = "(" + lhs.ToString() + " * " + rhs.ToString() + ")";
    return TFunction(func, deriv, repr);
}

TFunction operator/(const TFunction& lhs, const TFunction& rhs) {
    auto func = [lhs, rhs](double x) { return lhs(x) / rhs(x); };
    auto deriv = [lhs, rhs](double x) {
        double numerator = lhs.GetDeriv(x) * rhs(x) - lhs(x) * rhs.GetDeriv(x);
        double denominator = rhs(x) * rhs(x);
        return numerator / denominator;
    };
    std::string repr = "(" + lhs.ToString() + " / " + rhs.ToString() + ")";
    return TFunction(func, deriv, repr);
}

