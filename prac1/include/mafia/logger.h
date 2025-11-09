#pragma once

#include <filesystem>
#include <fstream>
#include <string_view>

class Logger {
    std::filesystem::path logs_directory_path;
    static int current_round_number;  // Static round counter

    Logger();  // Private constructor that clears old logs

public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    static Logger& get_instance();

    static int get_current_round() {
        return current_round_number;
    }

    static void increment_round() {
        ++current_round_number;
    }

    static void reset_round() {
        current_round_number = 0;
    }

    void log_round(std::string_view log_message);
    void log_final(std::string_view final_log_message);

private:
    void clear_old_logs();  // Now private, called from constructor
};
