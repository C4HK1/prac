#include "parallel/ProcessCoordinator.h"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "annealing/SimulatedAnnealing.h"
#include "cooling/CoolingFactory.h"
#include "interfaces/ICoolingSchedule.h"
#include "mutation/SchedulingMutation.h"
#include "solution/SchedulingSolution.h"

namespace {

struct WorkerProcess {
    pid_t pid{};
    int socketFd{};
};

struct WorkerResult {
    double cost{};
    std::vector<int> serializedSolution;
};

// Безопасная запись всех данных в сокет с учётом возможных EINTR.
void writeAll(int fd, const void* data, std::size_t size) {
    const char* buffer = static_cast<const char*>(data);
    std::size_t totalWritten = 0;
    while (totalWritten < size) {
        const auto remaining = size - totalWritten;
        const ssize_t written = ::write(fd, buffer + totalWritten, remaining);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
        }
        if (written == 0) {
            throw std::runtime_error("write returned 0 bytes");
        }
        totalWritten += static_cast<std::size_t>(written);
    }
}

// Безопасное чтение фиксированного объёма данных из сокета.
void readAll(int fd, void* data, std::size_t size) {
    char* buffer = static_cast<char*>(data);
    std::size_t totalRead = 0;
    while (totalRead < size) {
        const auto remaining = size - totalRead;
        const ssize_t received = ::read(fd, buffer + totalRead, remaining);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("read failed: ") + std::strerror(errno));
        }
        if (received == 0) {
            throw std::runtime_error("read returned 0 bytes");
        }
        totalRead += static_cast<std::size_t>(received);
    }
}

std::vector<int32_t> toInt32Vector(const std::vector<int>& input) {
    return {input.begin(), input.end()};
}

std::vector<int> toIntVector(const std::vector<int32_t>& input) {
    return {input.begin(), input.end()};
}

void sendTask(int fd, uint32_t seed, const std::vector<int32_t>& payload) {
    constexpr uint8_t kMessageTask = 1;
    writeAll(fd, &kMessageTask, sizeof(kMessageTask));
    writeAll(fd, &seed, sizeof(seed));
    const uint32_t size = static_cast<uint32_t>(payload.size());
    writeAll(fd, &size, sizeof(size));
    if (!payload.empty()) {
        writeAll(fd, payload.data(), payload.size() * sizeof(int32_t));
    }
}

void sendShutdown(int fd) {
    constexpr uint8_t kMessageShutdown = 0;
    writeAll(fd, &kMessageShutdown, sizeof(kMessageShutdown));
}

WorkerResult receiveResult(int fd) {
    WorkerResult result{};
    uint32_t payloadSize = 0;
    readAll(fd, &result.cost, sizeof(result.cost));
    readAll(fd, &payloadSize, sizeof(payloadSize));
    if (payloadSize > 0) {
        std::vector<int32_t> payload(payloadSize);
        readAll(fd, payload.data(), payload.size() * sizeof(int32_t));
        result.serializedSolution = toIntVector(payload);
    }
    return result;
}

// Функция обрабатывает локальный цикл ИО в дочернем процессе.
void workerLoop(int fd,
                const std::vector<uint32_t>& jobDurations,
                int numProcessors,
                double initialTemperature,
                const std::string& coolingName,
                int maxNoImprovementIterations) {
    try {
        auto coolingSchedule = makeCoolingSchedule(coolingName, initialTemperature);
        SchedulingMutation mutation;
        SimulatedAnnealing annealing(initialTemperature, maxNoImprovementIterations);

        while (true) {
            uint8_t messageType = 0;
            readAll(fd, &messageType, sizeof(messageType));

            if (messageType == 0) {
                break;
            }
            if (messageType != 1) {
                throw std::runtime_error("Worker received unknown message type");
            }

            uint32_t seed = 0;
            uint32_t payloadSize = 0;
            readAll(fd, &seed, sizeof(seed));
            readAll(fd, &payloadSize, sizeof(payloadSize));

            std::vector<int32_t> payload(payloadSize);
            if (payloadSize > 0) {
                readAll(fd, payload.data(), payload.size() * sizeof(int32_t));
            }
            auto serializedSchedule = toIntVector(payload);

            auto initialSolution = SchedulingSolution::deserialize(
                numProcessors, jobDurations, serializedSchedule, seed);

            std::mt19937 acceptanceRng(seed ^ static_cast<unsigned int>(::getpid()) ^ 0xA5A5A5A5u);
            auto bestSolution = annealing.optimize(*initialSolution, mutation, *coolingSchedule, acceptanceRng);

            auto* schedulingSolution = dynamic_cast<SchedulingSolution*>(bestSolution.get());
            if (schedulingSolution == nullptr) {
                throw std::runtime_error("Failed to cast solution to SchedulingSolution");
            }

            auto serializedBest = toInt32Vector(schedulingSolution->serialize());
            const double cost = bestSolution->cost();
            const uint32_t responseSize = static_cast<uint32_t>(serializedBest.size());

            writeAll(fd, &cost, sizeof(cost));
            writeAll(fd, &responseSize, sizeof(responseSize));
            if (!serializedBest.empty()) {
                writeAll(fd, serializedBest.data(), serializedBest.size() * sizeof(int32_t));
            }
        }
    } catch (const std::exception& ex) {
        const std::string message = std::string("Worker error: ") + ex.what() + '\n';
        ::write(STDERR_FILENO, message.c_str(), message.size());
    }

    ::close(fd);
    _exit(0);
}

}  // namespace

ProcessCoordinator::ProcessCoordinator(const std::vector<uint32_t>& jobDurations,
                                       int numProcessors,
                                       std::string coolingName,
                                       double initialTemperature,
                                       int numWorkers)
    : m_jobDurations(jobDurations),
      m_numProcessors(numProcessors),
      m_coolingName(std::move(coolingName)),
      m_initialTemperature(initialTemperature),
      m_numWorkers(numWorkers) {}

double ProcessCoordinator::run() {
    // Основной цикл: создаём процессы, выполняем заданное число глобальных итераций и возвращаем лучшую стоимость.
    ::signal(SIGPIPE, SIG_IGN);

    const auto baseSeed = static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());

    auto globalBest = std::make_unique<SchedulingSolution>(
        m_numProcessors, m_jobDurations, baseSeed);
    double globalBestCost = globalBest->cost();

    std::vector<WorkerProcess> workers;
    workers.reserve(static_cast<std::size_t>(m_numWorkers));

    // Создание пула процессов.
    for (int i = 0; i < m_numWorkers; ++i) {
        int sockets[2];
        if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) < 0) {
            throw std::runtime_error(std::string("socketpair failed: ") + std::strerror(errno));
        }

        const pid_t pid = ::fork();
        if (pid < 0) {
            throw std::runtime_error(std::string("fork failed: ") + std::strerror(errno));
        }

        if (pid == 0) {
            ::close(sockets[0]);
            workerLoop(sockets[1],
                       m_jobDurations,
                       m_numProcessors,
                       m_initialTemperature,
                       m_coolingName,
                       m_localStagnationLimit);
        }

        ::close(sockets[1]);
        workers.push_back(WorkerProcess{pid, sockets[0]});
    }

    std::mt19937 seedRng(baseSeed ^ 0x6C8E9CF5u);
    std::uniform_int_distribution<uint32_t> seedDistribution;

    int stagnationCounter = 0;
    int iteration = 0;

    while (stagnationCounter < m_globalStagnationLimit) {
        const auto serializedBest = toInt32Vector(globalBest->serialize());
        bool improved = false;

        // Отправляем каждому дочернему процессу текущий глобальный лидер.
        for (const auto& worker : workers) {
            const uint32_t seed = seedDistribution(seedRng);
            sendTask(worker.socketFd, seed, serializedBest);
        }

        // Собираем локальные результаты и обновляем глобальное решение.
        for (const auto& worker : workers) {
            const WorkerResult result = receiveResult(worker.socketFd);
            if (!result.serializedSolution.empty() && result.cost < globalBestCost) {
                const uint32_t newSeed = seedDistribution(seedRng);
                auto updatedSolution = SchedulingSolution::deserialize(
                    m_numProcessors, m_jobDurations, result.serializedSolution, newSeed);
                globalBest = std::move(updatedSolution);
                globalBestCost = result.cost;
                improved = true;
            }
        }

        if (improved) {
            stagnationCounter = 0;
        } else {
            ++stagnationCounter;
        }

        std::cout << "Iteration " << iteration++
                  << " best cost: " << globalBestCost
                  << " (stagnation " << stagnationCounter << ")\n";
    }

    for (const auto& worker : workers) {
        sendShutdown(worker.socketFd);
        ::close(worker.socketFd);
    }

        // Дожидаемся завершения всех процессов.
    for (const auto& worker : workers) {
        int status = 0;
        ::waitpid(worker.pid, &status, 0);
    }

    return globalBestCost;
}

