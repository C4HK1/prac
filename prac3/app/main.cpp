#include <iostream>
#include <vector>

#include "functionlib/FunctionFactory.h"
#include "functionlib/FunctionOperators.h"
#include "functionlib/TFunction.h"

int main() {
    FunctionFactory funcFactory;
    std::vector<TFunctionPtr> cont;

    auto f = funcFactory.Create("power", 2);  // PowerFunc x^2
    cont.push_back(f);

    auto g = funcFactory.Create("polynomial", std::vector<double>{7, 0, 3, 15});  // 7 + 3*x^2 + 15*x^3
    cont.push_back(g);

    for (const auto& ptr : cont) {
        std::cout << ptr->ToString() << " for x = 10 is " << (*ptr)(10) << std::endl;
    }

    auto p = *f + *g;
    std::cout << "p'(1) = " << p.GetDeriv(1) << std::endl;

    try {
        auto h = *f + std::string("abc");  // std::logic_error
        (void)h;
    } catch (const std::logic_error& ex) {
        std::cout << "Caught expected error: " << ex.what() << std::endl;
    }

    std::cout << "p'(3) = " << p.GetDeriv(3) << std::endl;
    return 0;
}

