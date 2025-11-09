#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Класс управляет запуском дочерних процессов имитации отжига
// и обменом данных через UNIX-сокеты.
class ProcessCoordinator {
public:
    ProcessCoordinator(const std::vector<uint32_t>& jobDurations,
                       int numProcessors,
                       std::string coolingName,
                       double initialTemperature,
                       int numWorkers);

    // Запускает полный цикл параллельного алгоритма и возвращает стоимость найденного решения.
    double run();

private:
    const std::vector<uint32_t>& m_jobDurations;
    int m_numProcessors;
    std::string m_coolingName;
    double m_initialTemperature;
    int m_numWorkers;

    int m_localStagnationLimit = 50;  // Уменьшено для быстрого тестирования
    int m_globalStagnationLimit = 5;  // Уменьшено для быстрого тестирования
};

