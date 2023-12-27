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
    std::uniform_int_distribution<std::uint8_t> distribution{0, 255};

    constexpr int maxCpuStates = 1000;
    int cpuStates = 0;
    int cpuStateIndex = 0;

    int cpuHertz;
    int hostFps;

    bool paused;
    bool doAdvanceOneClockCycle;
    bool waitingForKeyboard;

    std::uint8_t keyboardPressedValue;

    bool isKeyPressed[16];

    struct CpuState
    {
        CpuState* previousState;
        CpuState* nextState;
        
        c8::vga::VgaState vgaState;

        std::uint16_t pc;
        std::uint16_t ir;

        std::uint8_t dt;
        std::uint8_t st;
        std::uint8_t sp;

        std::uint8_t v[16];

        std::stack<std::uint16_t> stack;

        bool didUpdate;

        CpuState() : 
            previousState{nullptr}, 
            nextState{nullptr}, 
            didUpdate{true}
        {
        }

        CpuState(CpuState* state)
        {
            previousState = state;
            nextState = nullptr;

            pc = state->pc;
            ir = state->ir;
            dt = state->dt;
            st = state->st;
            sp = state->sp;
            stack = state->stack;
            didUpdate = true;
            vgaState = c8::vga::VgaState{state->vgaState};

            std::memcpy(v, state->v, 16);
        }

        std::uint8_t* getRegister(const std::uint8_t index)
        {
            if (index > 0xF) {
                return nullptr;
            }

            return v + index;
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

        void CLS()
        {
            vgaState.clear();
            pc += 2;
        }

        void RET()
        {
            const std::uint16_t addr = popFromStack();

            pc = addr + 2;
        }

        void JP_Addr(const std::uint16_t addr)
        {
            if (pc == addr) {
                didUpdate = false;
            } else {
                pc = addr;
            }
        }

        void CALL_Addr(const std::uint16_t addr)
        {
            pushToStack(pc);
            pc = addr;
        }

        void SE_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            if (*vx == value) {
                pc += 2;
            }

            pc += 2;
        }

        void SNE_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            if (*vx != value) {
                pc += 2;
            }

            pc += 2;
        }


        void SE_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            if (*vx == *vy) {
                pc += 2;
            }

            pc += 2;
        }

        void LD_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            *vx = value;
            pc += 2;
        }

        void ADD_Vx_Byte(const std::uint8_t x, const std::uint8_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            *vx += value;
            pc += 2;
        }

        void LD_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            *vx = *vy;
            pc += 2;
        }

        void OR_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            *vx = *vx | *vy;
            pc += 2;
        }

        void AND_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            *vx = *vx & *vy;
            pc += 2;
        }

        void XOR_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            *vx = *vx ^ *vy;
            pc += 2;
        }

        void ADD_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            const auto result = (int)*vx + (int)*vy;

            v[15] = result > 255 ? 1 : 0;
            *vx = static_cast<std::uint8_t>(result);

            pc += 2;
        }

        void SUB_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            const auto result = *vx - *vy;

            v[15] = *vx > *vy ? 1 : 0;

            *vx = static_cast<std::uint8_t>(result);
            pc += 2;
        }

        void SHR_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            if (c8::quirks::shiftWithVy) {
                *vx = *vy;
            }

            const auto bit = *vx & 0b0000'0001;

            v[15] = bit >= 1 ? 1 : 0;

            *vx = *vx >> 1;
            pc += 2;
        }

        void SUBN_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            const auto result = *vy - *vx;

            v[15] = *vy > *vx ? 1 : 0;

            *vx = static_cast<std::uint8_t>(result);   
            pc += 2;
        }

        void SHL_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            if (c8::quirks::shiftWithVy) {
                *vx = *vy;
            }

            const auto bit = *vx & 0b1000'0000;

            v[15] = bit >= 1 ? 1 : 0;

            *vx = *vx << 1;
            pc += 2;
        }

        void SNE_Vx_Vy(const std::uint8_t x, const std::uint8_t y)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr) {
                return;
            }

            if (*vx != *vy) {
                pc += 2;
            }

            pc += 2;
        }

        void LD_I_Addr(const std::uint16_t value) 
        {
            ir = value;
            pc += 2;
        }

        void JP_V0_Addr(const std::uint16_t value)
        {
            pc = value + v[0];
        }

        void RND_Vx_Byte(const std::uint8_t x, const std::uint16_t value)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            const std::uint8_t randomValue = distribution(generator);

            *vx = randomValue & value;
            pc += 2;
        }

        void DRW_Vx_Vy_Nibble(const std::uint8_t x, const std::uint8_t y, const std::uint8_t n)
        {
            std::uint8_t* vx = getRegister(x);
            std::uint8_t* vy = getRegister(y);

            if (vx == nullptr || vy == nullptr || n == 0) {
                return;
            }

            bool didErase = false;

            for (int i = 0; i < n; i++) {
                const std::uint8_t byte = c8::mem::readByte(ir + i);

                didErase = vgaState.drawByte(*vx, *vy + i, byte) || didErase;
            }

            v[15] = didErase ? 1 : 0;
            pc += 2;
        }

        void SKP_Vx(const std::uint8_t x) 
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            const std::uint8_t input = getCurrentKeyboardValue();

            if (*vx == input) {
                pc += 2;
            }

            pc += 2;
        }

        void SKNP_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            const std::uint8_t input = getCurrentKeyboardValue();

            if (*vx != input) {
                pc += 2;
            }

            pc += 2;
        }

        void LD_Vx_DT(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            *vx = dt;
            pc += 2;
        }

        void LD_Vx_K(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            waitingForKeyboard = true;

            if (keyboardPressedValue == 0xFF) {
                didUpdate = false;
                return;
            }

            *vx = keyboardPressedValue;
            pc += 2;

            keyboardPressedValue = 0xFF;
            waitingForKeyboard = false;
        }

        void LD_DT_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            dt = *vx;
            pc += 2;
        }

        void LD_ST_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            st = *vx;
            pc += 2;
        }

        void ADD_I_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            ir = ir + *vx;
            pc += 2;
        }

        void LD_F_Vx(const std::uint8_t x)
        {
            std::uint8_t* vx = getRegister(x);

            if (vx == nullptr) {
                return;
            }

            ir = c8::mem::getFontSpriteAddress(*vx);
            pc += 2;
        }

        void LD_B_Vx(const std::uint8_t x)
        {
            std::cout << "LD B V" << (int)x << std::endl;
        }

        void LD_IAddr_Vx(const std::uint8_t x)
        {
            for (std::uint8_t i = 0; i <= x; i++) {
                std::uint8_t* vx = getRegister(i);

                c8::mem::writeByte(ir + i, *vx);
            }

            if (c8::quirks::memoryIncrementI) {
                ir += x + 1;
            }

            pc += 2;
        }

        void LD_Vx_IAddr(const std::uint8_t x)
        {
            for (std::uint8_t i = 0; i <= x; i++) {
                std::uint8_t* vx = getRegister(i);

                *vx = c8::mem::readByte(ir + i);
            }

            if (c8::quirks::memoryIncrementI) {
                ir += x + 1;
            }

            pc += 2;
        }
    };

    CpuState* oldestCpuState;
    CpuState* cpuState;
    CpuState* nextCpuState;

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

    void destroy()
    {
        CpuState* stateToDelete = cpuState;

        while (stateToDelete != nullptr) { 
            CpuState* previousState = stateToDelete->previousState;

            delete stateToDelete;

            stateToDelete = previousState;
        }
    }

    void reset()
    {
        destroy();

        cpuStateIndex = 0;
        cpuStates = 0;
        cpuState = new CpuState{};
        cpuState->pc = 0x200;
        cpuState->vgaState.clear();

        oldestCpuState = cpuState;

        c8::mem::reset();
    }

    void keyboardKeyPressed(std::uint8_t value)
    {
        if (!waitingForKeyboard) {
            return;
        }

        std::cout << (int)value << std::endl;

        keyboardPressedValue = value;
    }

    std::uint16_t getProgramCounter()
    {
        return cpuState->pc;
    }

    void togglePaused()
    {
        paused = !paused;

        if (paused) {
        } else {
        }
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

        if (cpuState->previousState == nullptr) {
            return;
        }

        cpuStateIndex--;
        cpuState = cpuState->previousState;
    }

    void drawCpuInfo(sf::RenderTexture& texture)
    {
        std::stringstream ss;

        ss << "PC = " << c8::ui::Hex{cpuState->pc} << "\t" << "I = " << c8::ui::Hex{cpuState->ir} << "\n\n";
        ss << "DT = " << c8::ui::Hex{cpuState->dt} << "\t" << "ST = " << c8::ui::Hex{cpuState->st} << "\n\n";
        ss << "V0 = " << c8::ui::Hex{cpuState->v[0x0]} << "\tV8 = " << c8::ui::Hex{cpuState->v[0x8]} << "\n";
        ss << "V1 = " << c8::ui::Hex{cpuState->v[0x1]} << "\tV9 = " << c8::ui::Hex{cpuState->v[0x9]} << "\n";
        ss << "V2 = " << c8::ui::Hex{cpuState->v[0x2]} << "\tVA = " << c8::ui::Hex{cpuState->v[0xA]} << "\n";
        ss << "V3 = " << c8::ui::Hex{cpuState->v[0x3]} << "\tVB = " << c8::ui::Hex{cpuState->v[0xB]} << "\n";
        ss << "V4 = " << c8::ui::Hex{cpuState->v[0x4]} << "\tVC = " << c8::ui::Hex{cpuState->v[0xC]} << "\n";
        ss << "V5 = " << c8::ui::Hex{cpuState->v[0x5]} << "\tVD = " << c8::ui::Hex{cpuState->v[0xD]} << "\n";
        ss << "V6 = " << c8::ui::Hex{cpuState->v[0x6]} << "\tVE = " << c8::ui::Hex{cpuState->v[0xE]} << "\n";
        ss << "V7 = " << c8::ui::Hex{cpuState->v[0x7]} << "\tVF = " << c8::ui::Hex{cpuState->v[0xF]} << "\n\n";
        ss << "Emulator State = " << (paused ? "PAUSED" : "RUNNING") << "\n";
        ss << "Current CPU State = " << cpuStateIndex << "/" << maxCpuStates << "\n";
        ss << "CPU Frequency = " << cpuHertz << "Hz" << "\n";
        ss << "Render Speed = " << hostFps << "FPS" << "\n\n";
        ss << "Controls:\n";
        ss << "P = start/pause emulator" << "\n";
        ss << "Left/Right = forward/backward 1 CPU cycle";

        c8::ui::drawText(texture, 0, 0, ss);
    }

    void drawVga(sf::RenderTexture& texture)
    {
        cpuState->vgaState.render(texture);
    }

    void draw(sf::RenderTexture& vgaTexture, sf::RenderTexture& cpuInfoTexture)
    {
        drawVga(vgaTexture);
        drawCpuInfo(cpuInfoTexture);
    }

    void decrementTimers()
    {
        if (paused) {
            return;
        }

        if (cpuState->dt > 0) {
            cpuState->dt--;
        }

        if (cpuState->st > 0) {
            cpuState->st--;
        }
    }

    void processOpcode(const std::uint16_t word)
    {
        const c8::opcodes::Opcode opcode = c8::opcodes::decode(word);

        const std::uint8_t x = c8::opcodes::getOpcodeX(word);
        const std::uint8_t y = c8::opcodes::getOpcodeY(word);
        const std::uint8_t z = c8::opcodes::getOpcodeZ(word);

        const std::uint8_t kk = c8::opcodes::getOpcodeKK(word);
        const std::uint16_t nnn = c8::opcodes::getOpcodeNNN(word);

        switch (opcode) {
        case c8::opcodes::Opcode::CLS:
            return nextCpuState->CLS();
        case c8::opcodes::Opcode::RET:
            return nextCpuState->RET();
        case c8::opcodes::Opcode::JP_Addr:
            return nextCpuState->JP_Addr(nnn);
        case c8::opcodes::Opcode::CALL_Addr:
            return nextCpuState->CALL_Addr(nnn);
        case c8::opcodes::Opcode::SE_Vx_Byte:
            return nextCpuState->SE_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::SNE_Vx_Byte:
            return nextCpuState->SNE_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::SE_Vx_Vy:
            return nextCpuState->SE_Vx_Vy(x, y);
        case c8::opcodes::Opcode::LD_Vx_Byte:
            return nextCpuState->LD_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::ADD_Vx_Byte:
            return nextCpuState->ADD_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::LD_Vx_Vy:
            return nextCpuState->LD_Vx_Vy(x, y);
        case c8::opcodes::Opcode::OR_Vx_Vy:
            return nextCpuState->OR_Vx_Vy(x, y);
        case c8::opcodes::Opcode::AND_Vx_Vy:
            return nextCpuState->AND_Vx_Vy(x, y);
        case c8::opcodes::Opcode::XOR_Vx_Vy:
            return nextCpuState->XOR_Vx_Vy(x, y);
        case c8::opcodes::Opcode::ADD_Vx_Vy:
            return nextCpuState->ADD_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SUB_Vx_Vy:
            return nextCpuState->SUB_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SHR_Vx_Vy:
            return nextCpuState->SHR_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SUBN_Vx_Vy:
            return nextCpuState->SUBN_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SHL_Vx_Vy:
            return nextCpuState->SHL_Vx_Vy(x, y);
        case c8::opcodes::Opcode::SNE_Vx_Vy:
            return nextCpuState->SNE_Vx_Vy(x, y);
        case c8::opcodes::Opcode::LD_I_Addr:
            return nextCpuState->LD_I_Addr(nnn);
        case c8::opcodes::Opcode::JP_V0_Addr:
            return nextCpuState->JP_V0_Addr(nnn);
        case c8::opcodes::Opcode::RND_Vx_Byte:
            return nextCpuState->RND_Vx_Byte(x, kk);
        case c8::opcodes::Opcode::DRW_Vx_Vy_Nibble:
            return nextCpuState->DRW_Vx_Vy_Nibble(x, y, z);
        case c8::opcodes::Opcode::SKP_Vx:
            return nextCpuState->SKP_Vx(x);
        case c8::opcodes::Opcode::SKNP_Vx:
            return nextCpuState->SKNP_Vx(x);
        case c8::opcodes::Opcode::LD_Vx_DT:
            return nextCpuState->LD_Vx_DT(x);
        case c8::opcodes::Opcode::LD_Vx_K:
            return nextCpuState->LD_Vx_K(x);
        case c8::opcodes::Opcode::LD_DT_Vx:
            return nextCpuState->LD_DT_Vx(x);
        case c8::opcodes::Opcode::LD_ST_Vx:
            return nextCpuState->LD_ST_Vx(x);
        case c8::opcodes::Opcode::ADD_I_Vx:
            return nextCpuState->ADD_I_Vx(x);
        case c8::opcodes::Opcode::LD_F_Vx:
            return nextCpuState->LD_F_Vx(x);
        case c8::opcodes::Opcode::LD_B_Vx:
            return nextCpuState->LD_B_Vx(x);
        case c8::opcodes::Opcode::LD_IAddr_Vx:
            return nextCpuState->LD_IAddr_Vx(x);
        case c8::opcodes::Opcode::LD_Vx_IAddr:
            return nextCpuState->LD_Vx_IAddr(x);
        case c8::opcodes::Opcode::Invalid:
            return;
        }
    }

    void cleanupOldCpuStates()
    {
        while (cpuStates > maxCpuStates) {
            CpuState* newOldestCpuState = oldestCpuState->nextState;

            newOldestCpuState->previousState = nullptr;
            oldestCpuState->nextState = nullptr;
            oldestCpuState->previousState = nullptr;

            delete oldestCpuState;

            oldestCpuState = newOldestCpuState;
            cpuStates--;
            cpuStateIndex--;
        }
    }

    void executeClockCycle()
    {
        if (paused && !doAdvanceOneClockCycle) {
            return;
        }

        doAdvanceOneClockCycle = false;

        if (cpuState->nextState != nullptr) {
            cpuState = cpuState->nextState;
            cpuStateIndex++;

            return;
        }

        std::uint16_t opcode = c8::mem::readWord(cpuState->pc);

        if (opcode == 0x0){
            return;
        }

        nextCpuState = new CpuState{cpuState};

        processOpcode(opcode);

        if (!nextCpuState->didUpdate){
            delete nextCpuState;
            
            return;
        }

        cpuStateIndex++;
        cpuStates++;
        cpuState->nextState = nextCpuState;
        cpuState = nextCpuState;

        cleanupOldCpuStates();
    }
}
