#pragma once

#include "interfaces/ICoolingSchedule.h"

class LogarithmicCooling final : public ICoolingSchedule {
public:
    explicit LogarithmicCooling(double initialTemperature);

    double cool(double currentTemperature, int iteration) const override;

private:
    double m_initialTemperature;
};

