#include "solution/SchedulingSolution.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace {
constexpr int kUnassigned = -1;
}

// Модель решения: списки работ для каждого процессора.
SchedulingSolution::SchedulingSolution(int numProcessors,
                                       std::vector<uint32_t> jobDurations,
                                       unsigned int seed,
                                       bool initializeAssignments)
    : m_numProcessors(numProcessors),
      m_jobDurations(std::move(jobDurations)),
      m_processorJobs(numProcessors),
      m_jobToProcessor(m_jobDurations.size(), kUnassigned),
      m_jobPosition(m_jobDurations.size(), kUnassigned),
      m_rng(seed) {
    if (m_jobDurations.empty()) {
        throw std::invalid_argument("Job durations must not be empty");
    }
    if (numProcessors <= 0) {
        throw std::invalid_argument("Number of processors must be positive");
    }
    if (initializeAssignments) {
        // Выполняем равномерное случайное распределение работ.
        initializeRandomAssignment();
    }
}

double SchedulingSolution::cost() const {
    double totalCompletionTime = 0.0;
    for (const auto& processorQueue : m_processorJobs) {
        uint64_t completionTime = 0;
        for (int jobId : processorQueue) {
            completionTime += m_jobDurations.at(jobId);
            totalCompletionTime += static_cast<double>(completionTime);
        }
    }
    return totalCompletionTime;
}

std::unique_ptr<ISolution> SchedulingSolution::clone() const {
    return std::make_unique<SchedulingSolution>(*this);
}

std::unique_ptr<ISolution> SchedulingSolution::cloneWithNewSeed(unsigned int seed) const {
    auto cloned = std::make_unique<SchedulingSolution>(*this);
    cloned->m_rng.seed(seed);
    return cloned;
}

std::mt19937& SchedulingSolution::rng() {
    return m_rng;
}

int SchedulingSolution::numJobs() const {
    return static_cast<int>(m_jobDurations.size());
}

int SchedulingSolution::numProcessors() const {
    return m_numProcessors;
}

int SchedulingSolution::jobProcessor(int jobId) const {
    if (jobId < 0 || jobId >= numJobs()) {
        throw std::out_of_range("Job id is out of range");
    }
    return m_jobToProcessor.at(jobId);
}

int SchedulingSolution::jobPosition(int jobId) const {
    if (jobId < 0 || jobId >= numJobs()) {
        throw std::out_of_range("Job id is out of range");
    }
    return m_jobPosition.at(jobId);
}

const std::vector<std::vector<int>>& SchedulingSolution::processorAssignments() const {
    return m_processorJobs;
}

void SchedulingSolution::moveJob(int jobId, int targetProcessor, int targetPosition) {
    if (jobId < 0 || jobId >= numJobs()) {
        throw std::out_of_range("Job id is out of range");
    }
    if (targetProcessor < 0 || targetProcessor >= m_numProcessors) {
        throw std::out_of_range("Processor id is out of range");
    }

    int currentProcessor = m_jobToProcessor.at(jobId);
    if (currentProcessor == kUnassigned) {
        throw std::logic_error("Job is not assigned to any processor");
    }

    int currentPosition = m_jobPosition.at(jobId);
    auto& currentQueue = m_processorJobs[currentProcessor];
    auto& targetQueue = m_processorJobs[targetProcessor];

    if (targetPosition < 0 || targetPosition > static_cast<int>(targetQueue.size())) {
        targetPosition = static_cast<int>(targetQueue.size());
    }

    // Удаляем работу из исходного процессора и вставляем в новое место.
    removeJobFromCurrentProcessor(jobId);

    if (currentProcessor == targetProcessor && targetPosition > currentPosition) {
        --targetPosition;
    }

    insertJobIntoProcessor(jobId, targetProcessor, targetPosition);
}

void SchedulingSolution::swapJobs(int firstJobId, int secondJobId) {
    if (firstJobId < 0 || firstJobId >= numJobs() || secondJobId < 0 || secondJobId >= numJobs()) {
        throw std::out_of_range("Job id is out of range");
    }

    if (firstJobId == secondJobId) {
        return;
    }

    int firstProcessor = m_jobToProcessor.at(firstJobId);
    int secondProcessor = m_jobToProcessor.at(secondJobId);

    if (firstProcessor == kUnassigned || secondProcessor == kUnassigned) {
        throw std::logic_error("Some job is not assigned to any processor");
    }

    int firstPosition = m_jobPosition.at(firstJobId);
    int secondPosition = m_jobPosition.at(secondJobId);

    if (firstProcessor == secondProcessor) {
        // Внутренний обмен на одном процессоре.
        auto& queue = m_processorJobs[firstProcessor];
        std::swap(queue[firstPosition], queue[secondPosition]);
        m_jobPosition[queue[firstPosition]] = firstPosition;
        m_jobPosition[queue[secondPosition]] = secondPosition;
    } else {
        auto& firstQueue = m_processorJobs[firstProcessor];
        auto& secondQueue = m_processorJobs[secondProcessor];

        firstQueue[firstPosition] = secondJobId;
        secondQueue[secondPosition] = firstJobId;

        m_jobToProcessor[firstJobId] = secondProcessor;
        m_jobToProcessor[secondJobId] = firstProcessor;
        m_jobPosition[firstJobId] = secondPosition;
        m_jobPosition[secondJobId] = firstPosition;
    }
}

std::vector<int> SchedulingSolution::serialize() const {
    std::vector<int> data;
    data.reserve(static_cast<std::size_t>(numJobs() + m_numProcessors));
    for (const auto& queue : m_processorJobs) {
        data.push_back(static_cast<int>(queue.size()));
        data.insert(data.end(), queue.begin(), queue.end());
    }
    return data;
}

std::unique_ptr<SchedulingSolution> SchedulingSolution::deserialize(int numProcessors,
                                                                    const std::vector<uint32_t>& jobDurations,
                                                                    const std::vector<int>& data,
                                                                    unsigned int seed) {
    auto solution = std::make_unique<SchedulingSolution>(numProcessors, jobDurations, seed, false);

    solution->m_processorJobs.assign(numProcessors, {});
    std::fill(solution->m_jobToProcessor.begin(), solution->m_jobToProcessor.end(), kUnassigned);
    std::fill(solution->m_jobPosition.begin(), solution->m_jobPosition.end(), kUnassigned);

    std::size_t index = 0;
    int totalJobs = 0;

    for (int processor = 0; processor < numProcessors; ++processor) {
        if (index >= data.size()) {
            throw std::invalid_argument("Serialized data is inconsistent");
        }

        int count = data[index++];
        if (count < 0) {
            throw std::invalid_argument("Serialized data contains negative job count");
        }

        auto& queue = solution->m_processorJobs[processor];
        queue.reserve(static_cast<std::size_t>(count));

        for (int i = 0; i < count; ++i) {
            if (index >= data.size()) {
                throw std::invalid_argument("Serialized data is inconsistent");
            }
            int jobId = data[index++];
            if (jobId < 0 || jobId >= static_cast<int>(jobDurations.size())) {
                throw std::invalid_argument("Serialized data contains invalid job id");
            }
            if (solution->m_jobToProcessor[jobId] != kUnassigned) {
                throw std::invalid_argument("Serialized data assigns job more than once");
            }

            queue.push_back(jobId);
            solution->m_jobToProcessor[jobId] = processor;
            solution->m_jobPosition[jobId] = static_cast<int>(queue.size() - 1);
            ++totalJobs;
        }
    }

    if (totalJobs != static_cast<int>(jobDurations.size())) {
        throw std::invalid_argument("Serialized data does not cover all jobs");
    }

    if (index != data.size()) {
        throw std::invalid_argument("Serialized data contains redundant values");
    }

    return solution;
}

void SchedulingSolution::initializeRandomAssignment() {
    std::vector<int> jobIds(numJobs());
    std::iota(jobIds.begin(), jobIds.end(), 0);
    std::shuffle(jobIds.begin(), jobIds.end(), m_rng);

    for (std::size_t index = 0; index < jobIds.size(); ++index) {
        int processor = static_cast<int>(index % static_cast<std::size_t>(m_numProcessors));
        insertJobIntoProcessor(jobIds[index], processor, static_cast<int>(m_processorJobs[processor].size()));
    }
}

void SchedulingSolution::removeJobFromCurrentProcessor(int jobId) {
    int processor = m_jobToProcessor.at(jobId);
    int position = m_jobPosition.at(jobId);

    auto& queue = m_processorJobs[processor];
    queue.erase(queue.begin() + position);
    m_jobToProcessor[jobId] = kUnassigned;
    m_jobPosition[jobId] = kUnassigned;

    updatePositions(processor, position);
}

void SchedulingSolution::insertJobIntoProcessor(int jobId, int processor, int position) {
    if (processor < 0 || processor >= m_numProcessors) {
        throw std::out_of_range("Processor id is out of range");
    }

    auto& queue = m_processorJobs[processor];
    if (position < 0 || position > static_cast<int>(queue.size())) {
        throw std::out_of_range("Position is out of range");
    }

    queue.insert(queue.begin() + position, jobId);
    m_jobToProcessor[jobId] = processor;
    updatePositions(processor, position);
}

void SchedulingSolution::updatePositions(int processor, int startIndex) {
    auto& queue = m_processorJobs[processor];
    for (int index = startIndex; index < static_cast<int>(queue.size()); ++index) {
        m_jobPosition[queue[static_cast<std::size_t>(index)]] = index;
    }
}

