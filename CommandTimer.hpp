// CommandTimer.hpp

#ifndef COMMAND_TIMER_HPP
#define COMMAND_TIMER_HPP

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <iostream>
#include <limits>
#include <ctime>
#include <unistd.h>

#include "StatusDisplay.hpp"

class CommandTimer {
public:
    CommandTimer();
    ~CommandTimer();

    void schedule(int incomingSeconds);
    void cancel();

    float getTimeElapsed() const;
    float getTimeRemaining() const;
    std::string getTimeStartedAsString() const;

private:
    void cancelLocked();
    void runCountdown(int localSeconds);
    void updateDisplay(int localSeconds);

    std::thread timerThread;
    std::atomic<bool> shouldStop;
    std::atomic<bool> active;
    std::atomic<bool> hasFired;
    std::atomic<int> seconds;
    std::atomic<float> timeElapsed;
    std::atomic<float> timeRemaining;
    std::chrono::steady_clock::time_point timeStarted;
    std::chrono::system_clock::time_point timeStartedWallClock;
    std::mutex mutex;
};

// Implementation

inline CommandTimer::CommandTimer()
    : shouldStop(false),
      hasFired(false),
      seconds(0),
      timeElapsed(std::numeric_limits<float>::max()),
      timeRemaining(std::numeric_limits<float>::max()),
      active(false),
      timeStarted() {}

inline CommandTimer::~CommandTimer() {
    cancel();
}

inline void CommandTimer::schedule(int incomingSeconds) {
    std::lock_guard<std::mutex> lock(mutex);

    StatusDisplay::update(
        "Incoming command: " + std::to_string(incomingSeconds) + " seconds",
        "Current state: Active"
    );

	if (hasFired) {
		StatusDisplay::log("Incoming Command at t:" + std::to_string(incomingSeconds) +
						   " accepted, new command buffer!");
	} else if (active && incomingSeconds < timeRemaining.load()) {
		StatusDisplay::log("Incoming Command at t:" + std::to_string(incomingSeconds) +
						   " accepted! New command fires at a shorter timeframe.");
	} else if (active && incomingSeconds >= timeRemaining.load()) {
		StatusDisplay::log("Incoming Command at t:" + std::to_string(incomingSeconds) +
						   " denied! Current command has a shorter timeframe.");
		return;
	} else {
		StatusDisplay::log("Incoming Command at t:" + std::to_string(incomingSeconds) +
						   " accepted as a new command!");
	}

    cancelLocked();

    active = true;
    shouldStop = false;
    hasFired = false;
    seconds = incomingSeconds;
    timeElapsed = std::numeric_limits<float>::max();
    timeRemaining = std::numeric_limits<float>::max();
    timeStarted = std::chrono::steady_clock::now();
    timeStartedWallClock = std::chrono::system_clock::now();

    timerThread = std::thread(&CommandTimer::runCountdown, this, incomingSeconds);
}

inline void CommandTimer::cancel() {
    std::lock_guard<std::mutex> lock(mutex);
    cancelLocked();
}

inline float CommandTimer::getTimeElapsed() const {
    return timeElapsed.load();
}

inline float CommandTimer::getTimeRemaining() const {
    return timeRemaining.load();
}

inline std::string CommandTimer::getTimeStartedAsString() const {
    std::time_t startTimeT = std::chrono::system_clock::to_time_t(timeStartedWallClock);
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", std::localtime(&startTimeT));
    return std::string(buffer);
}

inline void CommandTimer::cancelLocked() {
    if (active) {
        StatusDisplay::update("Command cancelled", "Standing by...");
        shouldStop = true;

        if (timerThread.joinable()) {
            timerThread.join();
        }

        active = false;
        hasFired = false;
    }
}

inline void CommandTimer::runCountdown(int localSeconds) {
    auto deadline = timeStarted + std::chrono::seconds(localSeconds);

    while (std::chrono::steady_clock::now() < deadline) {
        if (shouldStop) return;

        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = now - timeStarted;
        timeElapsed = elapsed.count();
        timeRemaining = static_cast<float>(localSeconds) - timeElapsed.load();

        updateDisplay(localSeconds);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (!shouldStop) {
        StatusDisplay::update("!firing now!", "Awaiting next command...");
        std::cout.flush();
        hasFired = true;
        timeElapsed = std::numeric_limits<float>::max();
        timeRemaining = std::numeric_limits<float>::max();
    }
}

inline void CommandTimer::updateDisplay(int localSeconds) {
    StatusDisplay::update(
        "Countdown started at: " + getTimeStartedAsString(),
        " Elapsed: " + std::to_string(timeElapsed.load()) + "s | Remaining: " + std::to_string(timeRemaining.load()) + "s"
    );
}

#endif // COMMAND_TIMER_HPP
