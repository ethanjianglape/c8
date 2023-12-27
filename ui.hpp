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

#include <unordered_map>
#include <sstream>
#include <string>

#include <SFML/Graphics.hpp>

namespace c8::ui
{
    inline const std::unordered_map<sf::Keyboard::Key, std::uint8_t> valueByKey = {
        {sf::Keyboard::Key::Num0, 0x0}, {sf::Keyboard::Key::Numpad0, 0x0},
        {sf::Keyboard::Key::Num1, 0x1}, {sf::Keyboard::Key::Numpad1, 0x1},
        {sf::Keyboard::Key::Num2, 0x2}, {sf::Keyboard::Key::Numpad2, 0x2},
        {sf::Keyboard::Key::Num3, 0x3}, {sf::Keyboard::Key::Numpad3, 0x3},
        {sf::Keyboard::Key::Num4, 0x4}, {sf::Keyboard::Key::Numpad4, 0x4},
        {sf::Keyboard::Key::Num5, 0x5}, {sf::Keyboard::Key::Numpad5, 0x5},
        {sf::Keyboard::Key::Num6, 0x6}, {sf::Keyboard::Key::Numpad6, 0x6},
        {sf::Keyboard::Key::Num7, 0x7}, {sf::Keyboard::Key::Numpad7, 0x7},
        {sf::Keyboard::Key::Num8, 0x8}, {sf::Keyboard::Key::Numpad8, 0x8},
        {sf::Keyboard::Key::Num9, 0x9}, {sf::Keyboard::Key::Numpad9, 0x9},
        {sf::Keyboard::Key::A, 0xA},
        {sf::Keyboard::Key::B, 0xB},
        {sf::Keyboard::Key::C, 0xC},
        {sf::Keyboard::Key::D, 0xD},
        {sf::Keyboard::Key::E, 0xE},
        {sf::Keyboard::Key::F, 0xF},
    };

    void initialize();

    bool isOpen();

    void pollInput();

    void draw();

    bool isOpen();

    void drawText(
        sf::RenderTexture& texture, 
        const int x, 
        const int y, 
        const std::stringstream& ss
    );

    void drawText(
        sf::RenderTexture& texture, 
        const int x, 
        const int y, 
        std::string text
    );

    class Hex
    {
    private:
        const int value;
        const int hexDigits;
        const int decDigits;

        const bool includeDec;
    public:
        Hex(std::uint8_t value, bool includeDec = true);

        Hex(std::uint16_t value, bool includeDec = true);

        friend std::ostream& operator<<(std::ostream& ss, const Hex& hex);
    };
}