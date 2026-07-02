# CHIP-8 Emulator

A high-performance, lightweight CHIP-8 Interpreter written from scratch in C++ and powered by SDL3.

Instead of a traditional, bulky switch-case CPU loop, this emulator utilizes a highly optimized two-tier function pointer table architecture for rapid opcode decoding, coupled with precise hardware timer emulation.

## Features-

- Function Pointer Decoding Engine: Uses dynamic method-lookup tables (table[0x10]) to achieve near-instantaneous instruction routing and execution.

- Decoupled 60Hz Hardware Timers: Emulates the original hardware behavior by decoupling the Delay and Sound timers from the core CPU clock cycles.

- SDL3 Graphics Pipeline: Direct streaming texture mapping for crisp pixel rendering with modern layout architectures.

- Persisted Input Handling: Decoupled event loop state management to prevent input drops and handle continuous button holding seamlessly.

## Tech Stack & Dependencies-

- Language: C++20

- Multimedia API: SDL3 (Simple DirectMedia Layer 3)

- Build Tools: CMake / Standard g++ Compiler

## How to Build and Run

1. Prerequisites- 
Ensure you have the latest SDL3 runtime and development libraries installed on your system.

2. Compilation-
You can compile the project using a standard C++ compiler. Navigate to your project directory and execute:

`g++ main.cpp -o main.exe -I(SDL include location)\x86_64-w64-mingw32\include -L(SDL lib location)\x86_64-w64-mingw32\lib -lSDL3`

3. Execution-
Launch the executable and pass your favorite CHIP-8 ROM as an argument(For example):

`./main 10 3 Tetris.ch8`

## Controls Configuration-

| Original Keypad | Keyboard Key | Default Tetris Map |
| -------------------- | -------------------- |
| 4 | Q | 🔄 Rotate Piece |
| 5 | W | ⬅️ Move Left |
| 6 | E | ➡️ Move Right |
| 0 | X | Secondary Actions |
| 1 - 3 | 1 - 3 | System Inputs |
| — | ESC | 🛑 Safe Exit / Quit |

## Architecture Overview

### Opcode Processing Cycle

1. Fetch: Read a 2-byte opcode from memory using the Program Counter (PC).

2. Decode & Execute: Masks the nibbles and invokes the respective function straight out of the `table` array:
`((*this).*(table[(opcode & 0xF000u) >> 12u]))();`

3. Sync: Updates internal hardware clocks at 60Hz utilizing high-resolution system timestamps.

