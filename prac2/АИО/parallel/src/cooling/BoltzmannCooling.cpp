#include "cooling/BoltzmannCooling.h"

#include <cmath>
#include <stdexcept>

BoltzmannCooling::BoltzmannCooling(double initialTemperature)
    : m_initialTemperature(initialTemperature) {
    if (initialTemperature <= 0.0) {
        throw std::invalid_argument("Initial temperature must be positive");
    }
}

double BoltzmannCooling::cool(double /*currentTemperature*/, int iteration) const {
    // Используем логарифмический закон Больцмана, зависящий только от номера итерации.
    const auto safeIteration = static_cast<double>(iteration + 2);
    return m_initialTemperature / std::log(safeIteration);
}

