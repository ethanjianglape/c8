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

#include <array>
#include <algorithm>
#include <cstring>
#include <unordered_map>

#include "cpu.hpp"
#include "opcodes.hpp"
#include "memory.hpp"
#include "vga.hpp"
#include "ui.hpp"

namespace c8::cpu
{
    std::random_device rd;
    std::mt19937 generator{rd()};
    std::uniform_int_distribution<int> distribution{0, 255};

    int cpuHertz;
    int hostFps;

    bool paused;
    bool doAdvanceOneClockCycle;
    bool waitingForKeyboard;

    std::uint8_t keyboardPressedValue;

    class CpuState
    {
    public:
        c8::vga::VgaState vgaState;

        std::uint16_t pc;
        std::uint16_t ir;

        std::uint8_t dt;
        std::uint8_t st;
        std::uint8_t sp;

        std::array<std::uint8_t, 16> v;
        std::stack<std::uint16_t> stack;

        std::uint8_t* getRegister(const std::uint8_t index)
        {
            if (index > 0xF) {
                return nullptr;
            }

            return &v[index];
        }

        std::uint16_t popFromStack()
        {
            const std::uint16_t value =  stack.top();

            stack.pop();
            sp--;

            return value;
        }

        void pushToStack(const std::uint16_t value)
        {
            sp++;
            stack.push(value);
        }

        std::uint8_t getCurrentKeyboardValue()
        {
            for (const auto& [key, value] : c8::ui::valueByKey) { 
                if (sf::Keyboard::isKeyPressed(key)) {
                    return value;
                }
            }

            return 0xFF;
        }

        bool CLS()
        {
            vgaState.clear();
            pc += 2;

            return true;
        }

        bool RET()
        {
            const std::uint16_t addr = popFromStack();

            pc = addr + 2;

            return true;
        }

        bool JP_Addr(const std::uint16_t addr)
        {
            if (pc == addr) {
                return false;
            }

            pc = addr;

            return true;
        }

        bool CALL_Addr(const std::uint16_t addr)
        {
            pushToStack(pc);
            pc = addr;

            return true;
        }

        bool SE_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            if (*vx == value) {
                pc += 2;
            }

            pc += 2;

            return true;
        }

        bool SNE_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            if (*vx != value) {
                pc += 2;
            }

            pc += 2;

            return true;
        }


        bool SE_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            if (*vx == *vy) {
                pc += 2;
            }

            pc += 2;

            return true;
        }

        bool LD_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            *vx = value;
            pc += 2;

            return true;
        }

        bool ADD_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            *vx += value;
            pc += 2;

            return true;
        }

        bool LD_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            *vx = *vy;
            pc += 2;

            return true;
        }

        bool OR_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            *vx = *vx | *vy;
            pc += 2;

            return true;
        }

        bool AND_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            *vx = *vx & *vy;
            pc += 2;

            return true;
        }

        bool XOR_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            *vx = *vx ^ *vy;
            pc += 2;

            return true;
        }

        bool ADD_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            const auto result = (int)*vx + (int)*vy;

            v[15] = result > 255 ? 1 : 0;
            *vx = static_cast<std::uint8_t>(result);

            pc += 2;

            return true;
        }

        bool SUB_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            const auto result = *vx - *vy;

            v[15] = *vx > *vy ? 1 : 0;

            *vx = static_cast<std::uint8_t>(result);
            pc += 2;

            return true;
        }

        bool SHR_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            if (c8::quirks::shiftWithVy) {
                *vx = *vy;
            }

            const auto bit = *vx & 0b0000'0001;

            v[15] = bit >= 1 ? 1 : 0;

            *vx = *vx >> 1;
            pc += 2;

            return true;
        }

        bool SUBN_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            const auto result = *vy - *vx;

            v[15] = *vy > *vx ? 1 : 0;

            *vx = static_cast<std::uint8_t>(result);   
            pc += 2;

            return true;
        }

        bool SHL_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            if (c8::quirks::shiftWithVy) {
                *vx = *vy;
            }

            const auto bit = *vx & 0b1000'0000;

            v[15] = bit >= 1 ? 1 : 0;

            *vx = *vx << 1;
            pc += 2;

            return true;
        }

        bool SNE_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return false;
            }

            if (*vx != *vy) {
                pc += 2;
            }

            pc += 2;

            return true;
        }

        bool LD_I_Addr(const std::uint16_t value) 
        {
            ir = value;
            pc += 2;

            return true;
        }

        bool JP_V0_Addr(const std::uint16_t value)
        {
            pc = value + v[0];

            return true;
        }

        bool RND_Vx_Byte(const std::uint8_t x, const std::uint16_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            const std::uint8_t randomValue = distribution(generator);

            *vx = randomValue & value;
            pc += 2;

            return true;
        }

        bool DRW_Vx_Vy_Nibble(const std::uint8_t x, const std::uint8_t y, const std::uint8_t n)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr || n == 0) {
                return false;
            }

            bool didErase = false;

            for (int i = 0; i < n; i++) {
                const std::uint8_t byte = c8::mem::readByte(ir + i);

                didErase = vgaState.drawByte(*vx, *vy + i, byte) || didErase;
            }

            v[15] = didErase ? 1 : 0;
            pc += 2;

            return true;
        }

        bool SKP_Vx(const std::uint8_t x) 
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            const std::uint8_t input = getCurrentKeyboardValue();

            if (*vx == input) {
                pc += 2;
            }

            pc += 2;

            return true;
        }

        bool SKNP_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            const std::uint8_t input = getCurrentKeyboardValue();

            if (*vx != input) {
                pc += 2;
            }

            pc += 2;

            return true;
        }

        bool LD_Vx_DT(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            *vx = dt;
            pc += 2;

            return true;
        }

        bool LD_Vx_K(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            waitingForKeyboard = true;

            if (keyboardPressedValue == 0xFF) {
                return false;
            }

            *vx = keyboardPressedValue;
            pc += 2;

            keyboardPressedValue = 0xFF;
            waitingForKeyboard = false;

            return true;
        }

        bool LD_DT_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            dt = *vx;
            pc += 2;

            return true;
        }

        bool LD_ST_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            st = *vx;
            pc += 2;

            return true;
        }

        bool ADD_I_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            ir = ir + *vx;
            pc += 2;

            return true;
        }

        bool LD_F_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            ir = c8::mem::getFontSpriteAddress(*vx);
            pc += 2;

            return true;
        }

        bool LD_B_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return false;
            }

            const std::uint8_t hundreds = (*vx / 100) % 10;
            const std::uint8_t tens = (*vx / 10) % 10;
            const std::uint8_t ones = *vx % 10;

            c8::mem::writeByte(ir, hundreds);
            c8::mem::writeByte(ir + 1, tens);
            c8::mem::writeByte(ir + 2, ones);

            pc += 2;

            return true;
        }

        bool LD_IAddr_Vx(const std::uint8_t x)
        {
            for (std::uint8_t i = 0; i <= x; i++) {
                std::uint8_t* vx = getRegister(i);

                c8::mem::writeByte(ir + i, *vx);
            }

            if (c8::quirks::memoryIncrementI) {
                ir += x + 1;
            }

            pc += 2;

            return true;
        }

        bool LD_Vx_IAddr(const std::uint8_t x)
        {
            for (std::uint8_t i = 0; i <= x; i++) {
                std::uint8_t* vx = getRegister(i);

                *vx = c8::mem::readByte(ir + i);
            }

            if (c8::quirks::memoryIncrementI) {
                ir += x + 1;
            }

            pc += 2;

            return true;
        }
    };

    constexpr int maxCpuStates = 1000;

    CpuState cpuStates[maxCpuStates];

    std::uint64_t totalCpuCycles = 0;
    int currentCpuStateDisplayIndex = 0;
    int currentCpuStateIndex = 0;
    int headCpuStateIndex = 0;

    CpuState& getCurrentCpuState()
    {
        return cpuStates[currentCpuStateIndex];
    }

    void initialize()
    {
        paused = false;
        doAdvanceOneClockCycle = false;
        waitingForKeyboard = false;
        keyboardPressedValue = 0xFF;

        reset();
    }

    void setCpuFrequency(int hz)
    {
        cpuHertz = hz;
    }

    void setFps(int fps)
    {
        hostFps = fps;
    }

    void reset()
    {
        currentCpuStateIndex = 0;
        headCpuStateIndex = 0;
        totalCpuCycles = 0;

        CpuState& currentCpuState = getCurrentCpuState();

        currentCpuState.pc = 0x200;
        currentCpuState.vgaState.clear();

        c8::mem::reset();
    }

    void keyboardKeyPressed(std::uint8_t value)
    {
        if (!waitingForKeyboard) {
            return;
        }

        keyboardPressedValue = value;
    }

    std::uint16_t getProgramCounter()
    {
        return getCurrentCpuState().pc;
    }

    void togglePaused()
    {
        paused = !paused;
    }

    void advanceOneClockCycle()
    {
        if (!paused) {
            return;
        }

        doAdvanceOneClockCycle = true;
    }

    void backOneClockCylce()
    {
        if (!paused) {
            return;
        }

        if (currentCpuStateIndex != headCpuStateIndex + 1) {
            currentCpuStateIndex = currentCpuStateIndex == 0 ? maxCpuStates - 1 : currentCpuStateIndex - 1;
            currentCpuStateDisplayIndex--;
        }

        if (totalCpuCycles < maxCpuStates && currentCpuStateIndex > headCpuStateIndex) {
            currentCpuStateIndex = 0;
            currentCpuStateDisplayIndex = 0;
        }
    }

    void renderCpuInfo(sf::RenderTexture& texture)
    {
        std::stringstream ss;

        const CpuState& currentCpuState = getCurrentCpuState();

        ss << "PC = " << c8::ui::Hex{currentCpuState.pc} << "\t" << "I = " << c8::ui::Hex{currentCpuState.ir} << "\n\n";
        ss << "DT = " << c8::ui::Hex{currentCpuState.dt} << "\t" << "ST = " << c8::ui::Hex{currentCpuState.st} << "\n\n";
        ss << "V0 = " << c8::ui::Hex{currentCpuState.v[0x0]} << "\tV8 = " << c8::ui::Hex{currentCpuState.v[0x8]} << "\n";
        ss << "V1 = " << c8::ui::Hex{currentCpuState.v[0x1]} << "\tV9 = " << c8::ui::Hex{currentCpuState.v[0x9]} << "\n";
        ss << "V2 = " << c8::ui::Hex{currentCpuState.v[0x2]} << "\tVA = " << c8::ui::Hex{currentCpuState.v[0xA]} << "\n";
        ss << "V3 = " << c8::ui::Hex{currentCpuState.v[0x3]} << "\tVB = " << c8::ui::Hex{currentCpuState.v[0xB]} << "\n";
        ss << "V4 = " << c8::ui::Hex{currentCpuState.v[0x4]} << "\tVC = " << c8::ui::Hex{currentCpuState.v[0xC]} << "\n";
        ss << "V5 = " << c8::ui::Hex{currentCpuState.v[0x5]} << "\tVD = " << c8::ui::Hex{currentCpuState.v[0xD]} << "\n";
        ss << "V6 = " << c8::ui::Hex{currentCpuState.v[0x6]} << "\tVE = " << c8::ui::Hex{currentCpuState.v[0xE]} << "\n";
        ss << "V7 = " << c8::ui::Hex{currentCpuState.v[0x7]} << "\tVF = " << c8::ui::Hex{currentCpuState.v[0xF]} << "\n\n";
        ss << "Emulator State = " << (paused ? "PAUSED" : "RUNNING") << "\n";
        ss << "Current CPU State = " << currentCpuStateDisplayIndex << "/" << maxCpuStates << "\n";
        ss << "CPU Frequency = " << cpuHertz << "Hz" << "\n";
        ss << "Render Speed = " << hostFps << "FPS" << "\n\n";
        ss << "Controls:\n";
        ss << "P = start/pause emulator" << "\n";
        ss << "Left/Right = forward/backward 1 CPU cycle";

        c8::ui::drawText(texture, 0, 0, ss);
    }

    void renderVga(sf::RenderTexture& texture)
    {
        getCurrentCpuState().vgaState.render(texture);
    }

    void render(sf::RenderTexture& vgaTexture, sf::RenderTexture& cpuInfoTexture)
    {
        renderVga(vgaTexture);
        renderCpuInfo(cpuInfoTexture);
    }

    void decrementTimers()
    {
        if (paused && !doAdvanceOneClockCycle) {
            return;
        }

        CpuState& currentCpuState = getCurrentCpuState();

        if (currentCpuState.dt > 0) {
            currentCpuState.dt--;
        }

        if (currentCpuState.st > 0) {
            currentCpuState.st--;
        }
    }

    bool processOpcode(const std::uint16_t word)
    {
        const c8::opcodes::Opcode opcode = c8::opcodes::decode(word);

        const std::uint8_t x = c8::opcodes::getOpcodeX(word);
        const std::uint8_t y = c8::opcodes::getOpcodeY(word);
        const std::uint8_t z = c8::opcodes::getOpcodeZ(word);

        const std::uint8_t kk = c8::opcodes::getOpcodeKK(word);
        const std::uint16_t nnn = c8::opcodes::getOpcodeNNN(word);

        CpuState& currentCpuState = getCurrentCpuState();

        switch (opcode) {
        case c8::opcodes::Opcode::CLS:
            return currentCpuState.CLS();
        case c8::opcodes::Opcode::RET:
            return currentCpuState.RET();
        case c8::opcodes::Opcode::JP_Addr:
            return currentCpuState.JP_Addr(nnn);
        case c8::opcodes::Opcode::CALL_Addr:
            return currentCpuState.CALL_Addr(nnn);
        case c8::opcodes::Opcode::SE_Vx_Byte:
            return currentCpuState.SE_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::SNE_Vx_Byte:
            return currentCpuState.SNE_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::SE_Vx_Vy:
            return currentCpuState.SE_Vx_Vy(x, y);
        case c8::opcodes::Opcode::LD_Vx_Byte:
            return currentCpuState.LD_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::ADD_Vx_Byte:
            return currentCpuState.ADD_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::LD_Vx_Vy:
            return currentCpuState.LD_Vx_Vy(x, y);
        case c8::opcodes::Opcode::OR_Vx_Vy:
            return currentCpuState.OR_Vx_Vy(x, y);
        case c8::opcodes::Opcode::AND_Vx_Vy:
            return currentCpuState.AND_Vx_Vy(x, y);
        case c8::opcodes::Opcode::XOR_Vx_Vy:
            return currentCpuState.XOR_Vx_Vy(x, y);
        case c8::opcodes::Opcode::ADD_Vx_Vy:
            return currentCpuState.ADD_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SUB_Vx_Vy:
            return currentCpuState.SUB_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SHR_Vx_Vy:
            return currentCpuState.SHR_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SUBN_Vx_Vy:
            return currentCpuState.SUBN_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SHL_Vx_Vy:
            return currentCpuState.SHL_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SNE_Vx_Vy:
            return currentCpuState.SNE_Vx_Vy(x, y);
        case c8::opcodes::Opcode::LD_I_Addr:
            return currentCpuState.LD_I_Addr(nnn);
        case c8::opcodes::Opcode::JP_V0_Addr:
            return currentCpuState.JP_V0_Addr(nnn);
        case c8::opcodes::Opcode::RND_Vx_Byte:
            return currentCpuState.RND_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::DRW_Vx_Vy_Nibble:
            return currentCpuState.DRW_Vx_Vy_Nibble(x, y, z);
        case c8::opcodes::Opcode::SKP_Vx:
            return currentCpuState.SKP_Vx(x);
        case c8::opcodes::Opcode::SKNP_Vx:
            return currentCpuState.SKNP_Vx(x);
        case c8::opcodes::Opcode::LD_Vx_DT:
            return currentCpuState.LD_Vx_DT(x);
        case c8::opcodes::Opcode::LD_Vx_K:
            return currentCpuState.LD_Vx_K(x);
        case c8::opcodes::Opcode::LD_DT_Vx:
            return currentCpuState.LD_DT_Vx(x);
        case c8::opcodes::Opcode::LD_ST_Vx:
            return currentCpuState.LD_ST_Vx(x);
        case c8::opcodes::Opcode::ADD_I_Vx:
            return currentCpuState.ADD_I_Vx(x);
        case c8::opcodes::Opcode::LD_F_Vx:
            return currentCpuState.LD_F_Vx(x);
        case c8::opcodes::Opcode::LD_B_Vx:
            return currentCpuState.LD_B_Vx(x);
        case c8::opcodes::Opcode::LD_IAddr_Vx:
            return currentCpuState.LD_IAddr_Vx(x);
        case c8::opcodes::Opcode::LD_Vx_IAddr:
            return currentCpuState.LD_Vx_IAddr(x);
        case c8::opcodes::Opcode::Invalid:
        default:
            return false;
        }
    }

    void executeClockCycle()
    {
        if (paused && !doAdvanceOneClockCycle) {
            return;
        }

        doAdvanceOneClockCycle = false;
        currentCpuStateDisplayIndex = currentCpuStateDisplayIndex == maxCpuStates ? maxCpuStates : currentCpuStateDisplayIndex + 1;

        // If the currentCpuStateIndex != headCpuStateIndex then we are currently
        // in a past cpu cycle, so to advance by 1 clock cycle, just increment
        // currentCpuStateIndex until it equals headCpuStateIndex
        if (currentCpuStateIndex != headCpuStateIndex) {
            currentCpuStateIndex = currentCpuStateIndex == maxCpuStates - 1 ? 0 : currentCpuStateIndex + 1;

            return;
        }

        CpuState& currentCpuState = getCurrentCpuState();

        const std::uint16_t opcode = c8::mem::readWord(currentCpuState.pc);

        if (opcode == 0x0){
            return;
        }

        totalCpuCycles++;
        headCpuStateIndex = headCpuStateIndex == maxCpuStates - 1 ? 0 : headCpuStateIndex + 1;
        currentCpuStateIndex = headCpuStateIndex;

        cpuStates[headCpuStateIndex] = currentCpuState;

        const bool didUpdate = processOpcode(opcode);

        // If executing the next cpu instruction didn't result in any
        // changes to the cpu state, we do not need to keep this one in our history.
        if (!didUpdate) {
            headCpuStateIndex = headCpuStateIndex == 0 ? maxCpuStates - 1 : headCpuStateIndex - 1;
            currentCpuStateIndex = headCpuStateIndex;
        }
    }
}
