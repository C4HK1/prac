#include "annealing/SimulatedAnnealing.h"

#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>

#include "interfaces/ICoolingSchedule.h"
#include "interfaces/IMutation.h"
#include "interfaces/ISolution.h"

SimulatedAnnealing::SimulatedAnnealing(double initialTemperature, int maxNoImprovementIterations)
    : m_initialTemperature(initialTemperature),
      m_maxNoImprovementIterations(maxNoImprovementIterations) {
    if (initialTemperature <= 0.0) {
        throw std::invalid_argument("Initial temperature must be positive");
    }
    if (maxNoImprovementIterations <= 0) {
        throw std::invalid_argument("Max iterations without improvement must be positive");
    }
}

// Основной цикл имитации отжига: модифицируем решение и следим за лучшим найденным вариантом.
std::unique_ptr<ISolution> SimulatedAnnealing::optimize(const ISolution& initialSolution,
                                                        IMutation& mutation,
                                                        const ICoolingSchedule& coolingSchedule,
                                                        std::mt19937& rng) const {
    auto bestSolution = initialSolution.clone();
    auto currentSolution = initialSolution.clone();

    double bestCost = bestSolution->cost();
    double currentCost = currentSolution->cost();

    std::uniform_real_distribution<double> acceptanceDistribution(0.0, 1.0);
    double temperature = m_initialTemperature;
    int iteration = 0;
    int iterationsWithoutImprovement = 0;

    while (iterationsWithoutImprovement < m_maxNoImprovementIterations) {
        // Выполняем случайную мутацию текущего решения.
        auto candidateSolution = currentSolution->clone();
        mutation.apply(*candidateSolution);

        const double candidateCost = candidateSolution->cost();
        const double delta = candidateCost - currentCost;

        bool acceptCandidate = delta < 0.0;
        // С вероятностью, зависящей от температуры, принимаем ухудшающее решение.
        if (!acceptCandidate && temperature > std::numeric_limits<double>::min()) {
            const double acceptanceProbability = std::exp(-delta / temperature);
            if (acceptanceProbability > acceptanceDistribution(rng)) {
                acceptCandidate = true;
            }
        }

        if (acceptCandidate) {
            currentSolution = std::move(candidateSolution);
            currentCost = candidateCost;
        }

        if (currentCost < bestCost) {
            // Запоминаем найденное улучшение.
            bestCost = currentCost;
            bestSolution = currentSolution->clone();
            iterationsWithoutImprovement = 0;
        } else {
            ++iterationsWithoutImprovement;
        }

        temperature = coolingSchedule.cool(temperature, iteration);
        if (temperature <= std::numeric_limits<double>::min()) {
            temperature = std::numeric_limits<double>::min();
        }
        ++iteration;
    }

    return bestSolution;
}

