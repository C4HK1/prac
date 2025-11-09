#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

#include "annealing/SimulatedAnnealing.h"
#include "cooling/CoolingFactory.h"
#include "interfaces/ICoolingSchedule.h"
#include "mutation/SchedulingMutation.h"
#include "solution/SchedulingSolution.h"
#include "util/JobDataLoader.h"

int main(int argc, char* argv[]) {
    // Проверяем количество аргументов командной строки.
    if (argc < 4 || argc > 6) {
        std::cerr << "Usage: " << argv[0]
                  << " <jobs.csv> <num_processors> <cooling_schedule> [initial_temperature] [--negate-cost]\n";
        std::cerr << "Cooling schedules: boltzmann | cauchy | logarithmic\n";
        std::cerr << "  --negate-cost: использовать отрицание критерия К2 для отладки\n";
        return 1;
    }

    const std::string jobsFile = argv[1];
    const int numProcessors = std::stoi(argv[2]);
    const std::string coolingName = argv[3];
    double initialTemperature = 100.0;
    bool negateCost = false;
    
    // Парсим опциональные аргументы
    for (int i = 4; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--negate-cost") {
            negateCost = true;
        } else {
            // Пытаемся интерпретировать как начальную температуру
            try {
                initialTemperature = std::stod(arg);
            } catch (const std::exception&) {
                std::cerr << "Unknown argument: " << arg << '\n';
                return 1;
            }
        }
    }

    try {
        auto jobDurations = loadJobDurationsFromCsv(jobsFile);
        // Гарантируем валидность параметров запуска.
        if (numProcessors <= 0) {
            throw std::invalid_argument("Number of processors must be positive");
        }
        if (initialTemperature <= 0.0) {
            throw std::invalid_argument("Initial temperature must be positive");
        }

        // Создаём конкретный закон охлаждения на основе пользовательского выбора.
        auto coolingSchedule = makeCoolingSchedule(coolingName, initialTemperature);

        const auto seed = static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count());

        const auto startTime = std::chrono::steady_clock::now();

        // Формируем начальное решение и запускаем последовательный ИО.
        SchedulingSolution initialSolution(numProcessors, jobDurations, seed, true, negateCost);
        SchedulingMutation mutation;
        // Для режима отладки (отрицание К2) используем больше итераций без улучшения
        int maxNoImprovement = negateCost ? 10000 : 100;
        SimulatedAnnealing annealing(initialTemperature, maxNoImprovement);

        std::mt19937 acceptanceRng(seed ^ 0x9E3779B9u);
        auto bestSolution = annealing.optimize(initialSolution, mutation, *coolingSchedule, acceptanceRng);

        const auto finishTime = std::chrono::steady_clock::now();
        const double elapsedSeconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(finishTime - startTime).count();

        if (negateCost) {
            std::cout << "Режим отладки: отрицание критерия К2\n";
            std::cout << "Best solution cost (отрицание К2): " << bestSolution->cost() << '\n';
            // Приводим к исходному значению К2 для вывода
            double actualK2 = -bestSolution->cost();
            std::cout << "Фактическое значение К2: " << actualK2 << '\n';
        } else {
            std::cout << "Best solution cost (K2 sum of completion times): " << bestSolution->cost() << '\n';
        }
        std::cout << "Execution time (seconds): " << elapsedSeconds << '\n';
        
        // Выводим детальную информацию о решении
        if (auto* sol = dynamic_cast<SchedulingSolution*>(bestSolution.get())) {
            sol->printDetailedInfo();
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}

