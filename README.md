# A Countdown-based Propulsion Controller
---
## 🚀 What It Does

- TCP server listens on a specified port (default: `8124`)
- Accepts delayed fire commands (`int` values from client)
- Schedules a **countdown timer** to fire in `X` seconds
- New command **overrides** the previous if:
  - It’s shorter in duration
  - The previous one hasn’t fired yet
- Fires once countdown hits zero
- Rejects slower commands if a faster one is already running
- Supports cancellation with `-1` command
---
## 🧱 Components
### `main.cpp`
- Parses optional CLI port override
- Initializes `CommandTimer` and `TCPServer`
- Routes TCP input to the timer system
### `TCPServer.hpp`
- Listens on TCP port 8124 (or overridden)
- Accepts a single client
- Reads newline-separated integer commands
- Valid inputs:
  - `> 0`: delay in seconds
  - `-1`: cancel
  - invalid input: logged and ignored
### `CommandTimer.hpp`
- Manages countdown logic
- Thread-safe
- Tracks:
  - `seconds`: original command delay
  - `timeElapsed`: time passed since scheduling
  - `timeRemaining`: time left to fire
  - `timeStarted`: internal clock reference
  - `timeStartedWallClock`: human-readable HH:MM:SS
- Accepts override logic if new command is faster
- Emits `"firing now!"` upon execution
### `StatusDisplay.hpp`
- ANSI-based terminal UI
- Uses `\033` escape codes to:
  - Clear screen
  - Render real-time status
  - Scroll a Twitch-style fading log
- Supports:
  - `update(line1, line2)` — for persistent status
  - `log(message)` — shows last 6 logs, each fades after 5 seconds
---
## 🧰 Prerequisites
If you're using **WSL2 / Ubuntu 20.04+**, install the following to compile and run this project:
### Install Build Tools
```bash
sudo apt-get update && \
sudo apt-get install -y build-essential g++ make netcat
```
This installs:
- `g++` – C++ compiler
- `make` – to build with the Makefile
- `netcat` – to simulate TCP client input for testing
---
## 🧪 Usage
### Build
```bash
make
```
### Run
```bash
./propulsion_server         # Uses default port 8124
./propulsion_server 9000   # Override to port 9000
```
### Test via netcat
```bash
nc localhost 8124
15    # schedule fire in 15s
10    # override with a faster command
-1    # cancel pending command
```
---

