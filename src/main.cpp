/**
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>

#include <SFML/Graphics.hpp>

#include "cpu.hpp"
#include "memory.hpp"
#include "vga.hpp"
#include "config.hpp"
#include "ui.hpp"

void processArgs(int argc, char** argv)
{
    if (argc <= 1) {
        return;
    }

    std::vector<std::string> args;

    args.assign(argv + 1, argv + argc);

    for (const auto& arg : args) {
        std::ifstream file{arg};

        if (file.is_open()) {
            c8::mem::loadProgram(file);
        }

        file.close();
    }
}

void loop()
{
    using clock = std::chrono::high_resolution_clock;

    int clockCycles = 0;
    int frames = 0;

    auto lastFpsUpdate = clock::now();

    while (c8::ui::isOpen()) {
        const auto start = clock::now();

        c8::ui::pollInput();

        if (clockCycles < c8::config::targetCpuFrequency) {
            auto cyclesThisFrame = c8::config::targetCpuCyclesPerFrame;

            if (frames == c8::config::targetHostFps - 1) {
                cyclesThisFrame = c8::config::targetCpuFrequency - clockCycles;
            }

            for (int cycles = 0; cycles < cyclesThisFrame; cycles++) {
                c8::cpu::executeClockCycle();

                clockCycles++;
            }
        }

        c8::cpu::decrementTimers();
        c8::ui::draw();

        frames++;

        if (start - lastFpsUpdate >= std::chrono::seconds(1) || frames == c8::config::targetHostFps) {
            c8::cpu::setCpuFrequency(clockCycles);
            c8::cpu::setFps(frames);

            clockCycles = 0;
            frames = 0;
            lastFpsUpdate = clock::now();
        }

        const auto end = clock::now();

        if (end - start < c8::config::targetHostFpsRatio) {
            std::this_thread::sleep_until(start + c8::config::targetHostFpsRatio);
        }
    }
}

int main(int argc, char** argv)
{
    c8::ui::initialize();
    c8::mem::initialize();
    c8::cpu::initialize();

    processArgs(argc, argv);

    loop();

    c8::cpu::destroy();
}