#include "mutation/SchedulingMutation.h"

#include <random>
#include <stdexcept>

#include "solution/SchedulingSolution.h"

// Оператор мутации случайно перемещает работу или меняет две работы местами.
void SchedulingMutation::apply(ISolution& solution) {
    auto* schedulingSolution = dynamic_cast<SchedulingSolution*>(&solution);
    if (schedulingSolution == nullptr) {
        throw std::invalid_argument("SchedulingMutation can only be applied to SchedulingSolution");
    }

    auto& rng = schedulingSolution->rng();
    std::uniform_real_distribution<double> probability(0.0, 1.0);

    if (probability(rng) < 0.5) {
        relocateJob(*schedulingSolution);
    } else {
        swapRandomJobs(*schedulingSolution);
    }
}

void SchedulingMutation::relocateJob(SchedulingSolution& solution) {
    auto& rng = solution.rng();
    std::uniform_int_distribution<int> jobDistribution(0, solution.numJobs() - 1);
    std::uniform_int_distribution<int> processorDistribution(0, solution.numProcessors() - 1);

    const int jobId = jobDistribution(rng);
    const int targetProcessor = processorDistribution(rng);
    const auto& assignments = solution.processorAssignments();
    const int targetSize = static_cast<int>(assignments[targetProcessor].size());
    std::uniform_int_distribution<int> positionDistribution(0, targetSize);

    solution.moveJob(jobId, targetProcessor, positionDistribution(rng));
}

void SchedulingMutation::swapRandomJobs(SchedulingSolution& solution) {
    if (solution.numJobs() < 2) {
        return;
    }

    auto& rng = solution.rng();
    std::uniform_int_distribution<int> jobDistribution(0, solution.numJobs() - 1);

    int firstJob = jobDistribution(rng);
    int secondJob = jobDistribution(rng);
    int attempts = 0;
    while (secondJob == firstJob && attempts < 5) {
        secondJob = jobDistribution(rng);
        ++attempts;
    }
    if (firstJob == secondJob) {
        return;
    }

    solution.swapJobs(firstJob, secondJob);
}

