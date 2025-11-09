#pragma once

#include <memory>
#include <string_view>

class ICoolingSchedule;

std::unique_ptr<ICoolingSchedule> makeCoolingSchedule(std::string_view name, double initialTemperature);

