#include "cooling/LogarithmicCooling.h"

#include <cmath>
#include <stdexcept>

LogarithmicCooling::LogarithmicCooling(double initialTemperature)
    : m_initialTemperature(initialTemperature) {
    if (initialTemperature <= 0.0) {
        throw std::invalid_argument("Initial temperature must be positive");
    }
}

double LogarithmicCooling::cool(double /*currentTemperature*/, int iteration) const {
    // Возвращаем температуру по логарифмическому закону с плавным спадом.
    const double numerator = std::log(static_cast<double>(iteration + 2));
    const double denominator = static_cast<double>(iteration + 1);
    return m_initialTemperature * numerator / denominator;
}

