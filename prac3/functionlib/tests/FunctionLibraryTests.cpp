#include <gtest/gtest.h>

#include <cmath>

#include "functionlib/FunctionFactory.h"
#include "functionlib/FunctionOperators.h"
#include "functionlib/GradientDescent.h"

TEST(FunctionFactoryTest, CreatesBasicFunctions) {
    FunctionFactory factory;
    auto ident = factory.Create("ident");
    EXPECT_DOUBLE_EQ((*ident)(3.5), 3.5);
    EXPECT_DOUBLE_EQ(ident->GetDeriv(100.0), 1.0);
    EXPECT_EQ(ident->ToString(), "x");

    auto constant = factory.Create("const", -2.0);
    EXPECT_DOUBLE_EQ((*constant)(42.0), -2.0);
    EXPECT_DOUBLE_EQ(constant->GetDeriv(5.0), 0.0);

    auto power = factory.Create("power", 2.0);
    EXPECT_DOUBLE_EQ((*power)(3.0), 9.0);
    EXPECT_DOUBLE_EQ(power->GetDeriv(3.0), 6.0);

    auto exponential = factory.Create("exp");
    EXPECT_DOUBLE_EQ((*exponential)(0.0), 1.0);
    EXPECT_DOUBLE_EQ(exponential->GetDeriv(1.0), std::exp(1.0));

    auto polynomial = factory.Create("polynomial", std::vector<double>{1.0, 0.0, 3.0});
    EXPECT_DOUBLE_EQ((*polynomial)(2.0), 13.0);
    EXPECT_DOUBLE_EQ(polynomial->GetDeriv(2.0), 12.0);
}

TEST(FunctionFactoryTest, ThrowsOnUnknownType) {
    FunctionFactory factory;
    EXPECT_THROW(factory.Create("unknown"), std::invalid_argument);
    EXPECT_THROW(factory.Create("unknown", 1.0), std::invalid_argument);
    EXPECT_THROW(factory.Create("unknown", std::vector<double>{}), std::invalid_argument);
}

TEST(OperatorsTest, PerformsArithmeticAndDerivatives) {
    FunctionFactory factory;
    auto quadratic = factory.Create("power", 2.0);
    auto cubic = factory.Create("power", 3.0);

    auto sum = *quadratic + *cubic;
    EXPECT_DOUBLE_EQ(sum(2.0), 8.0 + 4.0);
    EXPECT_DOUBLE_EQ(sum.GetDeriv(2.0), 16.0);

    auto product = *quadratic * *cubic;  // x^5
    EXPECT_DOUBLE_EQ(product(2.0), std::pow(2.0, 5.0));
    EXPECT_DOUBLE_EQ(product.GetDeriv(2.0), 5.0 * std::pow(2.0, 4.0));

    auto division = *cubic / *quadratic;  // x
    EXPECT_DOUBLE_EQ(division(4.0), 4.0);
    EXPECT_DOUBLE_EQ(division.GetDeriv(4.0), 1.0);
}

TEST(OperatorsTest, RejectsUnsupportedTypes) {
    FunctionFactory factory;
    auto quadratic = factory.Create("power", 2.0);
    EXPECT_THROW(*quadratic + std::string("bad"), std::logic_error);
    EXPECT_THROW(std::string("bad") - *quadratic, std::logic_error);
    EXPECT_THROW(*quadratic * 10, std::logic_error);
    EXPECT_THROW(10 / *quadratic, std::logic_error);
}

TEST(FunctionDerivativeTest, HandlesCompositeExpressions) {
    FunctionFactory factory;
    auto poly = factory.Create("polynomial", std::vector<double>{-4.0, 0.0, 1.0});  // x^2 - 4
    auto expFunc = factory.Create("exp");

    auto expression = *poly + *expFunc;  // x^2 - 4 + exp(x)
    double x = 1.5;
    double expectedDerivative = poly->GetDeriv(x) + expFunc->GetDeriv(x);
    EXPECT_DOUBLE_EQ(expression.GetDeriv(x), expectedDerivative);
}

TEST(GradientDescentTest, FindsRootForQuadratic) {
    FunctionFactory factory;
    auto poly = factory.Create("polynomial", std::vector<double>{-4.0, 0.0, 1.0});  // x^2 - 4

    double root = FindRootByGradientDescent(*poly, 0.5, 0.05, 300);
    EXPECT_NEAR(root, 2.0, 1e-2);
}

