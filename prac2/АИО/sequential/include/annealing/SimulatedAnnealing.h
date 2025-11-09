#pragma once

#include <memory>
#include <random>

class ICoolingSchedule;
class IMutation;
class ISolution;

class SimulatedAnnealing {
public:
    SimulatedAnnealing(double initialTemperature, int maxNoImprovementIterations);

    std::unique_ptr<ISolution> optimize(const ISolution& initialSolution,
                                        IMutation& mutation,
                                        const ICoolingSchedule& coolingSchedule,
                                        std::mt19937& rng) const;

private:
    double m_initialTemperature;
    int m_maxNoImprovementIterations;
};

