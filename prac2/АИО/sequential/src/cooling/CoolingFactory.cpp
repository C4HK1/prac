#include "cooling/CoolingFactory.h"

#include <stdexcept>
#include <string>

#include "cooling/BoltzmannCooling.h"
#include "cooling/CauchyCooling.h"
#include "cooling/LogarithmicCooling.h"

std::unique_ptr<ICoolingSchedule> makeCoolingSchedule(std::string_view name, double initialTemperature) {
    if (name == "boltzmann") {
        return std::make_unique<BoltzmannCooling>(initialTemperature);
    }
    if (name == "cauchy") {
        return std::make_unique<CauchyCooling>(initialTemperature);
    }
    if (name == "logarithmic") {
        return std::make_unique<LogarithmicCooling>(initialTemperature);
    }
    throw std::invalid_argument("Unknown cooling schedule: " + std::string{name});
}

