#pragma once

// Интерфейс закона охлаждения: возвращает новое значение температуры
// на основе текущей температуры (может быть неиспользуемой) и номера итерации.
class ICoolingSchedule {
public:
    virtual ~ICoolingSchedule() = default;
    virtual double cool(double currentTemperature, int iteration) const = 0;
};

