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

#include <chrono>
#include <SFML/Graphics.hpp>
#include "vga.hpp"

namespace c8::config
{
    inline constexpr int targetHostFps = 60;
    inline constexpr auto targetHostFpsRatio = std::chrono::duration<int, std::ratio<1, targetHostFps>>{1};

    inline constexpr int targetCpuFrequency = 500;
    inline constexpr double targetCpuCyclesPerFrame = targetCpuFrequency / targetHostFps;

    inline constexpr bool showEmulatorInfo = true;

    inline constexpr int pixelWidth = 16;
    inline constexpr int pixelHeight = 16;

    inline const sf::Color backgroundColor = sf::Color::Black;
    inline const sf::Color pixelColor = sf::Color::Green;

    inline unsigned int getRenderWidth()
    {
        return c8::vga::frameBufferWidth * pixelWidth;
    }

    inline unsigned int getRenderHeight()
    {
        return c8::vga::frameBufferHeight * pixelHeight;
    }
}