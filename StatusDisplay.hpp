// StatusDisplay.hpp

#ifndef STATUS_DISPLAY_HPP
#define STATUS_DISPLAY_HPP

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

class StatusDisplay {
public:
    static void update(const std::string& line1, const std::string& line2 = "") {
        currentMainLine = line1;
        currentSubLine = line2;
        render();
    }

    static void log(const std::string& message) {
        auto now = std::chrono::steady_clock::now();
        logBuffer.push_back({message, now});
        if (logBuffer.size() > MAX_LOG_LINES)
            logBuffer.erase(logBuffer.begin());
        render();
    }

private:
    struct LogEntry {
        std::string text;
        std::chrono::steady_clock::time_point timestamp;
    };

    static inline std::vector<LogEntry> logBuffer;
    static inline std::string currentMainLine = "";
    static inline std::string currentSubLine = "";
    static constexpr size_t MAX_LOG_LINES = 6;

    static void render() {
        std::cout << "\033[2J\033[H"; // Clear screen + move cursor to top
        std::cout << "=== Propulsion System Status ===\n";
        std::cout << "Time: " << getTimestamp() << "\n";
        std::cout << currentMainLine << "\n";
        if (!currentSubLine.empty())
            std::cout << currentSubLine << "\n";
        std::cout << "\n-- 💬 Live Log --\n";

        auto now = std::chrono::steady_clock::now();
        for (const auto& entry : logBuffer) {
            float age = std::chrono::duration<float>(now - entry.timestamp).count();
            if (age < 5.0f) {
                std::cout << entry.text << "\n";
            }
        }
        std::cout.flush();
    }

    static std::string getTimestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm* now_tm = std::localtime(&now_time);

        std::ostringstream oss;
        oss << std::put_time(now_tm, "%H:%M:%S");
        return oss.str();
    }
};

#endif // STATUS_DISPLAY_HPP