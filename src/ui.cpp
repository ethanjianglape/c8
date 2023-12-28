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

#include <memory>

#include <SFML/Graphics.hpp>

#include "ui.hpp"
#include "cpu.hpp"
#include "vga.hpp"
#include "fonts.hpp"

namespace c8::ui
{
    std::unique_ptr<sf::RenderWindow> window;

    sf::Font font;

    void initialize()
    {
        font.loadFromMemory(&c8::fonts::courierFontData, c8::fonts::courierFontDataLength);

        sf::VideoMode vm{c8::vga::hostRenderWidth, 1000, sf::VideoMode::getDesktopMode().bitsPerPixel};
        window = std::make_unique<sf::RenderWindow>(vm, "C8");
    }

    void processKeyPressed(const sf::Event& event)
    {
        const sf::Keyboard::Key key = event.key.code;

        if (key == sf::Keyboard::Key::P) { 
            c8::cpu::togglePaused();
            return;
        }

        if (key == sf::Keyboard::Key::Right) {
            c8::cpu::advanceOneClockCycle();
            return;
        }

        if (key == sf::Keyboard::Key::Left) {
            c8::cpu::backOneClockCylce();
            return;
        }

        if (valueByKey.contains(key)) {
            std::uint8_t value = valueByKey.at(key);
            c8::cpu::keyboardKeyPressed(value);
            return;
        }
    }

    void pollInput()
    {
        sf::Event event;

        while (window->pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window->close();
                break;
            }

            if (event.type == sf::Event::KeyPressed) {
                processKeyPressed(event);
                break;
            }

            if (event.type == sf::Event::Resized) {
                sf::View view = window->getDefaultView();

                view.setSize({
                    (float)event.size.width,
                    (float)event.size.height
                });

                window->setView(view);

                break;
            }
        }
    }

    void draw()
    {
        window->clear(sf::Color::Black);

        sf::RenderTexture vgaTexture;
        sf::RenderTexture cpuInfoTexture;
        sf::RenderTexture memoryTexture;

        vgaTexture.create(c8::vga::hostRenderWidth, c8::vga::hostRenderHeight);
        cpuInfoTexture.create(500, 500);
        memoryTexture.create(500, 500);

        vgaTexture.clear(sf::Color::Black);
        cpuInfoTexture.clear(sf::Color::Black);
        memoryTexture.clear(sf::Color::Black);

        c8::cpu::render(vgaTexture, cpuInfoTexture);
        c8::mem::render(memoryTexture);

        vgaTexture.display();
        cpuInfoTexture.display();
        memoryTexture.display();

        sf::Sprite vgaSprite{vgaTexture.getTexture()};
        sf::Sprite cpuInfoSprite{cpuInfoTexture.getTexture()};
        sf::Sprite memorySprite{memoryTexture.getTexture()};

        vgaSprite.setPosition(0, 0);
        cpuInfoSprite.setPosition(500, c8::vga::hostRenderHeight + 10);
        memorySprite.setPosition(0, c8::vga::hostRenderHeight + 10);

        window->draw(vgaSprite);
        window->draw(cpuInfoSprite);
        window->draw(memorySprite);

        window->display();
    }

    void drawText(
        sf::RenderTexture& texture, 
        const int x, 
        const int y, 
        std::string str)
    {
        sf::Text text;

        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(18);
        text.setFillColor(sf::Color::Green);
        text.setPosition(x, y);

        texture.draw(text);
    }

    void drawText(
        sf::RenderTexture& texture, 
        const int x, 
        const int y, 
        const std::stringstream& ss)
    {
        drawText(texture, x, y, ss.str());
    }

    bool isOpen()
    {
        return window != nullptr && window->isOpen();
    }

    Hex::Hex(std::uint8_t value, bool includeDec)
        : value{value}, hexDigits{2}, decDigits{3}, includeDec{includeDec}
    {
    }

    Hex::Hex(std::uint16_t value, bool includeDec)
        : value{value}, hexDigits{4}, decDigits{6}, includeDec{includeDec}
    {
    }

    std::ostream& operator<<(std::ostream& ss, const Hex& hex)
    {
        ss << "0x" << std::setfill('0') << std::setw(hex.hexDigits) << std::uppercase << std::hex << hex.value;

        if (hex.includeDec) {
            ss << " (" << std::setfill('0') << std::setw(hex.decDigits) << std::dec << hex.value << ")";
        }

        return ss;
    }
}
