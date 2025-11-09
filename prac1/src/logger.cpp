#include "mafia/logger.h"

#include <iostream>

// Initialize static round counter
int Logger::current_round_number = 0;

Logger::Logger() {
    logs_directory_path = std::filesystem::current_path() / "logs";
    std::filesystem::create_directory(logs_directory_path);
    clear_old_logs();  // Clear old logs during construction
}

Logger& Logger::get_instance() {
    static Logger singleton_instance;
    return singleton_instance;
}

void Logger::clear_old_logs() {
    try {
        for (const auto& directory_entry : std::filesystem::directory_iterator(logs_directory_path)) {
            if (directory_entry.is_regular_file()) {
                std::filesystem::remove(directory_entry.path());
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
        // Silently ignore filesystem errors
    }
}

void Logger::log_round(std::string_view log_message) {
    std::string round_filename = "round_" + std::to_string(current_round_number) + ".log";
    std::ofstream round_log_file(logs_directory_path / round_filename, std::ios::app);

    round_log_file << log_message << std::endl;
}

void Logger::log_final(std::string_view final_log_message) {
    std::ofstream final_results_file(logs_directory_path / "final_results.log", std::ios::app);

    final_results_file << final_log_message << std::endl;
}
