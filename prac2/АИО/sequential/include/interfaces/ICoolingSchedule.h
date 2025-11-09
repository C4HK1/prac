#pragma once

// Интерфейс закона охлаждения: принимает текущую температуру
// (может не использоваться) и номер итерации, возвращает новую температуру.
class ICoolingSchedule {
public:
    virtual ~ICoolingSchedule() = default;
    virtual double cool(double currentTemperature, int iteration) const = 0;
};

