//
// Copyright 2020 Christoph Sprenger
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <age_debug.hpp>

#include "age_gb_cpu.hpp"

namespace
{

    constexpr uint8_t gb_zero_flag       = 0x80;
    constexpr uint8_t gb_subtract_flag   = 0x40;
    constexpr uint8_t gb_half_carry_flag = 0x20;
    constexpr uint8_t gb_carry_flag      = 0x10;

    constexpr unsigned gb_hcs_shift      = 4;
    constexpr int      gb_hcs_half_carry = gb_half_carry_flag << gb_hcs_shift;
    constexpr int      gb_hcs_subtract   = gb_subtract_flag << gb_hcs_shift;
    constexpr int      gb_hcs_old_carry  = gb_carry_flag << gb_hcs_shift;
    constexpr int      gb_hcs_flags      = gb_hcs_half_carry + gb_hcs_subtract;

} // namespace



//---------------------------------------------------------
//
//   we use preprocessor macros to keep the
//   instruction switch statement readable
//
//---------------------------------------------------------

#define CB_BIT(opcode) (1 << (((opcode) >> 3) & 0x07))

#define LOAD_BC ((m_b << 8) + m_c)
#define LOAD_DE ((m_d << 8) + m_e)
#define LOAD_HL ((m_h << 8) + m_l)

#define STORE_HL(value)              \
    {                                \
        m_h = ((value) >> 8) & 0xFF; \
        m_l = (value) &0xFF;         \
    }                                \
    (void) 0 // no-op to force semicolon when using this macro

#define TICK_MACHINE_CYCLE m_clock.tick_machine_cycle()



// ----- CPU flags

#define CARRY_INDICATOR_FLAG (m_carry_indicator & 0x100)

#define ZERO_FLAGGED  ((m_zero_indicator & 0xFF) == 0)
#define CARRY_FLAGGED (CARRY_INDICATOR_FLAG != 0)

#define CALCULATE_HALF_CARRY_SUBTRACT_FLAGS(destination)                                          \
    {                                                                                             \
        int operand1  = m_hcs_flags & 0xF;                                                        \
        int operand2  = (m_hcs_operand & 0xF) + ((m_hcs_flags & gb_hcs_old_carry) >> 8);          \
        operand2      = (m_hcs_flags & gb_hcs_subtract) > 0 ? -operand2 : operand2;               \
        int result    = operand1 + operand2;                                                      \
        (destination) = (m_hcs_flags & ~gb_hcs_half_carry) + ((result << 5) & gb_hcs_half_carry); \
    }                                                                                             \
    (void) 0 // no-op to force semicolon when using this macro

#define STORE_FLAGS_TO(destination)                                  \
    {                                                                \
        int hcs_flags;                                               \
        CALCULATE_HALF_CARRY_SUBTRACT_FLAGS(hcs_flags);              \
        hcs_flags      = (hcs_flags & gb_hcs_flags) >> gb_hcs_shift; \
        int zero_flag  = ZERO_FLAGGED ? gb_zero_flag : 0;            \
        int carry_flag = CARRY_INDICATOR_FLAG >> gb_hcs_shift;       \
        int flags      = zero_flag + carry_flag + hcs_flags;         \
        (destination)  = flags & 0xFF;                               \
    }                                                                \
    (void) 0 // no-op to force semicolon when using this macro

#define LOAD_FLAGS_FROM(source)                                            \
    {                                                                      \
        int tmp           = source;                                        \
        m_zero_indicator  = ~tmp & gb_zero_flag;                           \
        m_carry_indicator = (tmp & gb_carry_flag) << gb_hcs_shift;         \
        m_hcs_flags       = ((tmp << gb_hcs_shift) & gb_hcs_flags) + 0x08; \
        m_hcs_operand     = (tmp & gb_half_carry_flag) > 0 ? 0x0F : 0;     \
    }                                                                      \
    (void) 0 // no-op to force semicolon when using this macro



// ----- memory access

#define READ_BYTE(destination, address)                     \
    {                                                       \
        TICK_MACHINE_CYCLE;                                 \
        (destination) = m_bus.read_byte((address) &0xFFFF); \
    }                                                       \
    (void) 0 // no-op to force semicolon when using this macro

#define WRITE_BYTE(address, value)                          \
    {                                                       \
        TICK_MACHINE_CYCLE;                                 \
        m_bus.write_byte((address) &0xFFFF, (value) &0xFF); \
    }                                                       \
    (void) 0 // no-op to force semicolon when using this macro

#define WRITE_WORD(address, value)                                   \
    {                                                                \
        WRITE_BYTE((address) &0xFFFF, (value) &0xFF);                \
        WRITE_BYTE(((address) + 1) & 0xFFFF, ((value) >> 8) & 0xFF); \
    }                                                                \
    (void) 0 // no-op to force semicolon when using this macro

#define POP_BYTE_AT_PC(destination)   \
    {                                 \
        READ_BYTE(destination, m_pc); \
        ++m_pc;                       \
    }                                 \
    (void) 0 // no-op to force semicolon when using this macro

#define POP_SIGNED_BYTE_AT_PC(destination)             \
    {                                                  \
        POP_BYTE_AT_PC(destination);                   \
        (destination) = ((destination) ^ 0x80) - 0x80; \
    }                                                  \
    (void) 0 // no-op to force semicolon when using this macro

#define POP_WORD_AT_PC(destination)        \
    {                                      \
        int high, low;                     \
        POP_BYTE_AT_PC(low);               \
        POP_BYTE_AT_PC(high);              \
        (destination) = low + (high << 8); \
    }                                      \
    (void) 0 // no-op to force semicolon when using this macro

#define PUSH_BYTE(byte)         \
    {                           \
        --m_sp;                 \
        WRITE_BYTE(m_sp, byte); \
    }                           \
    (void) 0 // no-op to force semicolon when using this macro

#define POP_BYTE(destination)         \
    {                                 \
        READ_BYTE(destination, m_sp); \
        ++m_sp;                       \
    }                                 \
    (void) 0 // no-op to force semicolon when using this macro

#define PUSH_PC               \
    {                         \
        PUSH_BYTE(m_pc >> 8); \
        PUSH_BYTE(m_pc);      \
    }                         \
    (void) 0 // no-op to force semicolon when using this macro



// ----- jumps

// RST
#define RST(opcode)            \
    {                          \
        TICK_MACHINE_CYCLE;    \
        PUSH_PC;               \
        m_pc = (opcode) &0x38; \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// JP
#define JP                                   \
    {                                        \
        int low, high;                       \
        POP_BYTE_AT_PC(low);                 \
        POP_BYTE_AT_PC(high);                \
        m_pc = (low + (high << 8)) & 0xFFFF; \
        TICK_MACHINE_CYCLE;                  \
    }                                        \
    (void) 0 // no-op to force semicolon when using this macro

// JP <cond>
#define JP_IF(condition)    \
    if (condition)          \
    {                       \
        JP;                 \
    }                       \
    else                    \
    {                       \
        m_pc += 2;          \
        TICK_MACHINE_CYCLE; \
        TICK_MACHINE_CYCLE; \
    }                       \
    (void) 0 // no-op to force semicolon when using this macro

// CALL
#define CALL                    \
    {                           \
        int ret_pc = m_pc + 2;  \
        JP;                     \
        PUSH_BYTE(ret_pc >> 8); \
        PUSH_BYTE(ret_pc);      \
    }                           \
    (void) 0 // no-op to force semicolon when using this macro

// CALL <cond>
#define CALL_IF(condition)  \
    if (condition)          \
    {                       \
        CALL;               \
    }                       \
    else                    \
    {                       \
        m_pc += 2;          \
        TICK_MACHINE_CYCLE; \
        TICK_MACHINE_CYCLE; \
    }                       \
    (void) 0 // no-op to force semicolon when using this macro

// RET
#define RET                                  \
    {                                        \
        int low, high;                       \
        POP_BYTE(low);                       \
        POP_BYTE(high);                      \
        m_pc = (low + (high << 8)) & 0xFFFF; \
        TICK_MACHINE_CYCLE;                  \
    }                                        \
    (void) 0 // no-op to force semicolon when using this macro

// RET <cond>
#define RET_IF(condition)   \
    {                       \
        TICK_MACHINE_CYCLE; \
        if (condition)      \
        {                   \
            RET;            \
        }                   \
    }                       \
    (void) 0 // no-op to force semicolon when using this macro

// JR
#define JR                               \
    {                                    \
        int offset;                      \
        POP_SIGNED_BYTE_AT_PC(offset);   \
        m_pc = (offset + m_pc) & 0xFFFF; \
        TICK_MACHINE_CYCLE;              \
    }                                    \
    (void) 0 // no-op to force semicolon when using this macro

// JR <cond>
#define JR_IF(condition)    \
    if (condition)          \
    {                       \
        JR;                 \
    }                       \
    else                    \
    {                       \
        ++m_pc;             \
        TICK_MACHINE_CYCLE; \
    }                       \
    (void) 0 // no-op to force semicolon when using this macro



// ----- increment & decrement

// INC 8 bit value
// carry = unmodified, subtract = cleared, zero & half calculated
#define INC_8BIT(value)                         \
    {                                           \
        m_hcs_flags = m_zero_indicator = value; \
        m_hcs_operand                  = 1;     \
        ++m_zero_indicator;                     \
    }                                           \
    (void) 0 // no-op to force semicolon when using this macro

#define INC_REG(value)                     \
    {                                      \
        INC_8BIT(value);                   \
        (value) = m_zero_indicator & 0xFF; \
    }                                      \
    (void) 0 // no-op to force semicolon when using this macro

#define INC_MEM_HL                        \
    {                                     \
        int value;                        \
        int hl = LOAD_HL;                 \
        READ_BYTE(value, hl);             \
        INC_8BIT(value);                  \
        WRITE_BYTE(hl, m_zero_indicator); \
    }                                     \
    (void) 0 // no-op to force semicolon when using this macro

// DEC 8 bit value
// carry = unmodified, subtract = set, zero & half calculated
#define DEC(value)                                    \
    {                                                 \
        m_zero_indicator = value;                     \
        m_hcs_flags      = (value) + gb_hcs_subtract; \
        m_hcs_operand    = 1;                         \
        --m_zero_indicator;                           \
    }                                                 \
    (void) 0 // no-op to force semicolon when using this macro

#define DEC_REG(value)                     \
    {                                      \
        DEC(value);                        \
        (value) = m_zero_indicator & 0xFF; \
    }                                      \
    (void) 0 // no-op to force semicolon when using this macro

#define DEC_MEM_HL                        \
    {                                     \
        int value;                        \
        int hl = LOAD_HL;                 \
        READ_BYTE(value, hl);             \
        DEC(value);                       \
        WRITE_BYTE(hl, m_zero_indicator); \
    }                                     \
    (void) 0 // no-op to force semicolon when using this macro

// INC & DEC 16 bit value
#define ADD_TO_BYTES(high, low, value)          \
    {                                           \
        int tmp = (low) + (value);              \
        (low)   = tmp & 0xFF;                   \
        (high)  = ((high) + (tmp >> 8)) & 0xFF; \
        TICK_MACHINE_CYCLE;                     \
    }                                           \
    (void) 0 // no-op to force semicolon when using this macro

#define INC_BYTES(high, low) ADD_TO_BYTES(high, low, 1)
#define DEC_BYTES(high, low) ADD_TO_BYTES(high, low, -1)



// ----- arithmetic

// ADD & ADC 8 bit value
// subtract = cleared, carry & zero & half calculated
#define ADD(value)                                                      \
    {                                                                   \
        m_hcs_flags       = m_a;                                        \
        m_hcs_operand     = value;                                      \
        m_carry_indicator = m_zero_indicator = m_a + m_hcs_operand;     \
        m_a                                  = m_zero_indicator & 0xFF; \
    }                                                                   \
    (void) 0 // no-op to force semicolon when using this macro

#define ADC(value)                                                                 \
    {                                                                              \
        int carry         = CARRY_INDICATOR_FLAG;                                  \
        m_hcs_flags       = m_a + carry;                                           \
        m_hcs_operand     = value;                                                 \
        m_carry_indicator = m_zero_indicator = m_a + m_hcs_operand + (carry >> 8); \
        m_a                                  = m_zero_indicator & 0xFF;            \
    }                                                                              \
    (void) 0 // no-op to force semicolon when using this macro

#define ADD_MEM(address)           \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        ADD(value);                \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

#define ADC_MEM(address)           \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        ADC(value);                \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

// ADD to HL
// subtract = cleared, zero = unaffected, carry & half calculated
#define ADD_TO_HL(high, low)                                  \
    {                                                         \
        int tmp = m_l;                                        \
        tmp += (low);                                         \
        m_hcs_flags       = m_h + (tmp & gb_hcs_old_carry);   \
        m_hcs_operand     = high;                             \
        m_carry_indicator = m_h + m_hcs_operand + (tmp >> 8); \
        m_h               = m_carry_indicator & 0xFF;         \
        m_l               = tmp & 0xFF;                       \
        TICK_MACHINE_CYCLE;                                   \
    }                                                         \
    (void) 0 // no-op to force semicolon when using this macro

// ADD signed value to SP
// zero = subtract = cleared, carry & half calculated
#define ADD_SP                                           \
    {                                                    \
        int value;                                       \
        POP_SIGNED_BYTE_AT_PC(value);                    \
        m_hcs_operand     = value & 0xFF;                \
        m_hcs_flags       = m_sp & 0xFF;                 \
        m_carry_indicator = m_hcs_operand + m_hcs_flags; \
        m_zero_indicator  = 1;                           \
        m_sp              = (m_sp + value) & 0xFFFF;     \
    }                                                    \
    (void) 0 // no-op to force semicolon when using this macro

#define ADD_TO_SP           \
    {                       \
        ADD_SP;             \
        TICK_MACHINE_CYCLE; \
        TICK_MACHINE_CYCLE; \
    }                       \
    (void) 0 // no-op to force semicolon when using this macro

// SUB & SBC 8 bit value
// subtract = set, carry & zero & half calculated
#define SUB(value)                                                      \
    {                                                                   \
        m_hcs_flags       = m_a + gb_hcs_subtract;                      \
        m_hcs_operand     = value;                                      \
        m_carry_indicator = m_zero_indicator = m_a - m_hcs_operand;     \
        m_a                                  = m_zero_indicator & 0xFF; \
    }                                                                   \
    (void) 0 // no-op to force semicolon when using this macro

#define SBC(value)                                                                 \
    {                                                                              \
        int carry         = CARRY_INDICATOR_FLAG;                                  \
        m_hcs_flags       = m_a + carry + gb_hcs_subtract;                         \
        m_hcs_operand     = value;                                                 \
        m_carry_indicator = m_zero_indicator = m_a - m_hcs_operand - (carry >> 8); \
        m_a                                  = m_zero_indicator & 0xFF;            \
    }                                                                              \
    (void) 0 // no-op to force semicolon when using this macro

#define SUB_MEM(address)           \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        SUB(value);                \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

#define SBC_MEM(address)           \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        SBC(value);                \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

// AND 8 bit value
// carry = subtract = cleared, half = set, zero calculated
#define AND(value)                                             \
    {                                                          \
        m_zero_indicator  = m_a & (value);                     \
        m_carry_indicator = 0;                                 \
        m_hcs_flags = m_hcs_operand = 0x08;                    \
        m_a                         = m_zero_indicator & 0xFF; \
    }                                                          \
    (void) 0 // no-op to force semicolon when using this macro

#define AND_MEM(address)           \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        AND(value);                \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

// XOR 8 bit value
// carry = subtract = half = cleared, zero calculated
#define XOR(value)                                                                 \
    {                                                                              \
        m_zero_indicator  = m_a ^ (value);                                         \
        m_carry_indicator = m_hcs_flags = m_hcs_operand = 0;                       \
        m_a                                             = m_zero_indicator & 0xFF; \
    }                                                                              \
    (void) 0 // no-op to force semicolon when using this macro

#define XOR_MEM(address)           \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        XOR(value);                \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

// OR 8 bit value
// carry = subtract = half = cleared, zero calculated
#define OR(value)                                                                  \
    {                                                                              \
        m_zero_indicator  = m_a | (value);                                         \
        m_carry_indicator = m_hcs_flags = m_hcs_operand = 0;                       \
        m_a                                             = m_zero_indicator & 0xFF; \
    }                                                                              \
    (void) 0 // no-op to force semicolon when using this macro

#define OR_MEM(address)            \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        OR(value);                 \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

// CP 8 bit value
// subtract = set, carry & half & zero calculated
#define CP(value)                                                \
    {                                                            \
        m_carry_indicator = m_a;                                 \
        m_hcs_flags       = m_carry_indicator + gb_hcs_subtract; \
        m_hcs_operand     = value;                               \
        m_carry_indicator -= m_hcs_operand;                      \
        m_zero_indicator = m_carry_indicator;                    \
    }                                                            \
    (void) 0 // no-op to force semicolon when using this macro

#define CP_MEM(address)            \
    {                              \
        int value;                 \
        READ_BYTE(value, address); \
        CP(value);                 \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro



// ----- loads

// LD 8 bit value from memory
#define LD_IMM8_MEM_HL              \
    {                               \
        int value;                  \
        POP_BYTE_AT_PC(value);      \
        WRITE_BYTE(LOAD_HL, value); \
    }                               \
    (void) 0 // no-op to force semicolon when using this macro

// LD HL, SP + n
// flags set according to ADD SP, n
#define LD_HL_SP_ADD               \
    {                              \
        uint16_t sp_bak = m_sp;    \
        ADD_SP;                    \
        m_h  = (m_sp >> 8) & 0xFF; \
        m_l  = m_sp & 0xFF;        \
        m_sp = sp_bak;             \
        TICK_MACHINE_CYCLE;        \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro


// ----- bit operations

// bit-test 8 bit value
// carry = unmodified, subtract = cleared, half = set, zero calculated
#define BIT(value, opcode)                          \
    {                                               \
        m_zero_indicator = (value) &CB_BIT(opcode); \
        m_hcs_flags = m_hcs_operand = 0x08;         \
    }                                               \
    (void) 0 // no-op to force semicolon when using this macro

#define BIT_MEM_HL(opcode)         \
    {                              \
        int value;                 \
        READ_BYTE(value, LOAD_HL); \
        BIT(value, opcode);        \
    }                              \
    (void) 0 // no-op to force semicolon when using this macro

// reset specific bit in 8 bit value
// carry, zero, subtract, half = unmodified
#define RES_MEM_HL(opcode)        \
    {                             \
        int value;                \
        int hl = LOAD_HL;         \
        READ_BYTE(value, hl);     \
        value &= ~CB_BIT(opcode); \
        WRITE_BYTE(hl, value);    \
    }                             \
    (void) 0 // no-op to force semicolon when using this macro

// set bit in 8 bit value
// carry, zero, subtract, half = unmodified
#define SET_MEM_HL(opcode)       \
    {                            \
        int value;               \
        int hl = LOAD_HL;        \
        READ_BYTE(value, hl);    \
        value |= CB_BIT(opcode); \
        WRITE_BYTE(hl, value);   \
    }                            \
    (void) 0 // no-op to force semicolon when using this macro



// ----- shifts & rotations

// RLC 8 bit value
// half & subtract = cleared, carry = old bit 7, zero calculated (RLCA: zero = cleared)
#define RLC(value)                                                        \
    {                                                                     \
        m_hcs_flags       = 0;                                            \
        m_carry_indicator = (value) << 1;                                 \
        m_zero_indicator  = m_carry_indicator + (m_carry_indicator >> 8); \
        (value)           = m_zero_indicator & 0xFF;                      \
    }                                                                     \
    (void) 0 // no-op to force semicolon when using this macro

#define RLCA                  \
    {                         \
        RLC(m_a);             \
        m_zero_indicator = 1; \
    }                         \
    (void) 0 // no-op to force semicolon when using this macro

#define RLC_MEM_HL             \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        RLC(value);            \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// RL 8 bit value
// half & subtract = cleared, carry = old bit 7, zero calculated (RLA: zero = cleared)
#define RL(value)                                              \
    {                                                          \
        m_hcs_flags       = 0;                                 \
        int old_carry_bit = CARRY_INDICATOR_FLAG >> 8;         \
        m_carry_indicator = (value) << 1;                      \
        m_zero_indicator  = m_carry_indicator + old_carry_bit; \
        (value)           = m_zero_indicator & 0xFF;           \
    }                                                          \
    (void) 0 // no-op to force semicolon when using this macro

#define RLA                   \
    {                         \
        RL(m_a);              \
        m_zero_indicator = 1; \
    }                         \
    (void) 0 // no-op to force semicolon when using this macro

#define RL_MEM_HL              \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        RL(value);             \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// RRC 8 bit value
// half & subtract = cleared, carry = old bit 0, zero calculated
#define RRC(value)                                                                \
    {                                                                             \
        m_hcs_flags       = 0;                                                    \
        m_zero_indicator  = value;                                                \
        m_carry_indicator = m_zero_indicator << 8;                                \
        (value)           = ((m_carry_indicator + m_zero_indicator) >> 1) & 0xFF; \
    }                                                                             \
    (void) 0 // no-op to force semicolon when using this macro

#define RRCA                  \
    {                         \
        RRC(m_a);             \
        m_zero_indicator = 1; \
    }                         \
    (void) 0 // no-op to force semicolon when using this macro

#define RRC_MEM_HL             \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        RRC(value);            \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// RR 8 bit value
// half & subtract = cleared, carry = old bit 0, zero calculated
#define RR(value)                                           \
    {                                                       \
        m_hcs_flags       = 0;                              \
        m_zero_indicator  = (value) + CARRY_INDICATOR_FLAG; \
        m_carry_indicator = m_zero_indicator << 8;          \
        m_zero_indicator >>= 1;                             \
        (value) = m_zero_indicator & 0xFF;                  \
    }                                                       \
    (void) 0 // no-op to force semicolon when using this macro

#define RRA                   \
    {                         \
        RR(m_a);              \
        m_zero_indicator = 1; \
    }                         \
    (void) 0 // no-op to force semicolon when using this macro

#define RR_MEM_HL              \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        RR(value);             \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// SLA 8 bit value
// half & subtract = cleared, carry & zero calculated
#define SLA(value)                                   \
    {                                                \
        m_hcs_flags       = 0;                       \
        m_zero_indicator  = (value) << 1;            \
        m_carry_indicator = m_zero_indicator;        \
        (value)           = m_zero_indicator & 0xFF; \
    }                                                \
    (void) 0 // no-op to force semicolon when using this macro

#define SLA_MEM_HL             \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        SLA(value);            \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// SRA 8 bit value
// half & subtract = cleared, carry & zero calculated
#define SRA(value)                                 \
    {                                              \
        m_hcs_flags       = 0;                     \
        m_zero_indicator  = value;                 \
        m_carry_indicator = m_zero_indicator << 8; \
        m_zero_indicator >>= 1;                    \
        m_zero_indicator += (value) &0x80;         \
        (value) = m_zero_indicator & 0xFF;         \
    }                                              \
    (void) 0 // no-op to force semicolon when using this macro

#define SRA_MEM_HL             \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        SRA(value);            \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// SRL 8 bit value
// half & subtract = cleared, carry & zero calculated
#define SRL(value)                                 \
    {                                              \
        m_hcs_flags       = 0;                     \
        m_zero_indicator  = value;                 \
        m_carry_indicator = m_zero_indicator << 8; \
        m_zero_indicator >>= 1;                    \
        (value) = m_zero_indicator & 0xFF;         \
    }                                              \
    (void) 0 // no-op to force semicolon when using this macro

#define SRL_MEM_HL             \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        SRL(value);            \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro

// SWAP 8 bit value
// half & subtract & carry = cleared, zero calculated
#define SWAP(value)                                                        \
    {                                                                      \
        m_hcs_flags = m_carry_indicator = 0;                               \
        m_zero_indicator                = ((value) << 4) + ((value) >> 4); \
        (value)                         = m_zero_indicator & 0xFF;         \
    }                                                                      \
    (void) 0 // no-op to force semicolon when using this macro

#define SWAP_MEM_HL            \
    {                          \
        int value;             \
        int hl = LOAD_HL;      \
        READ_BYTE(value, hl);  \
        SWAP(value);           \
        WRITE_BYTE(hl, value); \
    }                          \
    (void) 0 // no-op to force semicolon when using this macro





//---------------------------------------------------------
//
//   utility methods
//
//---------------------------------------------------------

void age::gb_cpu::set_flags(int from_value)
{
    LOAD_FLAGS_FROM(from_value);
}

void age::gb_cpu::tick_push_byte(int byte)
{
    PUSH_BYTE(byte);
}

age::uint8_t age::gb_cpu::tick_read_byte(int address)
{
    uint8_t result;
    READ_BYTE(result, address);
    return result;
}





//---------------------------------------------------------
//
//   cpu instruction emulation
//
//---------------------------------------------------------

void age::gb_cpu::execute_prefetched()
{
    AGE_ASSERT(!(m_cpu_state & gb_cpu_state_frozen))

    // Let PC point to the byte after the prefetched opcode.
    // HALT bug: let PC point to the opcode after HALT
    //           (that opcode is also already prefetched)
    m_pc++;

    int opcode = m_prefetched_opcode;
    switch (opcode)
    {
        default:
            // invalid opcode, stay here and freeze CPU
            --m_pc;
            m_cpu_state |= gb_cpu_state_frozen;
            m_invalid_opcode = opcode;
            m_clock.log(gb_log_category::lc_cpu) << "invalid opcode " << log_hex8(opcode)
                                                 << " at PC == " << log_hex16(m_pc)
                                                 << ", cpu frozen";
            break;

            // increment & decrement

        case 0x04: INC_REG(m_b); break;
        case 0x0C: INC_REG(m_c); break;
        case 0x14: INC_REG(m_d); break;
        case 0x1C: INC_REG(m_e); break;
        case 0x24: INC_REG(m_h); break;
        case 0x2C: INC_REG(m_l); break;
        case 0x34: INC_MEM_HL; break;
        case 0x3C: INC_REG(m_a); break;

        case 0x05: DEC_REG(m_b); break;
        case 0x0D: DEC_REG(m_c); break;
        case 0x15: DEC_REG(m_d); break;
        case 0x1D: DEC_REG(m_e); break;
        case 0x25: DEC_REG(m_h); break;
        case 0x2D: DEC_REG(m_l); break;
        case 0x35: DEC_MEM_HL; break;
        case 0x3D: DEC_REG(m_a); break;

        case 0x03: INC_BYTES(m_b, m_c); break; // INC BC
        case 0x13: INC_BYTES(m_d, m_e); break; // INC DE
        case 0x23: INC_BYTES(m_h, m_l); break; // INC HL
        case 0x33:
            ++m_sp;
            TICK_MACHINE_CYCLE;
            break; // INC SP

        case 0x0B: DEC_BYTES(m_b, m_c); break; // DEC BC
        case 0x1B: DEC_BYTES(m_d, m_e); break; // DEC DE
        case 0x2B: DEC_BYTES(m_h, m_l); break; // DEC HL
        case 0x3B:
            --m_sp;
            TICK_MACHINE_CYCLE;
            break; // DEC SP

            // loads

        case 0x06: POP_BYTE_AT_PC(m_b); break; // LD B, x
        case 0x0E: POP_BYTE_AT_PC(m_c); break; // LD C, x
        case 0x16: POP_BYTE_AT_PC(m_d); break; // LD D, x
        case 0x1E: POP_BYTE_AT_PC(m_e); break; // LD E, x
        case 0x26: POP_BYTE_AT_PC(m_h); break; // LD H, x
        case 0x2E: POP_BYTE_AT_PC(m_l); break; // LD L, x
        case 0x36: LD_IMM8_MEM_HL; break;      // LD [HL], x
        case 0x3E: POP_BYTE_AT_PC(m_a); break; // LD A, x

        case 0x40: m_ld_b_b = true; break;         // LD B, B
        case 0x41: m_b = m_c; break;               // LD B, C
        case 0x42: m_b = m_d; break;               // LD B, D
        case 0x43: m_b = m_e; break;               // LD B, E
        case 0x44: m_b = m_h; break;               // LD B, H
        case 0x45: m_b = m_l; break;               // LD B, L
        case 0x46: READ_BYTE(m_b, LOAD_HL); break; // LD B, [HL]
        case 0x47: m_b = m_a; break;               // LD B, A

        case 0x48: m_c = m_b; break;
        case 0x49: break;
        case 0x4A: m_c = m_d; break;
        case 0x4B: m_c = m_e; break;
        case 0x4C: m_c = m_h; break;
        case 0x4D: m_c = m_l; break;
        case 0x4E: READ_BYTE(m_c, LOAD_HL); break;
        case 0x4F: m_c = m_a; break;

        case 0x50: m_d = m_b; break;
        case 0x51: m_d = m_c; break;
        case 0x52: break;
        case 0x53: m_d = m_e; break;
        case 0x54: m_d = m_h; break;
        case 0x55: m_d = m_l; break;
        case 0x56: READ_BYTE(m_d, LOAD_HL); break;
        case 0x57: m_d = m_a; break;

        case 0x58: m_e = m_b; break;
        case 0x59: m_e = m_c; break;
        case 0x5A: m_e = m_d; break;
        case 0x5B: break;
        case 0x5C: m_e = m_h; break;
        case 0x5D: m_e = m_l; break;
        case 0x5E: READ_BYTE(m_e, LOAD_HL); break;
        case 0x5F: m_e = m_a; break;

        case 0x60: m_h = m_b; break;
        case 0x61: m_h = m_c; break;
        case 0x62: m_h = m_d; break;
        case 0x63: m_h = m_e; break;
        case 0x64: break;
        case 0x65: m_h = m_l; break;
        case 0x66: READ_BYTE(m_h, LOAD_HL); break;
        case 0x67: m_h = m_a; break;

        case 0x68: m_l = m_b; break;
        case 0x69: m_l = m_c; break;
        case 0x6A: m_l = m_d; break;
        case 0x6B: m_l = m_e; break;
        case 0x6C: m_l = m_h; break;
        case 0x6D: break;
        case 0x6E: READ_BYTE(m_l, LOAD_HL); break;
        case 0x6F: m_l = m_a; break;

        case 0x70: WRITE_BYTE(LOAD_HL, m_b); break; // LD [HL], B
        case 0x71: WRITE_BYTE(LOAD_HL, m_c); break; // LD [HL], C
        case 0x72: WRITE_BYTE(LOAD_HL, m_d); break; // LD [HL], D
        case 0x73: WRITE_BYTE(LOAD_HL, m_e); break; // LD [HL], E
        case 0x74: WRITE_BYTE(LOAD_HL, m_h); break; // LD [HL], H
        case 0x75: WRITE_BYTE(LOAD_HL, m_l); break; // LD [HL], L
        case 0x77: WRITE_BYTE(LOAD_HL, m_a); break; // LD [HL], A

        case 0x78: m_a = m_b; break;
        case 0x79: m_a = m_c; break;
        case 0x7A: m_a = m_d; break;
        case 0x7B: m_a = m_e; break;
        case 0x7C: m_a = m_h; break;
        case 0x7D: m_a = m_l; break;
        case 0x7E: READ_BYTE(m_a, LOAD_HL); break;
        case 0x7F: break;

        case 0x02: WRITE_BYTE(LOAD_BC, m_a); break; // LD [BC], A
        case 0x0A: READ_BYTE(m_a, LOAD_BC); break;  // LD A, [BC]
        case 0x12: WRITE_BYTE(LOAD_DE, m_a); break; // LD [DE], A
        case 0x1A: READ_BYTE(m_a, LOAD_DE); break;  // LD A, [DE]

        case 0xF8: LD_HL_SP_ADD; break; // LD HL, SP + x
        case 0xF9:
            m_sp = LOAD_HL & 0xFFFF;
            TICK_MACHINE_CYCLE;
            break; // LD SP, HL
        case 0x08: {
            int address;
            POP_WORD_AT_PC(address);
            WRITE_WORD(address, m_sp);
        }
        break; // LD [xx], SP

        case 0xE0: {
            uint8_t offset;
            POP_BYTE_AT_PC(offset);
            WRITE_BYTE(0xFF00 + offset, m_a);
        }
        break; // LDH [x], A
        case 0xF0: {
            uint8_t offset;
            POP_BYTE_AT_PC(offset);
            READ_BYTE(m_a, 0xFF00 + offset);
        }
        break;                                           // LDH A, [x]
        case 0xE2: WRITE_BYTE(0xFF00 + m_c, m_a); break; // LDH [C], A
        case 0xF2: READ_BYTE(m_a, 0xFF00 + m_c); break;  // LDH A, [C]
        case 0xEA: {
            int address;
            POP_WORD_AT_PC(address);
            WRITE_BYTE(address, m_a);
        }
        break; // LD [xx], A
        case 0xFA: {
            int address;
            POP_WORD_AT_PC(address);
            READ_BYTE(m_a, address);
        }
        break; // LD A, [xx]
        case 0x22: {
            int hl = LOAD_HL;
            WRITE_BYTE(hl, m_a);
            ++hl;
            STORE_HL(hl);
        }
        break; // LDI [HL], A
        case 0x32: {
            int hl = LOAD_HL;
            WRITE_BYTE(hl, m_a);
            --hl;
            STORE_HL(hl);
        }
        break; // LDD [HL], A
        case 0x2A: {
            int hl = LOAD_HL;
            READ_BYTE(m_a, hl);
            ++hl;
            STORE_HL(hl);
        }
        break; // LDI A, [HL]
        case 0x3A: {
            int hl = LOAD_HL;
            READ_BYTE(m_a, hl);
            --hl;
            STORE_HL(hl);
        }
        break; // LDD A, [HL]

        case 0x01:
            POP_BYTE_AT_PC(m_c);
            POP_BYTE_AT_PC(m_b);
            break; // LD BC, xx
        case 0x11:
            POP_BYTE_AT_PC(m_e);
            POP_BYTE_AT_PC(m_d);
            break; // LD DE, xx
        case 0x21:
            POP_BYTE_AT_PC(m_l);
            POP_BYTE_AT_PC(m_h);
            break; // LD HL, xx
        case 0x31: {
            int h, l;
            POP_BYTE_AT_PC(l);
            POP_BYTE_AT_PC(h);
            m_sp = (l + (h << 8)) & 0xFFFF;
        }
        break; // LD AF, xx

            // arithmetic

        case 0xC6:
            ADD_MEM(m_pc);
            ++m_pc;
            break;
        case 0x80: ADD(m_b); break;
        case 0x81: ADD(m_c); break;
        case 0x82: ADD(m_d); break;
        case 0x83: ADD(m_e); break;
        case 0x84: ADD(m_h); break;
        case 0x85: ADD(m_l); break;
        case 0x86: ADD_MEM(LOAD_HL); break;
        case 0x87: ADD(m_a); break;

        case 0xE8: ADD_TO_SP; break;
        case 0x09: ADD_TO_HL(m_b, m_c); break;
        case 0x19: ADD_TO_HL(m_d, m_e); break;
        case 0x29: ADD_TO_HL(m_h, m_l); break;
        case 0x39: ADD_TO_HL(m_sp >> 8, m_sp & 0xFF); break;

        case 0xCE:
            ADC_MEM(m_pc);
            ++m_pc;
            break;
        case 0x88: ADC(m_b); break;
        case 0x89: ADC(m_c); break;
        case 0x8A: ADC(m_d); break;
        case 0x8B: ADC(m_e); break;
        case 0x8C: ADC(m_h); break;
        case 0x8D: ADC(m_l); break;
        case 0x8E: ADC_MEM(LOAD_HL); break;
        case 0x8F: ADC(m_a); break;

        case 0xD6:
            SUB_MEM(m_pc);
            ++m_pc;
            break;
        case 0x90: SUB(m_b); break;
        case 0x91: SUB(m_c); break;
        case 0x92: SUB(m_d); break;
        case 0x93: SUB(m_e); break;
        case 0x94: SUB(m_h); break;
        case 0x95: SUB(m_l); break;
        case 0x96: SUB_MEM(LOAD_HL); break;
        case 0x97: SUB(m_a); break;

        case 0xDE:
            SBC_MEM(m_pc);
            ++m_pc;
            break;
        case 0x98: SBC(m_b); break;
        case 0x99: SBC(m_c); break;
        case 0x9A: SBC(m_d); break;
        case 0x9B: SBC(m_e); break;
        case 0x9C: SBC(m_h); break;
        case 0x9D: SBC(m_l); break;
        case 0x9E: SBC_MEM(LOAD_HL); break;
        case 0x9F: SBC(m_a); break;

        case 0xE6:
            AND_MEM(m_pc);
            ++m_pc;
            break;
        case 0xA0: AND(m_b); break;
        case 0xA1: AND(m_c); break;
        case 0xA2: AND(m_d); break;
        case 0xA3: AND(m_e); break;
        case 0xA4: AND(m_h); break;
        case 0xA5: AND(m_l); break;
        case 0xA6: AND_MEM(LOAD_HL); break;
        case 0xA7: AND(m_a); break;

        case 0xEE:
            XOR_MEM(m_pc);
            ++m_pc;
            break;
        case 0xA8: XOR(m_b); break;
        case 0xA9: XOR(m_c); break;
        case 0xAA: XOR(m_d); break;
        case 0xAB: XOR(m_e); break;
        case 0xAC: XOR(m_h); break;
        case 0xAD: XOR(m_l); break;
        case 0xAE: XOR_MEM(LOAD_HL); break;
        case 0xAF: XOR(m_a); break;

        case 0xF6:
            OR_MEM(m_pc);
            ++m_pc;
            break;
        case 0xB0: OR(m_b); break;
        case 0xB1: OR(m_c); break;
        case 0xB2: OR(m_d); break;
        case 0xB3: OR(m_e); break;
        case 0xB4: OR(m_h); break;
        case 0xB5: OR(m_l); break;
        case 0xB6: OR_MEM(LOAD_HL); break;
        case 0xB7: OR(m_a); break;

        case 0xFE:
            CP_MEM(m_pc);
            ++m_pc;
            break;
        case 0xB8: CP(m_b); break;
        case 0xB9: CP(m_c); break;
        case 0xBA: CP(m_d); break;
        case 0xBB: CP(m_e); break;
        case 0xBC: CP(m_h); break;
        case 0xBD: CP(m_l); break;
        case 0xBE: CP_MEM(LOAD_HL); break;
        case 0xBF:
            CP(m_a);
            break;

            // jumps

        case 0xC7: RST(0xC7); break;
        case 0xCF: RST(0xCF); break;
        case 0xD7: RST(0xD7); break;
        case 0xDF: RST(0xDF); break;
        case 0xE7: RST(0xE7); break;
        case 0xEF: RST(0xEF); break;
        case 0xF7: RST(0xF7); break;
        case 0xFF: RST(0xFF); break;

        case 0xCD: CALL; break;
        case 0xC4: CALL_IF(!ZERO_FLAGGED); break;
        case 0xCC: CALL_IF(ZERO_FLAGGED); break;
        case 0xD4: CALL_IF(!CARRY_FLAGGED); break;
        case 0xDC: CALL_IF(CARRY_FLAGGED); break;

        case 0xC3: JP; break;
        case 0xE9: m_pc = LOAD_HL & 0xFFFF; break;
        case 0xC2: JP_IF(!ZERO_FLAGGED); break;
        case 0xCA: JP_IF(ZERO_FLAGGED); break;
        case 0xD2: JP_IF(!CARRY_FLAGGED); break;
        case 0xDA: JP_IF(CARRY_FLAGGED); break;

        case 0xC9: RET; break;

        case 0xD9: // RETI
            RET;
            m_interrupts.set_ime(true, "RETI");
            break;

        case 0xC0: RET_IF(!ZERO_FLAGGED); break;
        case 0xC8: RET_IF(ZERO_FLAGGED); break;
        case 0xD0: RET_IF(!CARRY_FLAGGED); break;
        case 0xD8: RET_IF(CARRY_FLAGGED); break;

        case 0x18: JR; break;
        case 0x20: JR_IF(!ZERO_FLAGGED); break;
        case 0x28: JR_IF(ZERO_FLAGGED); break;
        case 0x30: JR_IF(!CARRY_FLAGGED); break;
        case 0x38:
            JR_IF(CARRY_FLAGGED);
            break;

            // stack (push & pop)

        case 0xC5:
            TICK_MACHINE_CYCLE;
            PUSH_BYTE(m_b);
            PUSH_BYTE(m_c);
            break; // PUSH BC
        case 0xD5:
            TICK_MACHINE_CYCLE;
            PUSH_BYTE(m_d);
            PUSH_BYTE(m_e);
            break; // PUSH DE
        case 0xE5:
            TICK_MACHINE_CYCLE;
            PUSH_BYTE(m_h);
            PUSH_BYTE(m_l);
            break; // PUSH HL
        case 0xF5:
            TICK_MACHINE_CYCLE;
            PUSH_BYTE(m_a);
            {
                uint8_t f;
                STORE_FLAGS_TO(f);
                PUSH_BYTE(f);
            }
            break; // PUSH AF

        case 0xC1:
            POP_BYTE(m_c);
            POP_BYTE(m_b);
            break; // POP BC
        case 0xD1:
            POP_BYTE(m_e);
            POP_BYTE(m_d);
            break; // POP DE
        case 0xE1:
            POP_BYTE(m_l);
            POP_BYTE(m_h);
            break; // POP HL
        case 0xF1: {
            uint8_t f;
            POP_BYTE(f);
            LOAD_FLAGS_FROM(f);
        }
            POP_BYTE(m_a);
            break; // POP AF

            // misc

        case 0x07: RLCA; break;
        case 0x0F: RRCA; break;
        case 0x17: RLA; break;
        case 0x1F: RRA; break;

        case 0x00: break; // NOP

        case 0x10: { // STOP
            m_interrupts.log() << "STOP encountered (no STOP M-cycle executed yet)";
            READ_BYTE(m_prefetched_opcode, m_pc);
            m_interrupts.log() << "STOP: prefetched op code " << log_hex8(m_prefetched_opcode);
            TICK_MACHINE_CYCLE;
            m_bus.execute_stop();
            if (m_interrupts.halt())
            {
                // same number of machine cycles, different number of T4 cycles
                int clk_offset = m_clock.is_double_speed() ? 0x10000 : 0x20000;
                m_events.schedule_event(gb_event::unhalt, clk_offset);
                m_interrupts.log() << "STOP: HALT mode period after STOP lasts until clock cycle "
                                   << m_events.get_event_cycle(gb_event::unhalt);
            }
            else
            {
                m_interrupts.log() << "STOP: HALT mode period after STOP terminated immediately";
            }
            return;
        }

        case 0x2F:
            m_a           = ~m_a;
            m_hcs_flags   = gb_hcs_subtract;
            m_hcs_operand = 1;
            break; // CPL
        case 0x37:
            m_carry_indicator = 0x100;
            m_hcs_flags = m_hcs_operand = 0;
            break; // SCF
        case 0x3F:
            m_carry_indicator ^= 0x100;
            m_hcs_flags = m_hcs_operand = 0;
            break; // CCF

        case 0x76: { // HALT
            auto msg = m_interrupts.log();
            msg << "executing HALT instruction";

            m_clock.tick_machine_cycle();
            m_prefetched_opcode = m_bus.read_byte(m_pc);

            m_bus.handle_events(); // make sure the IF register is up-to-date
            bool halted = m_interrupts.halt();
            if (halted)
            {
                msg << "\n    * CPU HALTed";
                if (m_device.is_dmg_device())
                {
                    msg << "\n    * applying extra DMG HALT delay (2 m-cycles)";
                    m_clock.tick_machine_cycle();
                    m_clock.tick_machine_cycle();
                    m_bus.handle_events(); // interrupt may terminate HALT
                }
                return;
            }

            msg << "\n    * HALT immediately terminated by pending interrupts"
                << "\n    * \"HALT bug\", decrementing PC";
            // IRQ: handler returns to HALT instruction
            // else: PC is incremented for next instruction
            //       and points to the byte after HALT
            --m_pc;
            return;
        }

        case 0xF3: // DI
            m_interrupts.set_ime(false, "DI");
            m_cpu_state &= ~gb_cpu_state_ei;
            break;

        case 0xFB: // EI
            if (!m_interrupts.get_ime())
            {
                m_interrupts.log() << "EI encountered: enable interrupt dispatching after the next CPU instruction";
                m_cpu_state |= gb_cpu_state_ei;
            }
            break;

        case 0x27: // DAA
        {
            uint8_t f;
            STORE_FLAGS_TO(f);
            // calculate correction based on carry flags
            uint8_t correction = ((f & gb_carry_flag) > 0) ? 0x60 : 0;
            if ((f & gb_half_carry_flag) > 0)
            {
                correction += 0x06;
            }
            // adjust subtraction
            if ((f & gb_subtract_flag) > 0)
            {
                m_a -= correction;
            }
            // adjust addition
            else
            {
                if ((m_a & 0x0F) > 0x09)
                {
                    correction |= 0x06;
                }
                if (m_a > 0x99)
                {
                    correction |= 0x60;
                }
                m_a += correction;
            }
            // fix flags
            f &= gb_subtract_flag;
            if (m_a == 0)
            {
                f |= gb_zero_flag;
            }
            if (correction >= 0x60)
            {
                f |= gb_carry_flag;
            }
            LOAD_FLAGS_FROM(f);
            break;
        }

            // CB instructions

        case 0xCB: {
            POP_BYTE_AT_PC(opcode);

            switch (opcode)
            {
                    // rotates & shifts

                case 0x00: RLC(m_b); break;
                case 0x01: RLC(m_c); break;
                case 0x02: RLC(m_d); break;
                case 0x03: RLC(m_e); break;
                case 0x04: RLC(m_h); break;
                case 0x05: RLC(m_l); break;
                case 0x06: RLC_MEM_HL; break;
                case 0x07: RLC(m_a); break;

                case 0x08: RRC(m_b); break;
                case 0x09: RRC(m_c); break;
                case 0x0A: RRC(m_d); break;
                case 0x0B: RRC(m_e); break;
                case 0x0C: RRC(m_h); break;
                case 0x0D: RRC(m_l); break;
                case 0x0E: RRC_MEM_HL; break;
                case 0x0F: RRC(m_a); break;

                case 0x10: RL(m_b); break;
                case 0x11: RL(m_c); break;
                case 0x12: RL(m_d); break;
                case 0x13: RL(m_e); break;
                case 0x14: RL(m_h); break;
                case 0x15: RL(m_l); break;
                case 0x16: RL_MEM_HL; break;
                case 0x17: RL(m_a); break;

                case 0x18: RR(m_b); break;
                case 0x19: RR(m_c); break;
                case 0x1A: RR(m_d); break;
                case 0x1B: RR(m_e); break;
                case 0x1C: RR(m_h); break;
                case 0x1D: RR(m_l); break;
                case 0x1E: RR_MEM_HL; break;
                case 0x1F: RR(m_a); break;

                case 0x20: SLA(m_b); break;
                case 0x21: SLA(m_c); break;
                case 0x22: SLA(m_d); break;
                case 0x23: SLA(m_e); break;
                case 0x24: SLA(m_h); break;
                case 0x25: SLA(m_l); break;
                case 0x26: SLA_MEM_HL; break;
                case 0x27: SLA(m_a); break;

                case 0x28: SRA(m_b); break;
                case 0x29: SRA(m_c); break;
                case 0x2A: SRA(m_d); break;
                case 0x2B: SRA(m_e); break;
                case 0x2C: SRA(m_h); break;
                case 0x2D: SRA(m_l); break;
                case 0x2E: SRA_MEM_HL; break;
                case 0x2F: SRA(m_a); break;

                case 0x30: SWAP(m_b); break;
                case 0x31: SWAP(m_c); break;
                case 0x32: SWAP(m_d); break;
                case 0x33: SWAP(m_e); break;
                case 0x34: SWAP(m_h); break;
                case 0x35: SWAP(m_l); break;
                case 0x36: SWAP_MEM_HL; break;
                case 0x37: SWAP(m_a); break;

                case 0x38: SRL(m_b); break;
                case 0x39: SRL(m_c); break;
                case 0x3A: SRL(m_d); break;
                case 0x3B: SRL(m_e); break;
                case 0x3C: SRL(m_h); break;
                case 0x3D: SRL(m_l); break;
                case 0x3E: SRL_MEM_HL; break;
                case 0x3F:
                    SRL(m_a);
                    break;

                    // bit

                case 0x40: BIT(m_b, 0x40); break;
                case 0x41: BIT(m_c, 0x41); break;
                case 0x42: BIT(m_d, 0x42); break;
                case 0x43: BIT(m_e, 0x43); break;
                case 0x44: BIT(m_h, 0x44); break;
                case 0x45: BIT(m_l, 0x45); break;
                case 0x46: BIT_MEM_HL(0x46); break;
                case 0x47: BIT(m_a, 0x47); break;

                case 0x48: BIT(m_b, 0x48); break;
                case 0x49: BIT(m_c, 0x49); break;
                case 0x4A: BIT(m_d, 0x4A); break;
                case 0x4B: BIT(m_e, 0x4B); break;
                case 0x4C: BIT(m_h, 0x4C); break;
                case 0x4D: BIT(m_l, 0x4D); break;
                case 0x4E: BIT_MEM_HL(0x4E); break;
                case 0x4F: BIT(m_a, 0x4F); break;

                case 0x50: BIT(m_b, 0x50); break;
                case 0x51: BIT(m_c, 0x51); break;
                case 0x52: BIT(m_d, 0x52); break;
                case 0x53: BIT(m_e, 0x53); break;
                case 0x54: BIT(m_h, 0x54); break;
                case 0x55: BIT(m_l, 0x55); break;
                case 0x56: BIT_MEM_HL(0x56); break;
                case 0x57: BIT(m_a, 0x57); break;

                case 0x58: BIT(m_b, 0x58); break;
                case 0x59: BIT(m_c, 0x59); break;
                case 0x5A: BIT(m_d, 0x5A); break;
                case 0x5B: BIT(m_e, 0x5B); break;
                case 0x5C: BIT(m_h, 0x5C); break;
                case 0x5D: BIT(m_l, 0x5D); break;
                case 0x5E: BIT_MEM_HL(0x5E); break;
                case 0x5F: BIT(m_a, 0x5F); break;

                case 0x60: BIT(m_b, 0x60); break;
                case 0x61: BIT(m_c, 0x61); break;
                case 0x62: BIT(m_d, 0x62); break;
                case 0x63: BIT(m_e, 0x63); break;
                case 0x64: BIT(m_h, 0x64); break;
                case 0x65: BIT(m_l, 0x65); break;
                case 0x66: BIT_MEM_HL(0x66); break;
                case 0x67: BIT(m_a, 0x67); break;

                case 0x68: BIT(m_b, 0x68); break;
                case 0x69: BIT(m_c, 0x69); break;
                case 0x6A: BIT(m_d, 0x6A); break;
                case 0x6B: BIT(m_e, 0x6B); break;
                case 0x6C: BIT(m_h, 0x6C); break;
                case 0x6D: BIT(m_l, 0x6D); break;
                case 0x6E: BIT_MEM_HL(0x6E); break;
                case 0x6F: BIT(m_a, 0x6F); break;

                case 0x70: BIT(m_b, 0x70); break;
                case 0x71: BIT(m_c, 0x71); break;
                case 0x72: BIT(m_d, 0x72); break;
                case 0x73: BIT(m_e, 0x73); break;
                case 0x74: BIT(m_h, 0x74); break;
                case 0x75: BIT(m_l, 0x75); break;
                case 0x76: BIT_MEM_HL(0x76); break;
                case 0x77: BIT(m_a, 0x77); break;

                case 0x78: BIT(m_b, 0x78); break;
                case 0x79: BIT(m_c, 0x79); break;
                case 0x7A: BIT(m_d, 0x7A); break;
                case 0x7B: BIT(m_e, 0x7B); break;
                case 0x7C: BIT(m_h, 0x7C); break;
                case 0x7D: BIT(m_l, 0x7D); break;
                case 0x7E: BIT_MEM_HL(0x7E); break;
                case 0x7F:
                    BIT(m_a, 0x7F);
                    break;

                    // res

                case 0x80: m_b &= ~CB_BIT(0x80); break;
                case 0x81: m_c &= ~CB_BIT(0x81); break;
                case 0x82: m_d &= ~CB_BIT(0x82); break;
                case 0x83: m_e &= ~CB_BIT(0x83); break;
                case 0x84: m_h &= ~CB_BIT(0x84); break;
                case 0x85: m_l &= ~CB_BIT(0x85); break;
                case 0x86: RES_MEM_HL(0x86); break;
                case 0x87: m_a &= ~CB_BIT(0x87); break;

                case 0x88: m_b &= ~CB_BIT(0x88); break;
                case 0x89: m_c &= ~CB_BIT(0x89); break;
                case 0x8A: m_d &= ~CB_BIT(0x8A); break;
                case 0x8B: m_e &= ~CB_BIT(0x8B); break;
                case 0x8C: m_h &= ~CB_BIT(0x8C); break;
                case 0x8D: m_l &= ~CB_BIT(0x8D); break;
                case 0x8E: RES_MEM_HL(0x8E); break;
                case 0x8F: m_a &= ~CB_BIT(0x8F); break;

                case 0x90: m_b &= ~CB_BIT(0x90); break;
                case 0x91: m_c &= ~CB_BIT(0x91); break;
                case 0x92: m_d &= ~CB_BIT(0x92); break;
                case 0x93: m_e &= ~CB_BIT(0x93); break;
                case 0x94: m_h &= ~CB_BIT(0x94); break;
                case 0x95: m_l &= ~CB_BIT(0x95); break;
                case 0x96: RES_MEM_HL(0x96); break;
                case 0x97: m_a &= ~CB_BIT(0x97); break;

                case 0x98: m_b &= ~CB_BIT(0x98); break;
                case 0x99: m_c &= ~CB_BIT(0x99); break;
                case 0x9A: m_d &= ~CB_BIT(0x9A); break;
                case 0x9B: m_e &= ~CB_BIT(0x9B); break;
                case 0x9C: m_h &= ~CB_BIT(0x9C); break;
                case 0x9D: m_l &= ~CB_BIT(0x9D); break;
                case 0x9E: RES_MEM_HL(0x9E); break;
                case 0x9F: m_a &= ~CB_BIT(0x9F); break;

                case 0xA0: m_b &= ~CB_BIT(0xA0); break;
                case 0xA1: m_c &= ~CB_BIT(0xA1); break;
                case 0xA2: m_d &= ~CB_BIT(0xA2); break;
                case 0xA3: m_e &= ~CB_BIT(0xA3); break;
                case 0xA4: m_h &= ~CB_BIT(0xA4); break;
                case 0xA5: m_l &= ~CB_BIT(0xA5); break;
                case 0xA6: RES_MEM_HL(0xA6); break;
                case 0xA7: m_a &= ~CB_BIT(0xA7); break;

                case 0xA8: m_b &= ~CB_BIT(0xA8); break;
                case 0xA9: m_c &= ~CB_BIT(0xA9); break;
                case 0xAA: m_d &= ~CB_BIT(0xAA); break;
                case 0xAB: m_e &= ~CB_BIT(0xAB); break;
                case 0xAC: m_h &= ~CB_BIT(0xAC); break;
                case 0xAD: m_l &= ~CB_BIT(0xAD); break;
                case 0xAE: RES_MEM_HL(0xAE); break;
                case 0xAF: m_a &= ~CB_BIT(0xAF); break;

                case 0xB0: m_b &= ~CB_BIT(0xB0); break;
                case 0xB1: m_c &= ~CB_BIT(0xB1); break;
                case 0xB2: m_d &= ~CB_BIT(0xB2); break;
                case 0xB3: m_e &= ~CB_BIT(0xB3); break;
                case 0xB4: m_h &= ~CB_BIT(0xB4); break;
                case 0xB5: m_l &= ~CB_BIT(0xB5); break;
                case 0xB6: RES_MEM_HL(0xB6); break;
                case 0xB7: m_a &= ~CB_BIT(0xB7); break;

                case 0xB8: m_b &= ~CB_BIT(0xB8); break;
                case 0xB9: m_c &= ~CB_BIT(0xB9); break;
                case 0xBA: m_d &= ~CB_BIT(0xBA); break;
                case 0xBB: m_e &= ~CB_BIT(0xBB); break;
                case 0xBC: m_h &= ~CB_BIT(0xBC); break;
                case 0xBD: m_l &= ~CB_BIT(0xBD); break;
                case 0xBE: RES_MEM_HL(0xBE); break;
                case 0xBF:
                    m_a &= ~CB_BIT(0xBF);
                    break;

                    // set

                case 0xC0: m_b |= CB_BIT(0xC0); break;
                case 0xC1: m_c |= CB_BIT(0xC1); break;
                case 0xC2: m_d |= CB_BIT(0xC2); break;
                case 0xC3: m_e |= CB_BIT(0xC3); break;
                case 0xC4: m_h |= CB_BIT(0xC4); break;
                case 0xC5: m_l |= CB_BIT(0xC5); break;
                case 0xC6: SET_MEM_HL(0xC6); break;
                case 0xC7: m_a |= CB_BIT(0xC7); break;

                case 0xC8: m_b |= CB_BIT(0xC8); break;
                case 0xC9: m_c |= CB_BIT(0xC9); break;
                case 0xCA: m_d |= CB_BIT(0xCA); break;
                case 0xCB: m_e |= CB_BIT(0xCB); break;
                case 0xCC: m_h |= CB_BIT(0xCC); break;
                case 0xCD: m_l |= CB_BIT(0xCD); break;
                case 0xCE: SET_MEM_HL(0xCE); break;
                case 0xCF: m_a |= CB_BIT(0xCF); break;

                case 0xD0: m_b |= CB_BIT(0xD0); break;
                case 0xD1: m_c |= CB_BIT(0xD1); break;
                case 0xD2: m_d |= CB_BIT(0xD2); break;
                case 0xD3: m_e |= CB_BIT(0xD3); break;
                case 0xD4: m_h |= CB_BIT(0xD4); break;
                case 0xD5: m_l |= CB_BIT(0xD5); break;
                case 0xD6: SET_MEM_HL(0xD6); break;
                case 0xD7: m_a |= CB_BIT(0xD7); break;

                case 0xD8: m_b |= CB_BIT(0xD8); break;
                case 0xD9: m_c |= CB_BIT(0xD9); break;
                case 0xDA: m_d |= CB_BIT(0xDA); break;
                case 0xDB: m_e |= CB_BIT(0xDB); break;
                case 0xDC: m_h |= CB_BIT(0xDC); break;
                case 0xDD: m_l |= CB_BIT(0xDD); break;
                case 0xDE: SET_MEM_HL(0xDE); break;
                case 0xDF: m_a |= CB_BIT(0xDF); break;

                case 0xE0: m_b |= CB_BIT(0xE0); break;
                case 0xE1: m_c |= CB_BIT(0xE1); break;
                case 0xE2: m_d |= CB_BIT(0xE2); break;
                case 0xE3: m_e |= CB_BIT(0xE3); break;
                case 0xE4: m_h |= CB_BIT(0xE4); break;
                case 0xE5: m_l |= CB_BIT(0xE5); break;
                case 0xE6: SET_MEM_HL(0xE6); break;
                case 0xE7: m_a |= CB_BIT(0xE7); break;

                case 0xE8: m_b |= CB_BIT(0xE8); break;
                case 0xE9: m_c |= CB_BIT(0xE9); break;
                case 0xEA: m_d |= CB_BIT(0xEA); break;
                case 0xEB: m_e |= CB_BIT(0xEB); break;
                case 0xEC: m_h |= CB_BIT(0xEC); break;
                case 0xED: m_l |= CB_BIT(0xED); break;
                case 0xEE: SET_MEM_HL(0xEE); break;
                case 0xEF: m_a |= CB_BIT(0xEF); break;

                case 0xF0: m_b |= CB_BIT(0xF0); break;
                case 0xF1: m_c |= CB_BIT(0xF1); break;
                case 0xF2: m_d |= CB_BIT(0xF2); break;
                case 0xF3: m_e |= CB_BIT(0xF3); break;
                case 0xF4: m_h |= CB_BIT(0xF4); break;
                case 0xF5: m_l |= CB_BIT(0xF5); break;
                case 0xF6: SET_MEM_HL(0xF6); break;
                case 0xF7: m_a |= CB_BIT(0xF7); break;

                case 0xF8: m_b |= CB_BIT(0xF8); break;
                case 0xF9: m_c |= CB_BIT(0xF9); break;
                case 0xFA: m_d |= CB_BIT(0xFA); break;
                case 0xFB: m_e |= CB_BIT(0xFB); break;
                case 0xFC: m_h |= CB_BIT(0xFC); break;
                case 0xFD: m_l |= CB_BIT(0xFD); break;
                case 0xFE: SET_MEM_HL(0xFE); break;
                case 0xFF: m_a |= CB_BIT(0xFF); break;
            }
            break;
        }
    }

    READ_BYTE(m_prefetched_opcode, m_pc);
}
