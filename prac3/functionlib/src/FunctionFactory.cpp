#include "functionlib/FunctionFactory.h"

#include "functions/ConstFunction.h"
#include "functions/ExpFunction.h"
#include "functions/IdentFunction.h"
#include "functions/PolynomialFunction.h"
#include "functions/PowerFunction.h"

#include <stdexcept>

TFunctionPtr FunctionFactory::Create(const std::string& type) const {
    if (type == "ident") {
        return std::make_shared<IdentFunction>();
    }
    if (type == "exp") {
        return std::make_shared<ExpFunction>();
    }
    throw std::invalid_argument("Unsupported function type: " + type);
}

TFunctionPtr FunctionFactory::Create(const std::string& type, double param) const {
    if (type == "const") {
        return std::make_shared<ConstFunction>(param);
    }
    if (type == "power") {
        return std::make_shared<PowerFunction>(param);
    }
    throw std::invalid_argument("Unsupported function type for double parameter: " + type);
}

TFunctionPtr FunctionFactory::Create(const std::string& type, const std::vector<double>& params) const {
    if (type == "polynomial") {
        return std::make_shared<PolynomialFunction>(params);
    }
    throw std::invalid_argument("Unsupported function type for vector parameter: " + type);
}

