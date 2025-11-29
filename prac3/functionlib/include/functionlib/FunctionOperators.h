#pragma once

#include "functionlib/TFunction.h"

#include <stdexcept>
#include <type_traits>

FUNCTIONLIB_EXPORT TFunction operator+(const TFunction& lhs, const TFunction& rhs);
FUNCTIONLIB_EXPORT TFunction operator-(const TFunction& lhs, const TFunction& rhs);
FUNCTIONLIB_EXPORT TFunction operator*(const TFunction& lhs, const TFunction& rhs);
FUNCTIONLIB_EXPORT TFunction operator/(const TFunction& lhs, const TFunction& rhs);

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator+(const TFunction&, const T&) {
    throw std::logic_error("Invalid operand type for operator+");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator+(const T&, const TFunction&) {
    throw std::logic_error("Invalid operand type for operator+");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator-(const TFunction&, const T&) {
    throw std::logic_error("Invalid operand type for operator-");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator-(const T&, const TFunction&) {
    throw std::logic_error("Invalid operand type for operator-");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator*(const TFunction&, const T&) {
    throw std::logic_error("Invalid operand type for operator*");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator*(const T&, const TFunction&) {
    throw std::logic_error("Invalid operand type for operator*");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator/(const TFunction&, const T&) {
    throw std::logic_error("Invalid operand type for operator/");
}

template <typename T>
std::enable_if_t<!std::is_base_of_v<TFunction, T>, TFunction>
operator/(const T&, const TFunction&) {
    throw std::logic_error("Invalid operand type for operator/");
}

