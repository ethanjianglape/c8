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

#include <cstring>

#include "vga.hpp"
#include "cpu.hpp"
#include "config.hpp"

namespace c8::vga
{
    void VgaState::clear()
    {
        for (std::uint8_t y = 0; y < frameBufferHeight; y++) {
            for (std::uint8_t x = 0; x < frameBufferWidth; x++) {
                frameBuffer[y][x] = false;
            }
        }
    }

    bool VgaState::drawByte(
        std::uint8_t x, 
        std::uint8_t y,
        const std::uint8_t byte)
    {
        if (x >= frameBufferWidth) {
            x = 0;
        }

        if (y >= frameBufferHeight) {
            y = 0;
        }

        bool didErase = false;

        const bool originalBits[8] = {
            frameBuffer[y][x + 0],
            frameBuffer[y][x + 1],
            frameBuffer[y][x + 2],
            frameBuffer[y][x + 3],
            frameBuffer[y][x + 4],
            frameBuffer[y][x + 5],
            frameBuffer[y][x + 6],
            frameBuffer[y][x + 7],
        };

        frameBuffer[y][x + 0] ^= ((byte & 0b1000'0000) > 0);
        frameBuffer[y][x + 1] ^= ((byte & 0b0100'0000) > 0);
        frameBuffer[y][x + 2] ^= ((byte & 0b0010'0000) > 0);
        frameBuffer[y][x + 3] ^= ((byte & 0b0001'0000) > 0);
        frameBuffer[y][x + 4] ^= ((byte & 0b0000'1000) > 0);
        frameBuffer[y][x + 5] ^= ((byte & 0b0000'0100) > 0);
        frameBuffer[y][x + 6] ^= ((byte & 0b0000'0010) > 0);
        frameBuffer[y][x + 7] ^= ((byte & 0b0000'0001) > 0);

        const bool newBits[8] = {
            frameBuffer[y][x + 0],
            frameBuffer[y][x + 1],
            frameBuffer[y][x + 2],
            frameBuffer[y][x + 3],
            frameBuffer[y][x + 4],
            frameBuffer[y][x + 5],
            frameBuffer[y][x + 6],
            frameBuffer[y][x + 7],
        };

        for (int i = 0; i < 8; i++) {
            const bool originalBit = originalBits[i];
            const bool newBit = newBits[i];

            if (originalBit && !newBit) {
                didErase = true;
            }
        }

        return didErase;
    }

    void VgaState::render(sf::RenderTexture& texture) const
    {
        for (std::uint8_t y = 0; y < frameBufferHeight; y++) {
            for (std::uint8_t x = 0; x < frameBufferWidth; x++) {
                const bool bit = frameBuffer[y][x];

                const int hostX = x * c8::config::pixelWidth;
                const int hostY = y * c8::config::pixelHeight;

                if (bit) {
                    sf::RectangleShape rec{sf::Vector2f{c8::config::pixelWidth, c8::config::pixelHeight}};

                    rec.setFillColor(c8::config::pixelColor);
                    rec.setPosition(hostX, hostY);

                    texture.draw(rec);
                }
            }
        }
    }
}