#pragma once

#include "interfaces/ICoolingSchedule.h"

class CauchyCooling final : public ICoolingSchedule {
public:
    explicit CauchyCooling(double initialTemperature);

    double cool(double currentTemperature, int iteration) const override;

private:
    double m_initialTemperature;
};

