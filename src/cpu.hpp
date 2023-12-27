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

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <bitset>
#include <iostream>
#include <random>
#include <stack>
#include <ratio>
#include <sstream>
#include <iomanip>
#include <cstdint>

#include <SFML/Graphics.hpp>

#include "memory.hpp"
#include "vga.hpp"
#include "quirks.hpp"

namespace c8::cpu
{
    void initialize();

    void setCpuFrequency(int hz);

    void setFps(int fps);

    void destroy();

    void reset();

    void keyboardKeyPressed(std::uint8_t value);

    std::uint16_t getProgramCounter();

    void togglePaused();

    void advanceOneClockCycle();

    void backOneClockCylce();

    void decrementTimers();

    void draw(sf::RenderTexture& vgaTexture, sf::RenderTexture& cpuInfoTexture);

    std::uint8_t* getRegister(const std::uint8_t index);

    void processOpcode(const std::uint16_t opcode);

    void executeClockCycle();

    void decrementTimers();
}
