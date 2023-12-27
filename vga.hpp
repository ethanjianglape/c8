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
#include <cstdint>

#include <SFML/Graphics.hpp>

namespace c8::vga
{
    inline constexpr std::uint8_t frameBufferWidth = 64;
    inline constexpr std::uint8_t frameBufferHeight = 32;

    inline constexpr int hostPixelSize = 16;

    inline constexpr int hostRenderWidth = frameBufferWidth * hostPixelSize;
    inline constexpr int hostRenderHeight = frameBufferHeight * hostPixelSize;

    class VgaState
    {
    private:
        bool frameBuffer[frameBufferHeight][frameBufferWidth];

    public:
        bool drawByte(std::uint8_t x, std::uint8_t y, const std::uint8_t byte);

        void clear();

        void render(sf::RenderTexture& texture) const;
    };
}
