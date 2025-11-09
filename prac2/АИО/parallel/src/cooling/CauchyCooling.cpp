#include "cooling/CauchyCooling.h"

#include <stdexcept>

CauchyCooling::CauchyCooling(double initialTemperature)
    : m_initialTemperature(initialTemperature) {
    if (initialTemperature <= 0.0) {
        throw std::invalid_argument("Initial temperature must be positive");
    }
}

double CauchyCooling::cool(double /*currentTemperature*/, int iteration) const {
    // Температура убывает по закону Коши: обратно пропорционально номеру итерации.
    return m_initialTemperature / static_cast<double>(iteration + 1);
}

