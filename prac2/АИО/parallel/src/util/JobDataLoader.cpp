#include "util/JobDataLoader.h"

#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

std::vector<uint32_t> loadJobDurationsFromCsv(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open jobs file: " + filename);
    }

    std::vector<uint32_t> durations;
    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (firstLine) {
            firstLine = false;
            if (!line.empty() && !std::isdigit(static_cast<unsigned char>(line.front()))) {
                continue;
            }
        }

        std::stringstream ss(line);
        std::string jobId;
        std::string durationStr;

        if (!std::getline(ss, jobId, ',')) {
            throw std::runtime_error("Invalid CSV row: " + line);
        }
        if (!std::getline(ss, durationStr, ',')) {
            throw std::runtime_error("Invalid CSV row: " + line);
        }

        int jobIndex = static_cast<int>(durations.size());
        auto underscore = jobId.find_last_of('_');
        if (underscore != std::string::npos) {
            try {
                jobIndex = std::stoi(jobId.substr(underscore + 1)) - 1;
            } catch (const std::exception&) {
                jobIndex = static_cast<int>(durations.size());
            }
        }

        if (jobIndex < 0) {
            jobIndex = static_cast<int>(durations.size());
        }
        if (static_cast<int>(durations.size()) <= jobIndex) {
            durations.resize(static_cast<std::size_t>(jobIndex + 1), 0);
        }
        durations[static_cast<std::size_t>(jobIndex)] = static_cast<uint32_t>(std::stoul(durationStr));
    }

    if (durations.empty()) {
        throw std::runtime_error("No job durations found in file: " + filename);
    }

    return durations;
}

