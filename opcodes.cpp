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

#include "opcodes.hpp"

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace c8::opcodes
{
    std::uint8_t getOpcodeInstruction(const std::uint16_t opcode)
    {
        return opcode >> 12;
    }

    std::uint8_t getOpcodeX(const std::uint16_t opcode)
    {
        return (opcode >> 8) & 0b1111;
    }

    std::uint8_t getOpcodeY(const std::uint16_t opcode)
    {
        return (opcode >> 4) & 0b1111;
    }

    std::uint8_t getOpcodeZ(const std::uint16_t opcode)
    {
        return opcode & 0b1111;
    }

    std::uint8_t getOpcodeKK(const std::uint16_t opcode)
    {
        return opcode & 0b0000'0000'1111'1111;
    }

    std::uint16_t getOpcodeNNN(const std::uint16_t opcode)
    {
        return opcode & 0b0000'1111'1111'1111;
    }

    Opcode decode(const std::uint16_t opcode)
    {
        if (opcode == 0) {
            return Opcode::Invalid;
        }

        const std::uint8_t instruction = getOpcodeInstruction(opcode);
        const std::uint8_t z = getOpcodeZ(opcode);
        const std::uint8_t kk = getOpcodeKK(opcode);

        switch (instruction) {
        case 0x0:
            if (kk == 0xE0) {
                return Opcode::CLS;
            }

            if (kk == 0xEE) {
                return Opcode::RET;
            }

            return Opcode::Invalid;
        case 0x1:
            return Opcode::JP_Addr;
        case 0x2:
            return Opcode::CALL_Addr;
        case 0x3:
            return Opcode::SE_Vx_Byte;
        case 0x4:
            return Opcode::SNE_Vx_Byte;
        case 0x5:
            if (z == 0x0) {
                return Opcode::SE_Vx_Vy;
            }

            return Opcode::Invalid;
        case 0x6:
            return Opcode::LD_Vx_Byte;
        case 0x7:
            return Opcode::ADD_Vx_Byte;
        case 0x8:
            if (z == 0x0) {
                return Opcode::LD_Vx_Vy;
            }

            if (z == 0x1) {
                return Opcode::OR_Vx_Vy;
            }
            
            if (z == 0x2) {
                return Opcode::AND_Vx_Vy;
            }
            
            if (z == 0x3) {
                return Opcode::XOR_Vx_Vy;
            }
            
            if (z == 0x4) {
                return Opcode::ADD_Vx_Vy;
            }
            
            if (z == 0x5) {
                return Opcode::SUB_Vx_Vy;
            }
            
            if (z == 0x6) {
                return Opcode::SHR_Vx_Vy;
            }
            
            if (z == 0x7) {
                return Opcode::SUBN_Vx_Vy;
            }

            if (z == 0xE) {
                return Opcode::SHL_Vx_Vy;
            }

            return Opcode::Invalid;
        case 0x9:
            if (z == 0x0) {
                return Opcode::SNE_Vx_Vy;
            }

            return Opcode::Invalid;
        case 0xA:
            return Opcode::LD_I_Addr;
        case 0xB:
            return Opcode::JP_V0_Addr;
        case 0xC:
            return Opcode::RND_Vx_Byte;
        case 0xD:
            return Opcode::DRW_Vx_Vy_Nibble;
        case 0xE:
            if (kk == 0x9E) {
                return Opcode::SKP_Vx;
            }

            if (kk == 0xA1) {
                return Opcode::SKNP_Vx;
            }

            return Opcode::Invalid;
        case 0xF:
            if (kk == 0x07) {
                return Opcode::LD_Vx_DT;
            }

            if (kk == 0x0A) {
                return Opcode::LD_Vx_K;
            }

            if (kk == 0x15) {
                return Opcode::LD_DT_Vx;
            }

            if (kk == 0x18) {
                return Opcode::LD_ST_Vx;
            }

            if (kk == 0x1E) {
                return Opcode::ADD_I_Vx;
            }

            if (kk == 0x29) {
                return Opcode::LD_F_Vx;
            }

            if (kk == 0x33) {
                return Opcode::LD_B_Vx;
            }

            if (kk == 0x55) {
                return Opcode::LD_IAddr_Vx;
            }

            if (kk == 0x65) { 
                return Opcode::LD_Vx_IAddr;
            }

            return Opcode::Invalid;
        }

        return Opcode::Invalid;
    }

    std::string getOpcodeName(const std::uint16_t word)
    {
        const Opcode opcode = c8::opcodes::decode(word);

        const int x = c8::opcodes::getOpcodeX(word);
        const int y = c8::opcodes::getOpcodeY(word);
        const int z = c8::opcodes::getOpcodeZ(word);

        const int kk = c8::opcodes::getOpcodeKK(word);
        const int nnn = c8::opcodes::getOpcodeNNN(word);

        std::stringstream ss;

        switch (opcode) {
        case Opcode::CLS:
            ss << "CLS";
            break;
        case Opcode::RET:
            ss << "RET";
            break;
        case Opcode::JP_Addr:
            ss << "JP 0x" << std::setfill('0') << std::setw(3) << std::uppercase << std::hex << nnn << std::dec << " (" << nnn << ")";
            break;
        case Opcode::CALL_Addr:
            ss << "CALL 0x" << std::setfill('0') << std::setw(3) << std::uppercase << std::hex << nnn << std::dec << " (" << nnn << ")";
            break;
        case Opcode::SE_Vx_Byte:
            ss << "SE V" << std::uppercase << std::hex << x << ", " << kk;
            break;
        case Opcode::SNE_Vx_Byte:
            ss << "SNE V" << std::uppercase << std::hex << x << ", " << kk;
            break;
        case Opcode::SE_Vx_Vy:
            ss << "SE V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::LD_Vx_Byte:
            ss << "LD V" << std::uppercase << std::hex << x << ", " << kk;
            break;
        case Opcode::ADD_Vx_Byte:
            ss << "ADD V" << std::uppercase << std::hex << x << ", " << kk;
            break;
        case Opcode::LD_Vx_Vy:
            ss << "LD V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::OR_Vx_Vy:
            ss << "OR V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::AND_Vx_Vy:
            ss << "AND V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::XOR_Vx_Vy:
            ss << "XOR V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::ADD_Vx_Vy:
            ss << "ADD V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::SUB_Vx_Vy:
            ss << "SUB V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::SHR_Vx_Vy:
            ss << "SHR V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::SUBN_Vx_Vy:
            ss << "SUBN V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::SHL_Vx_Vy:
            ss << "SHL V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::SNE_Vx_Vy:
            ss << "SNE V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y;
            break;
        case Opcode::LD_I_Addr:
            ss << "LD I, 0x" << std::setfill('0') << std::setw(3) << std::uppercase << std::hex << nnn << std::dec << " (" << nnn << ")";
            break;
        case Opcode::JP_V0_Addr:
            ss << "JP V0, 0x" << std::setfill('0') << std::setw(3) << std::uppercase << std::hex << nnn << std::dec << " (" << nnn << ")";
            break;
        case Opcode::RND_Vx_Byte:
            ss << "RND V" << std::uppercase << std::hex << x << ", " << kk;
            break;
        case Opcode::DRW_Vx_Vy_Nibble:
            ss << "DRW V" << std::uppercase << std::hex << x << ", V" << std::uppercase << std::hex << y << ", " << z;
            break;
        case Opcode::SKP_Vx:
            ss << "SKP V" << std::uppercase << std::hex << x;
            break;
        case Opcode::SKNP_Vx:
            ss << "SKNP V" << std::uppercase << std::hex << x;
            break;
        case Opcode::LD_Vx_DT:
            ss << "LD V" << std::uppercase << std::hex << x << ", DT";
            break;
        case Opcode::LD_Vx_K:
            ss << "LD V" << std::uppercase << std::hex << x << ", K";
            break;
        case Opcode::LD_DT_Vx:
            ss << "LD DT, V" << std::uppercase << std::hex << x;
            break;
        case Opcode::LD_ST_Vx:
            ss << "LD ST, V" << std::uppercase << std::hex << x;
            break;
        case Opcode::ADD_I_Vx:
            ss << "ADD I, V" << std::uppercase << std::hex << x;
            break;
        case Opcode::LD_F_Vx:
            ss << "LD F, V" << std::uppercase << std::hex << x;
            break;
        case Opcode::LD_B_Vx:
            ss << "LD B, V" << std::uppercase << std::hex << x;
            break;
        case Opcode::LD_IAddr_Vx:
            ss << "LD [I], V" << std::uppercase << std::hex << x;
            break;
        case Opcode::LD_Vx_IAddr:
            ss << "LD V" << std::uppercase << std::hex << x << ", [I]";
            break;
        case Opcode::Invalid:
            break;
        }

        return std::string{ss.str()};
    }
}
