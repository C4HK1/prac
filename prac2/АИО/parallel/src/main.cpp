#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

#include "parallel/ProcessCoordinator.h"
#include "util/JobDataLoader.h"

int main(int argc, char* argv[]) {
    // Проверяем корректность входных параметров запуска.
    if (argc < 5 || argc > 6) {
        std::cerr << "Usage: " << argv[0]
                  << " <jobs.csv> <num_processors> <cooling_schedule> <num_workers> [initial_temperature]\n";
        std::cerr << "Cooling schedules: boltzmann | cauchy | logarithmic\n";
        return 1;
    }

    const std::string jobsFile = argv[1];
    const int numProcessors = std::stoi(argv[2]);
    const std::string coolingName = argv[3];
    const int numWorkers = std::stoi(argv[4]);
    const double initialTemperature = (argc == 6) ? std::stod(argv[5]) : 100.0;

    if (numProcessors <= 0) {
        std::cerr << "Number of processors must be positive\n";
        return 1;
    }
    if (numWorkers <= 0) {
        std::cerr << "Number of workers must be positive\n";
        return 1;
    }
    if (initialTemperature <= 0.0) {
        std::cerr << "Initial temperature must be positive\n";
        return 1;
    }

    try {
        // Загружаем длительности работ и передаём их координатору процессов.
        auto jobDurations = loadJobDurationsFromCsv(jobsFile);

        ProcessCoordinator coordinator(jobDurations,
                                       numProcessors,
                                       coolingName,
                                       initialTemperature,
                                       numWorkers);

        const auto startTime = std::chrono::steady_clock::now();
        const double finalCost = coordinator.run();
        const auto finishTime = std::chrono::steady_clock::now();
        const double elapsedSeconds =
            std::chrono::duration_cast<std::chrono::duration<double>>(finishTime - startTime).count();

        std::cout << "Final best cost (K2 sum of completion times): " << finalCost << '\n';
        std::cout << "Execution time (seconds): " << elapsedSeconds << '\n';
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}

