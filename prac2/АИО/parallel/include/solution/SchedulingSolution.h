#pragma once

#include <cstdint>
#include <memory>
#include <random>
#include <vector>

#include "interfaces/ISolution.h"

class SchedulingSolution final : public ISolution {
public:
    SchedulingSolution(int numProcessors,
                       std::vector<uint32_t> jobDurations,
                       unsigned int seed,
                       bool initializeAssignments = true);

    SchedulingSolution(const SchedulingSolution&) = default;
    SchedulingSolution& operator=(const SchedulingSolution&) = default;

    double cost() const override;

    std::unique_ptr<ISolution> clone() const override;
    std::unique_ptr<ISolution> cloneWithNewSeed(unsigned int seed) const override;

    std::mt19937& rng();

    int numJobs() const;
    int numProcessors() const;

    int jobProcessor(int jobId) const;
    int jobPosition(int jobId) const;

    const std::vector<std::vector<int>>& processorAssignments() const;

    void moveJob(int jobId, int targetProcessor, int targetPosition);
    void swapJobs(int firstJobId, int secondJobId);

    std::vector<int> serialize() const;
    static std::unique_ptr<SchedulingSolution> deserialize(int numProcessors,
                                                           const std::vector<uint32_t>& jobDurations,
                                                           const std::vector<int>& data,
                                                           unsigned int seed);

private:
    void initializeRandomAssignment();
    void removeJobFromCurrentProcessor(int jobId);
    void insertJobIntoProcessor(int jobId, int processor, int position);
    void updatePositions(int processor, int startIndex);

private:
    int m_numProcessors;
    std::vector<uint32_t> m_jobDurations;
    std::vector<std::vector<int>> m_processorJobs;
    std::vector<int> m_jobToProcessor;
    std::vector<int> m_jobPosition;
    mutable std::mt19937 m_rng;
};

