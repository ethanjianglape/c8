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

#include <string>
#include <cstdint>

namespace c8::opcodes
{
    enum class Opcode: int
    {
        Invalid = -1,
        CLS = 0,
        RET = 1,
        JP_Addr = 2,
        CALL_Addr = 3,
        SE_Vx_Byte = 4,
        SNE_Vx_Byte = 5,
        SE_Vx_Vy = 6,
        LD_Vx_Byte = 7,
        ADD_Vx_Byte = 8,
        LD_Vx_Vy = 9,
        OR_Vx_Vy = 10,
        AND_Vx_Vy = 11,
        XOR_Vx_Vy = 12,
        ADD_Vx_Vy = 13,
        SUB_Vx_Vy = 14,
        SHR_Vx_Vy = 15,
        SUBN_Vx_Vy = 16,
        SHL_Vx_Vy = 17,
        SNE_Vx_Vy = 18,
        LD_I_Addr = 19,
        JP_V0_Addr = 20,
        RND_Vx_Byte = 21,
        DRW_Vx_Vy_Nibble = 22,
        SKP_Vx = 23,
        SKNP_Vx = 24,
        LD_Vx_DT = 25,
        LD_Vx_K = 26,
        LD_DT_Vx = 27,
        LD_ST_Vx = 28,
        ADD_I_Vx = 29,
        LD_F_Vx = 30,
        LD_B_Vx = 31,
        LD_IAddr_Vx = 32,
        LD_Vx_IAddr = 33
    };

    /**
     * opcode instruction = xxxx 0000 0000 0000
    */
    std::uint8_t getOpcodeInstruction(const std::uint16_t opcode);

    /**
     * opcode x = 0000 xxxx 0000 0000
    */
    std::uint8_t getOpcodeX(const std::uint16_t opcode);

    /**
     * opcode y = 0000 0000 yyyy 0000
    */
    std::uint8_t getOpcodeY(const std::uint16_t opcode);

    /**
     * opcode z = 0000 0000 0000 zzzz
    */
    std::uint8_t getOpcodeZ(const std::uint16_t opcode);

    /**
     * opcode kk = 0000 0000 kkkk kkkk;
    */
    std::uint8_t getOpcodeKK(const std::uint16_t opcode);

    /**
     * opcode nnn = 0000 nnnn nnnn nnnn
    */
    std::uint16_t getOpcodeNNN(const std::uint16_t opcode);

    Opcode decode(const std::uint16_t opcode);

    std::string getOpcodeName(const std::uint16_t opcode);
}