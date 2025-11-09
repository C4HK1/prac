#pragma once

#include "interfaces/ICoolingSchedule.h"

class BoltzmannCooling final : public ICoolingSchedule {
public:
    explicit BoltzmannCooling(double initialTemperature);

    double cool(double currentTemperature, int iteration) const override;

private:
    double m_initialTemperature;
};

