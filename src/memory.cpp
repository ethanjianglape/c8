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

#include "memory.hpp"

#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <cstring>

#include "vga.hpp"
#include "cpu.hpp"
#include "opcodes.hpp"
#include "ui.hpp"

namespace c8::mem
{
    constexpr std::uint8_t sprites[16][5] = {
        { 0xF0, 0x90, 0x90, 0x90, 0xF0 }, // 0
        { 0x20, 0x60, 0x20, 0x20, 0x70 }, // 1
        { 0xF0, 0x10, 0xF0, 0x80, 0xF0 }, // 2
        { 0xF0, 0x10, 0xF0, 0x10, 0xF0 }, // 3
        { 0x90, 0x90, 0xF0, 0x10, 0x10 }, // 4
        { 0xF0, 0x80, 0xF0, 0x10, 0xF0 }, // 5
        { 0xF0, 0x80, 0xF0, 0x90, 0xF0 }, // 6
        { 0xF0, 0x10, 0x20, 0x40, 0x40 }, // 7
        { 0xF0, 0x90, 0xF0, 0x90, 0xF0 }, // 8
        { 0xF0, 0x90, 0xF0, 0x10, 0xF0 }, // 9
        { 0xF0, 0x90, 0xF0, 0x90, 0x90 }, // A
        { 0xE0, 0x90, 0xE0, 0x90, 0xE0 }, // B
        { 0xF0, 0x80, 0x80, 0x80, 0xF0 }, // C
        { 0xE0, 0x90, 0x90, 0x90, 0xE0 }, // D
        { 0xF0, 0x80, 0xF0, 0x80, 0xF0 }, // E
        { 0xF0, 0x80, 0xF0, 0x80, 0x80 }, // F
    };

    constexpr std::uint8_t defaultProgram[] = {
        0x12, 0x0c, 0xf0, 0x80, 0x80, 0x80, 0xf0, 0xf0, 0x90, 0xf0, 0x90, 0xf0,
        0x6e, 0x10, 0x60, 0x1b, 0x61, 0x0a, 0xa2, 0x02, 0xd0, 0x15, 0x80, 0xf0,
        0x60, 0x20, 0x61, 0x0a, 0xa2, 0x07, 0xd0, 0x15, 0x80, 0xf0, 0x60, 0x01,
        0x40, 0x00, 0x12, 0x2a, 0x12, 0x22, 0x6f, 0x03, 0x8e, 0xf5, 0x8f, 0x00,
        0xfe, 0x29, 0xfe, 0x65, 0x00, 0xee, 0x60, 0xff, 0x61, 0x00, 0xe1, 0xa1,
        0x60, 0x00, 0x61, 0x01, 0xe1, 0xa1, 0x60, 0x01, 0x61, 0x02, 0xe1, 0xa1,
        0x60, 0x02, 0x61, 0x03, 0xe1, 0xa1, 0x60, 0x03, 0x61, 0x04, 0xe1, 0xa1,
        0x60, 0x04, 0x61, 0x05, 0xe1, 0xa1, 0x60, 0x05, 0x61, 0x06, 0xe1, 0xa1,
        0x60, 0x06, 0x61, 0x07, 0xe1, 0xa1, 0x60, 0x07, 0x61, 0x08, 0xe1, 0xa1,
        0x60, 0x08, 0x61, 0x09, 0xe1, 0xa1, 0x60, 0x09, 0x61, 0x0a, 0xe1, 0xa1,
        0x60, 0x0a, 0x61, 0x0b, 0xe1, 0xa1, 0x60, 0x0b, 0x61, 0x0c, 0xe1, 0xa1,
        0x60, 0x0c, 0x61, 0x0d, 0xe1, 0xa1, 0x60, 0x0d, 0x61, 0x0e, 0xe1, 0xa1,
        0x60, 0x0e, 0x61, 0x0f, 0xe1, 0xa1, 0x60, 0x0f, 0x6f, 0x03, 0x8e, 0xf5,
        0x8f, 0x00, 0xfe, 0x29, 0xfe, 0x65, 0x00, 0xee, 0x85, 0x20, 0x84, 0x10,
        0x83, 0x00, 0x66, 0x00, 0xf5, 0x33, 0xf2, 0x65, 0xf0, 0x29, 0xd3, 0x45,
        0x86, 0xf1, 0xf1, 0x29, 0x73, 0x05, 0xd3, 0x45, 0x86, 0xf1, 0xf2, 0x29,
        0x73, 0x05, 0xd3, 0x45, 0x86, 0xf1, 0x6f, 0x03, 0x8e, 0xf5, 0x8f, 0x60,
        0xfe, 0x29, 0xfe, 0x65, 0x00, 0xee
    };

    constexpr int defaultProgramLength = sizeof(defaultProgram);
    constexpr int maxBufferSize = 4096;

    std::uint8_t buffer[maxBufferSize];
    std::uint8_t originalBuffer[maxBufferSize];

    void zeroMemory();

    void writeSprite(const std::uint16_t addr, const std::uint8_t* sprite);

    void writeSprites();

    void reset()
    {
        std::memcpy(buffer, originalBuffer, maxBufferSize);
    }

    void drawMemoryInfoLine(
        std::stringstream& ss, 
        const std::uint16_t addr, 
        const bool isCurrentAddr = false)
    {
        const std::uint16_t word = readWord(addr);

        if (isCurrentAddr) {
            ss << " >";
        } else {
            ss << "  ";
        }

        ss << c8::ui::Hex{addr, false} << "\t"
           << c8::ui::Hex{word, false} << "\t"
           << c8::opcodes::getOpcodeName(word) << "\n";
    }

    void render(sf::RenderTexture& texture)
    {
        const int linesAroundPc = 10;
        const std::uint16_t pc = c8::cpu::getProgramCounter();

        std::stringstream ss;

        for (int i = 0; i < linesAroundPc; i++) {
            const std::uint16_t addr = pc - (linesAroundPc * 2) + (i * 2);

            drawMemoryInfoLine(ss, addr);
        }

        drawMemoryInfoLine(ss, pc, true);

        for (int i = 0; i < linesAroundPc; i++) {
            const std::uint16_t addr = pc + (i * 2) + 2;

            drawMemoryInfoLine(ss, addr);
        }

        c8::ui::drawText(texture, 0, 0, ss);
    }

    std::uint8_t readByte(const std::uint16_t addr) 
    {
        if (addr >= maxBufferSize) {
            return 0;
        }

        return buffer[addr];
    }

    std::uint16_t readWord(const std::uint16_t addr)
    {
        if (addr >= maxBufferSize) {
            return 0;
        }

        const std::uint8_t highByte = readByte(addr);
        const std::uint8_t lowByte = readByte(addr + 1);
        
        return (highByte << 8) | lowByte;
    }

    void writeByte(const int addr, const std::uint8_t data)
    {
        if (addr < 0 || addr >= maxBufferSize) {
            return;
        }

        buffer[addr] = data;
    }

    void writeSprite(const std::uint16_t addr, const std::uint8_t* sprite)
    {
        writeByte(addr, sprite[0]);
        writeByte(addr + 1, sprite[1]);
        writeByte(addr + 2, sprite[2]);
        writeByte(addr + 3, sprite[3]);
        writeByte(addr + 4, sprite[4]);
    }

    void writeSprites()
    {
        std::uint16_t addr = 0x0;

        for (int i = 0; i < 16; i++) {
            const std::uint8_t* sprite = sprites[i];

            writeSprite(addr + (i * 5), sprite);
        }
    }

    void zeroMemory() 
    {
        std::fill(buffer, buffer + maxBufferSize, 0);
    }

    void loadDefaultProgram()
    {
        std::memcpy(buffer + 0x200, defaultProgram, defaultProgramLength);
    }

    void initialize()
    {
        zeroMemory();
        writeSprites();
        loadDefaultProgram();

        std::memcpy(originalBuffer, buffer, maxBufferSize);
    }

    void loadProgram(std::ifstream& file)
    {
        file.seekg(0, file.end);

        int length = file.tellg();

        file.seekg(0, file.beg);
        file.read((char*)buffer + 0x200, length);

        std::memcpy(originalBuffer, buffer, maxBufferSize);
    }

    std::uint16_t getFontSpriteAddress(const std::uint8_t spriteIndex)
    {
        return spriteIndex * 5;
    }
}
