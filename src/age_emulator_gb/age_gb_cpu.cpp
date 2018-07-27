//
// Copyright 2018 Christoph Sprenger
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

#if 0
#define LOG(x) AGE_GB_CYCLE_LOG(x)
#else
#define LOG(x)
#endif

namespace age {

constexpr uint8 gb_zero_flag = 0x80;
constexpr uint8 gb_subtract_flag = 0x40;
constexpr uint8 gb_half_carry_flag = 0x20;
constexpr uint8 gb_carry_flag = 0x10;

constexpr uint gb_hcs_shift = 4;
constexpr uint gb_hcs_half_carry = gb_half_carry_flag << gb_hcs_shift;
constexpr uint gb_hcs_subtract = gb_subtract_flag << gb_hcs_shift;
constexpr uint gb_hcs_old_carry = gb_carry_flag << gb_hcs_shift;
constexpr uint gb_hcs_flags = gb_hcs_half_carry + gb_hcs_subtract;

}



//---------------------------------------------------------
//
//   we use preprocessor macros to keep the
//   instruction switch statement as small
//   as possible
//
//---------------------------------------------------------

#define CB_BIT(opcode) (1 << ((opcode >> 3) & 0x07))

#define CARRY_INDICATOR_FLAG (m_carry_indicator & 0x100)

#define LOAD_BC ((static_cast<uint16>(m_b) << 8) + m_c)
#define LOAD_DE ((static_cast<uint16>(m_d) << 8) + m_e)
#define LOAD_HL ((static_cast<uint16>(m_h) << 8) + m_l)

#define STORE_HL(value) { \
    m_h = static_cast<uint8>(value >> 8); \
    m_l = static_cast<uint8>(value); \
    }

#define INC_CYCLES m_core.oscillate_cpu_cycle()



// ----- CPU flags

#define ZERO_FLAGGED ((m_zero_indicator & 0xFF) == 0)
#define CARRY_FLAGGED (CARRY_INDICATOR_FLAG != 0)

#define CALCULATE_HALF_CARRY_SUBTRACT_FLAGS(destination) { \
    uint _operand1 = m_hcs_flags & 0xF; \
    uint _operand2 = (m_hcs_operand & 0xF) + ((m_hcs_flags & gb_hcs_old_carry) >> 8); \
    _operand2 = (m_hcs_flags & gb_hcs_subtract) > 0 ? -_operand2 : _operand2; \
    uint _result = _operand1 + _operand2; \
    destination = (m_hcs_flags & ~gb_hcs_half_carry) + ((_result << 5) & gb_hcs_half_carry); \
    }

#define STORE_FLAGS_TO(destination) { \
    uint _hcs_flags; \
    CALCULATE_HALF_CARRY_SUBTRACT_FLAGS(_hcs_flags) \
    _hcs_flags = (_hcs_flags & gb_hcs_flags) >> gb_hcs_shift; \
    uint _zero_flag = ZERO_FLAGGED ? gb_zero_flag : 0; \
    uint _carry_flag = CARRY_INDICATOR_FLAG >> gb_hcs_shift; \
    uint _flags = _zero_flag + _carry_flag + _hcs_flags; \
    destination = static_cast<uint8>(_flags); \
    }

#define LOAD_FLAGS_FROM(source) { \
    uint _tmp = source; \
    m_zero_indicator = ~_tmp & gb_zero_flag; \
    m_carry_indicator = (_tmp & gb_carry_flag) << gb_hcs_shift; \
    m_hcs_flags = ((_tmp << gb_hcs_shift) & gb_hcs_flags) + 0x08; \
    m_hcs_operand = (_tmp & gb_half_carry_flag) > 0 ? 0x0F : 0; \
    }



// ----- memory access

#define READ_BYTE(destination, address) { \
    m_bus.handle_events(); \
    destination = m_bus.read_byte(address); \
    INC_CYCLES; \
    }

#define WRITE_BYTE(address, value) { \
    m_bus.handle_events(); \
    m_bus.write_byte(address, value); \
    INC_CYCLES; \
    }

#define WRITE_WORD(address, value) { \
    WRITE_BYTE(address, static_cast<uint8>(value)) \
    WRITE_BYTE(address + 1, static_cast<uint8>(value >> 8)) \
    }

#define POP_BYTE_AT_PC(destination) { \
    READ_BYTE(destination, m_pc) \
    ++m_pc; \
    }

#define POP_SIGNED_BYTE_AT_PC(destination) { \
    POP_BYTE_AT_PC(destination) \
    destination = (destination ^ 0x80) - 0x80; \
    }

#define POP_WORD_AT_PC(destination) { \
    uint _high, _low; \
    POP_BYTE_AT_PC(_low) \
    POP_BYTE_AT_PC(_high) \
    destination = static_cast<uint16>(_low + (_high << 8)); \
    }

#define PUSH_BYTE(byte) { \
    --m_sp; \
    WRITE_BYTE(m_sp, byte) \
    }

#define POP_BYTE(destination) { \
    READ_BYTE(destination, m_sp) \
    ++m_sp; \
    }

#define PUSH_PC { \
    PUSH_BYTE(m_pc >> 8) \
    PUSH_BYTE(m_pc) \
    }



// ----- jumps

// RST (16 cycles)
#define RST(opcode) { \
    INC_CYCLES; \
    PUSH_PC \
    m_pc = opcode & 0x38; \
    }

// JP (16 cycles)
#define JP { \
    uint _low, _high; \
    POP_BYTE_AT_PC(_low) \
    POP_BYTE_AT_PC(_high) \
    m_pc = static_cast<uint16>(_low + (_high << 8)); \
    INC_CYCLES; \
    }

// JP <cond> (16 cycles if jumped, 12 cycles if not)
#define JP_IF(condition) { \
    if (condition) { \
    JP \
    } \
    else { \
    m_pc += 2; \
    INC_CYCLES; \
    INC_CYCLES; \
    } \
    }

// CALL (24 cycles)
#define CALL { \
    uint16 _ret_pc = m_pc + 2; \
    JP \
    PUSH_BYTE(_ret_pc >> 8) \
    PUSH_BYTE(_ret_pc) \
    }

// CALL <cond> (24 cycles if jumped, 12 cycles if not)
#define CALL_IF(condition) { \
    if (condition) { \
    CALL \
    } \
    else { \
    m_pc += 2; \
    INC_CYCLES; \
    INC_CYCLES; \
    } \
    }

// RET (16 cycles)
#define RET { \
    uint _low, _high; \
    POP_BYTE(_low); \
    POP_BYTE(_high); \
    m_pc = static_cast<uint16>(_low + (_high << 8)); \
    INC_CYCLES; \
    }

// RET <cond> (20 cycles if jumped, 8 cycles if not)
#define RET_IF(condition) { \
    if (condition) { \
    INC_CYCLES; \
    RET \
    } \
    INC_CYCLES; \
    }

// JR (12 cycles)
#define JR { \
    uint _offset; \
    POP_SIGNED_BYTE_AT_PC(_offset) \
    m_pc = static_cast<uint16>(_offset + m_pc); \
    INC_CYCLES; \
    }

// JR <cond> (12 cycles if jumped, 8 cycles if not)
#define JR_IF(condition) { \
    if (condition) { \
    JR \
    } \
    else { \
    ++m_pc; \
    INC_CYCLES; \
    } \
    }



// ----- increment & decrement

// INC 8 bit value (4 cycles, 12 cycles if value incremented in memory)
// carry = unmodified, subtract = cleared, zero & half calculated
#define INC_8BIT(value) { \
    m_hcs_flags = m_zero_indicator = value; \
    m_hcs_operand = 1; \
    ++m_zero_indicator; \
    }

#define INC_REG(value) { \
    INC_8BIT(value) \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define INC_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    INC_8BIT(_value) \
    WRITE_BYTE(_hl, static_cast<uint8>(m_zero_indicator)) \
    }

// DEC 8 bit value (4 cycles, 12 cycles if value incremented in memory)
// carry = unmodified, subtract = set, zero & half calculated
#define DEC(value) { \
    m_zero_indicator = value; \
    m_hcs_flags = value + gb_hcs_subtract; \
    m_hcs_operand = 1; \
    --m_zero_indicator; \
    }

#define DEC_REG(value) { \
    DEC(value); \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define DEC_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    DEC(_value) \
    WRITE_BYTE(_hl, static_cast<uint8>(m_zero_indicator)) \
    }

// INC & DEC 16 bit value (8 cycles)
#define ADD_TO_BYTES(high, low, value) { \
    uint _tmp = low + value; \
    low = static_cast<uint8>(_tmp); \
    high = static_cast<uint8>(high + (_tmp >> 8)); \
    INC_CYCLES; \
    }

#define INC_BYTES(high, low) ADD_TO_BYTES(high, low, 1)
#define DEC_BYTES(high, low) ADD_TO_BYTES(high, low, -1)



// ----- arithmetic

// ADD & ADC 8 bit value (4 cycles, 8 cycles if value read from memory)
// subtract = cleared, carry & zero & half calculated
#define ADD(value) { \
    m_hcs_flags = m_a; \
    m_hcs_operand = value; \
    m_carry_indicator = m_zero_indicator = m_a + m_hcs_operand; \
    m_a = static_cast<uint8>(m_zero_indicator); \
    }

#define ADC(value) { \
    uint _carry = CARRY_INDICATOR_FLAG; \
    m_hcs_flags = m_a + _carry; \
    m_hcs_operand = value; \
    m_carry_indicator = m_zero_indicator = m_a + m_hcs_operand + (_carry >> 8); \
    m_a = static_cast<uint8>(m_zero_indicator); \
    }

#define ADD_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    ADD(_value) \
    }

#define ADC_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    ADC(_value) \
    }

// ADD to HL (8 cycles)
// subtract = cleared, zero = unaffected, carry & half calculated
#define ADD_TO_HL(high, low) { \
    uint _tmp = m_l; \
    _tmp += low; \
    m_hcs_flags = m_h + (_tmp & gb_hcs_old_carry); \
    m_hcs_operand = high; \
    m_carry_indicator = m_h + m_hcs_operand + (_tmp >> 8); \
    m_h = static_cast<uint8>(m_carry_indicator); \
    m_l = static_cast<uint8>(_tmp); \
    INC_CYCLES; \
    }

// ADD signed value to SP (16 cycles)
// zero = subtract = cleared, carry & half calculated
#define ADD_SP { \
    uint _value; \
    POP_SIGNED_BYTE_AT_PC(_value) \
    m_hcs_operand = _value & 0xFF; \
    m_hcs_flags = m_sp & 0xFF; \
    m_carry_indicator = m_hcs_operand + m_hcs_flags; \
    m_zero_indicator = 1; \
    m_sp = static_cast<uint16>(m_sp + _value); \
    }

#define ADD_TO_SP { \
    ADD_SP \
    INC_CYCLES; \
    INC_CYCLES; \
    }

// SUB & SBC 8 bit value (4 cycles, 8 cycles if value read from memory)
// subtract = set, carry & zero & half calculated
#define SUB(value) { \
    m_hcs_flags = m_a + gb_hcs_subtract; \
    m_hcs_operand = value; \
    m_carry_indicator = m_zero_indicator = m_a - m_hcs_operand; \
    m_a = static_cast<uint8>(m_zero_indicator); \
    }

#define SBC(value) { \
    uint _carry = CARRY_INDICATOR_FLAG; \
    m_hcs_flags = m_a + _carry + gb_hcs_subtract; \
    m_hcs_operand = value; \
    m_carry_indicator = m_zero_indicator = m_a - m_hcs_operand - (_carry >> 8); \
    m_a = static_cast<uint8>(m_zero_indicator); \
    }

#define SUB_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    SUB(_value) \
    }

#define SBC_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    SBC(_value) \
    }

// AND 8 bit value (4 cycles, 8 cycles if value read from memory)
// carry = subtract = cleared, half = set, zero calculated
#define AND(value) { \
    m_zero_indicator = m_a & value; \
    m_a = static_cast<uint8>(m_zero_indicator); \
    m_carry_indicator = 0; \
    m_hcs_flags = m_hcs_operand = 0x08; \
    }

#define AND_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    AND(_value) \
    }

// XOR 8 bit value (4 cycles, 8 cycles if value read from memory)
// carry = subtract = half = cleared, zero calculated
#define XOR(value) { \
    m_zero_indicator = m_a ^ value; \
    m_a = static_cast<uint8>(m_zero_indicator); \
    m_carry_indicator = m_hcs_flags = m_hcs_operand = 0; \
    }

#define XOR_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    XOR(_value) \
    }

// OR 8 bit value (4 cycles, 8 cycles if value read from memory)
// carry = subtract = half = cleared, zero calculated
#define OR(value) { \
    m_zero_indicator = m_a | value; \
    m_a = static_cast<uint8>(m_zero_indicator); \
    m_carry_indicator = m_hcs_flags = m_hcs_operand = 0; \
    }

#define OR_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    OR(_value) \
    }

// CP 8 bit value (4 cycles, 8 cycles if value read from memory)
// subtract = set, carry & half & zero calculated
#define CP(value) { \
    m_carry_indicator = m_a; \
    m_hcs_flags = m_carry_indicator + gb_hcs_subtract; \
    m_hcs_operand = value; \
    m_carry_indicator -= m_hcs_operand; \
    m_zero_indicator = m_carry_indicator; \
    }

#define CP_MEM(address) { \
    uint8 _value; \
    READ_BYTE(_value, address) \
    CP(_value) \
    }



// ----- loads

// LD 8 bit value from memory
#define LD_IMM8_MEM_HL { \
    uint _value; \
    POP_BYTE_AT_PC(_value) \
    WRITE_BYTE(LOAD_HL, _value) \
    }

// LD HL, SP + n (12 cycles)
// flags set according to ADD SP, n
#define LD_HL_SP_ADD { \
    uint16 _sp_bak = m_sp; \
    ADD_SP \
    m_h = static_cast<uint8>(m_sp >> 8); \
    m_l = static_cast<uint8>(m_sp); \
    m_sp = _sp_bak; \
    INC_CYCLES; \
    }


// ----- bit operations

// bit-test 8 bit value
// carry = unmodified, subtract = cleared, half = set, zero calculated
#define BIT(value, opcode) { \
    m_zero_indicator = value & CB_BIT(opcode); \
    m_hcs_flags = m_hcs_operand = 0x08; \
    }

#define BIT_MEM_HL(opcode) { \
    uint8 _value; \
    READ_BYTE(_value, LOAD_HL) \
    BIT(_value, opcode) \
    }

// reset bit in 8 bit value
// carry, zero, subtract, half = unmodified
#define RES_MEM_HL(opcode) { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    _value &= ~CB_BIT(opcode); \
    WRITE_BYTE(_hl, _value) \
    }

// set bit in 8 bit value
// carry, zero, subtract, half = unmodified
#define SET_MEM_HL(opcode) { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    _value |= CB_BIT(opcode); \
    WRITE_BYTE(_hl, _value) \
    }



// ----- shifts & rotations

// RLC 8 bit value
// half & subtract = cleared, carry = old bit 7, zero calculated (RLCA: zero = cleared)
#define RLC(value) { \
    m_hcs_flags = 0; \
    m_carry_indicator = value; \
    m_carry_indicator <<= 1; \
    m_zero_indicator = m_carry_indicator + (m_carry_indicator >> 8); \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define RLCA { \
    RLC(m_a) \
    m_zero_indicator = 1; \
    }

#define RLC_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    RLC(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// RL 8 bit value
// half & subtract = cleared, carry = old bit 7, zero calculated (RLA: zero = cleared)
#define RL(value) { \
    m_hcs_flags = 0; \
    uint _old_carry_bit = CARRY_INDICATOR_FLAG >> 8; \
    m_carry_indicator = value; \
    m_carry_indicator <<= 1; \
    m_zero_indicator = m_carry_indicator + _old_carry_bit; \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define RLA { \
    RL(m_a) \
    m_zero_indicator = 1; \
    }

#define RL_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    RL(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// RRC 8 bit value
// half & subtract = cleared, carry = old bit 0, zero calculated
#define RRC(value) { \
    m_hcs_flags = 0; \
    m_zero_indicator = value; \
    m_carry_indicator = m_zero_indicator << 8; \
    value = static_cast<uint8>((m_carry_indicator + m_zero_indicator) >> 1); \
    }

#define RRCA { \
    RRC(m_a) \
    m_zero_indicator = 1; \
    }

#define RRC_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    RRC(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// RR 8 bit value
// half & subtract = cleared, carry = old bit 0, zero calculated
#define RR(value) { \
    m_hcs_flags = 0; \
    m_zero_indicator = value + CARRY_INDICATOR_FLAG; \
    m_carry_indicator = m_zero_indicator << 8; \
    m_zero_indicator >>= 1; \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define RRA { \
    RR(m_a) \
    m_zero_indicator = 1; \
    }

#define RR_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    RR(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// SLA 8 bit value
// half & subtract = cleared, carry & zero calculated
#define SLA(value) { \
    m_hcs_flags = 0; \
    m_zero_indicator = value; \
    m_zero_indicator <<= 1; \
    m_carry_indicator = m_zero_indicator; \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define SLA_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    SLA(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// SRA 8 bit value
// half & subtract = cleared, carry & zero calculated
#define SRA(value) { \
    m_hcs_flags = 0; \
    m_zero_indicator = value; \
    m_carry_indicator = m_zero_indicator << 8; \
    m_zero_indicator >>= 1; \
    m_zero_indicator += value & 0x80; \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define SRA_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    SRA(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// SRL 8 bit value
// half & subtract = cleared, carry & zero calculated
#define SRL(value) { \
    m_hcs_flags = 0; \
    m_zero_indicator = value; \
    m_carry_indicator = m_zero_indicator << 8; \
    m_zero_indicator >>= 1; \
    value = static_cast<uint8>(m_zero_indicator); \
    }

#define SRL_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    SRL(_value); \
    WRITE_BYTE(_hl, _value) \
    }

// SWAP 8 bit value
// half & subtract & carry = cleared, zero calculated
#define SWAP(value) { \
    m_hcs_flags = m_carry_indicator = 0; \
    m_zero_indicator = value = (value << 4) + (value >> 4); \
    }

#define SWAP_MEM_HL { \
    uint8 _value; \
    uint16 _hl = LOAD_HL; \
    READ_BYTE(_value, _hl) \
    SWAP(_value); \
    WRITE_BYTE(_hl, _value) \
    }





//---------------------------------------------------------
//
//   gb_cpu constructor & register access
//
//---------------------------------------------------------

age::gb_cpu::gb_cpu(gb_core &core, gb_bus &bus)
    : m_core(core),
      m_bus(bus)
{
    // reset registers (writing m_regs)
    m_pc = 0x0100;
    m_sp = 0xFFFE;

    if (m_core.get_mode() == gb_mode::dmg)
    {
        m_a = 0x01;
        LOAD_FLAGS_FROM(0xB0)
        m_b = 0x00;
        m_c = 0x13;
        m_d = 0x00;
        m_e = 0xD8;
        m_h = 0x01;
        m_l = 0x4D;
    }
    else
    {
        // gb_mode::cgb || gb_mode::dmg_on_cgb
        m_a = 0x11;
        LOAD_FLAGS_FROM(0x80)
        m_b = 0x00;
        m_c = 0x00;
        m_d = 0x00;
        m_e = 0x08;
        m_h = 0x00;
        m_l = 0x7C;
    }
}



age::gb_test_info age::gb_cpu::get_test_info() const
{
    gb_test_info result;

    result.m_is_cgb = m_core.is_cgb();
    result.m_mooneye_debug_op = m_mooneye_debug_op;

    result.m_a = m_a;
    result.m_b = m_b;
    result.m_c = m_c;
    result.m_d = m_d;
    result.m_e = m_e;
    result.m_h = m_h;
    result.m_l = m_l;

    return result;
}





//---------------------------------------------------------
//
//   cpu instruction emulation
//
//---------------------------------------------------------

void age::gb_cpu::emulate_instruction()
{
    uint16 interrupt_pc = m_core.get_interrupt_to_service();
    if (interrupt_pc != 0)
    {
        // http://gbdev.gg8.se/wiki/articles/Interrupts
        INC_CYCLES; // execute two wait states
        INC_CYCLES;
        PUSH_PC;    // 8 cycles
        INC_CYCLES; // load new PC
        m_pc = interrupt_pc;
        return;
    }

    // no event handling necessary here, done in emulator's main loop
    uint8 opcode = m_bus.read_byte(m_pc);
    INC_CYCLES;

    if (!m_next_byte_twice)
    {
        ++m_pc;
    }
    m_next_byte_twice = false;

    switch (opcode)
    {
        default:
            --m_pc; // stay here
            break;

        // increment & decrement

        case 0x04: INC_REG(m_b) break;
        case 0x0C: INC_REG(m_c) break;
        case 0x14: INC_REG(m_d) break;
        case 0x1C: INC_REG(m_e) break;
        case 0x24: INC_REG(m_h) break;
        case 0x2C: INC_REG(m_l) break;
        case 0x34: INC_MEM_HL break;
        case 0x3C: INC_REG(m_a) break;

        case 0x05: DEC_REG(m_b) break;
        case 0x0D: DEC_REG(m_c) break;
        case 0x15: DEC_REG(m_d) break;
        case 0x1D: DEC_REG(m_e) break;
        case 0x25: DEC_REG(m_h) break;
        case 0x2D: DEC_REG(m_l) break;
        case 0x35: DEC_MEM_HL break;
        case 0x3D: DEC_REG(m_a) break;

        case 0x03: INC_BYTES(m_b, m_c) break; // INC BC (8 cycles)
        case 0x13: INC_BYTES(m_d, m_e) break; // INC DE (8 cycles)
        case 0x23: INC_BYTES(m_h, m_l) break; // INC HL (8 cycles)
        case 0x33: ++m_sp; INC_CYCLES; break; // INC SP (8 cycles)

        case 0x0B: DEC_BYTES(m_b, m_c) break; // DEC BC (8 cycles)
        case 0x1B: DEC_BYTES(m_d, m_e) break; // DEC DE (8 cycles)
        case 0x2B: DEC_BYTES(m_h, m_l) break; // DEC HL (8 cycles)
        case 0x3B: --m_sp; INC_CYCLES; break; // DEC SP (8 cycles)

            // loads

        case 0x06: POP_BYTE_AT_PC(m_b) break; // LD B, x (8 cycles)
        case 0x0E: POP_BYTE_AT_PC(m_c) break; // LD C, x (8 cycles)
        case 0x16: POP_BYTE_AT_PC(m_d) break; // LD D, x (8 cycles)
        case 0x1E: POP_BYTE_AT_PC(m_e) break; // LD E, x (8 cycles)
        case 0x26: POP_BYTE_AT_PC(m_h) break; // LD H, x (8 cycles)
        case 0x2E: POP_BYTE_AT_PC(m_l) break; // LD L, x (8 cycles)
        case 0x36: LD_IMM8_MEM_HL break;      // LD [HL], x (12 cycles)
        case 0x3E: POP_BYTE_AT_PC(m_a) break; // LD A, x (8 cycles)

        case 0x40: m_mooneye_debug_op = true; break; // LD B, B (4 cycles)
        case 0x41: m_b = m_c; break; // LD B, C (4 cycles)
        case 0x42: m_b = m_d; break; // LD B, D (4 cycles)
        case 0x43: m_b = m_e; break; // LD B, E (4 cycles)
        case 0x44: m_b = m_h; break; // LD B, H (4 cycles)
        case 0x45: m_b = m_l; break; // LD B, L (4 cycles)
        case 0x46: READ_BYTE(m_b, LOAD_HL) break; // LD B, [HL] (8 cycles)
        case 0x47: m_b = m_a; break; // LD B, A (4 cycles)

        case 0x48: m_c = m_b; break;
        case 0x49: break;
        case 0x4A: m_c = m_d; break;
        case 0x4B: m_c = m_e; break;
        case 0x4C: m_c = m_h; break;
        case 0x4D: m_c = m_l; break;
        case 0x4E: READ_BYTE(m_c, LOAD_HL) break;
        case 0x4F: m_c = m_a; break;

        case 0x50: m_d = m_b; break;
        case 0x51: m_d = m_c; break;
        case 0x52: break;
        case 0x53: m_d = m_e; break;
        case 0x54: m_d = m_h; break;
        case 0x55: m_d = m_l; break;
        case 0x56: READ_BYTE(m_d, LOAD_HL) break;
        case 0x57: m_d = m_a; break;

        case 0x58: m_e = m_b; break;
        case 0x59: m_e = m_c; break;
        case 0x5A: m_e = m_d; break;
        case 0x5B: break;
        case 0x5C: m_e = m_h; break;
        case 0x5D: m_e = m_l; break;
        case 0x5E: READ_BYTE(m_e, LOAD_HL) break;
        case 0x5F: m_e = m_a; break;

        case 0x60: m_h = m_b; break;
        case 0x61: m_h = m_c; break;
        case 0x62: m_h = m_d; break;
        case 0x63: m_h = m_e; break;
        case 0x64: break;
        case 0x65: m_h = m_l; break;
        case 0x66: READ_BYTE(m_h, LOAD_HL) break;
        case 0x67: m_h = m_a; break;

        case 0x68: m_l = m_b; break;
        case 0x69: m_l = m_c; break;
        case 0x6A: m_l = m_d; break;
        case 0x6B: m_l = m_e; break;
        case 0x6C: m_l = m_h; break;
        case 0x6D: break;
        case 0x6E: READ_BYTE(m_l, LOAD_HL) break;
        case 0x6F: m_l = m_a; break;

        case 0x70: WRITE_BYTE(LOAD_HL, m_b) break; // LD [HL], B (8 cycles)
        case 0x71: WRITE_BYTE(LOAD_HL, m_c) break; // LD [HL], C (8 cycles)
        case 0x72: WRITE_BYTE(LOAD_HL, m_d) break; // LD [HL], D (8 cycles)
        case 0x73: WRITE_BYTE(LOAD_HL, m_e) break; // LD [HL], E (8 cycles)
        case 0x74: WRITE_BYTE(LOAD_HL, m_h) break; // LD [HL], H (8 cycles)
        case 0x75: WRITE_BYTE(LOAD_HL, m_l) break; // LD [HL], L (8 cycles)
        case 0x77: WRITE_BYTE(LOAD_HL, m_a) break; // LD [HL], A (8 cycles)

        case 0x78: m_a = m_b; break;
        case 0x79: m_a = m_c; break;
        case 0x7A: m_a = m_d; break;
        case 0x7B: m_a = m_e; break;
        case 0x7C: m_a = m_h; break;
        case 0x7D: m_a = m_l; break;
        case 0x7E: READ_BYTE(m_a, LOAD_HL) break;
        case 0x7F: break;

        case 0x02: WRITE_BYTE(LOAD_BC, m_a) break; // LD [BC], A
        case 0x0A: READ_BYTE(m_a, LOAD_BC) break;  // LD A, [BC]
        case 0x12: WRITE_BYTE(LOAD_DE, m_a) break; // LD [DE], A
        case 0x1A: READ_BYTE(m_a, LOAD_DE) break;  // LD A, [DE]

        case 0xF8: LD_HL_SP_ADD break; // LD HL, SP + x
        case 0xF9: m_sp = LOAD_HL; INC_CYCLES; break; // LD SP, HL (8 cycles)
        case 0x08: { uint16 address; POP_WORD_AT_PC(address) WRITE_WORD(address, m_sp) } break; // LD [xx], SP

        case 0xE0: { uint8 offset; POP_BYTE_AT_PC(offset) WRITE_BYTE(0xFF00 + offset, m_a) } break; // LDH [x], A
        case 0xF0: { uint8 offset; POP_BYTE_AT_PC(offset) READ_BYTE(m_a, 0xFF00 + offset) } break;  // LDH A, [x]
        case 0xE2: WRITE_BYTE(0xFF00 + m_c, m_a) break; // LDH [C], A
        case 0xF2: READ_BYTE(m_a, 0xFF00 + m_c) break;  // LDH A, [C]
        case 0xEA: { uint16 address; POP_WORD_AT_PC(address) WRITE_BYTE(address, m_a) } break; // LD [xx], A
        case 0xFA: { uint16 address; POP_WORD_AT_PC(address) READ_BYTE(m_a, address) } break;  // LD A, [xx]
        case 0x22: { uint16 hl = LOAD_HL; WRITE_BYTE(hl, m_a) ++hl; STORE_HL(hl) } break; // LDI [HL], A
        case 0x32: { uint16 hl = LOAD_HL; WRITE_BYTE(hl, m_a) --hl; STORE_HL(hl) } break; // LDD [HL], A
        case 0x2A: { uint16 hl = LOAD_HL; READ_BYTE(m_a, hl) ++hl; STORE_HL(hl) } break;  // LDI A, [HL]
        case 0x3A: { uint16 hl = LOAD_HL; READ_BYTE(m_a, hl) --hl; STORE_HL(hl) } break;  // LDD A, [HL]

        case 0x01: POP_BYTE_AT_PC(m_c) POP_BYTE_AT_PC(m_b) break; // LD BC, xx (12 cycles)
        case 0x11: POP_BYTE_AT_PC(m_e) POP_BYTE_AT_PC(m_d) break; // LD DE, xx (12 cycles)
        case 0x21: POP_BYTE_AT_PC(m_l) POP_BYTE_AT_PC(m_h) break; // LD HL, xx (12 cycles)
        case 0x31: { uint h, l; POP_BYTE_AT_PC(l) POP_BYTE_AT_PC(h) m_sp = static_cast<uint16>(l + (h << 8)); } break; // LD AF, xx (12 cycles)

            // arithmetic

        case 0xC6: ADD_MEM(m_pc) ++m_pc; break;
        case 0x80: ADD(m_b) break;
        case 0x81: ADD(m_c) break;
        case 0x82: ADD(m_d) break;
        case 0x83: ADD(m_e) break;
        case 0x84: ADD(m_h) break;
        case 0x85: ADD(m_l) break;
        case 0x86: ADD_MEM(LOAD_HL) break;
        case 0x87: ADD(m_a) break;

        case 0xE8: ADD_TO_SP break;
        case 0x09: ADD_TO_HL(m_b, m_c) break;
        case 0x19: ADD_TO_HL(m_d, m_e) break;
        case 0x29: ADD_TO_HL(m_h, m_l) break;
        case 0x39: ADD_TO_HL(m_sp >> 8, m_sp & 0xFF) break;

        case 0xCE: ADC_MEM(m_pc) ++m_pc; break;
        case 0x88: ADC(m_b) break;
        case 0x89: ADC(m_c) break;
        case 0x8A: ADC(m_d) break;
        case 0x8B: ADC(m_e) break;
        case 0x8C: ADC(m_h) break;
        case 0x8D: ADC(m_l) break;
        case 0x8E: ADC_MEM(LOAD_HL) break;
        case 0x8F: ADC(m_a) break;

        case 0xD6: SUB_MEM(m_pc) ++m_pc; break;
        case 0x90: SUB(m_b) break;
        case 0x91: SUB(m_c) break;
        case 0x92: SUB(m_d) break;
        case 0x93: SUB(m_e) break;
        case 0x94: SUB(m_h) break;
        case 0x95: SUB(m_l) break;
        case 0x96: SUB_MEM(LOAD_HL) break;
        case 0x97: SUB(m_a) break;

        case 0xDE: SBC_MEM(m_pc) ++m_pc; break;
        case 0x98: SBC(m_b) break;
        case 0x99: SBC(m_c) break;
        case 0x9A: SBC(m_d) break;
        case 0x9B: SBC(m_e) break;
        case 0x9C: SBC(m_h) break;
        case 0x9D: SBC(m_l) break;
        case 0x9E: SBC_MEM(LOAD_HL) break;
        case 0x9F: SBC(m_a) break;

        case 0xE6: AND_MEM(m_pc) ++m_pc; break;
        case 0xA0: AND(m_b) break;
        case 0xA1: AND(m_c) break;
        case 0xA2: AND(m_d) break;
        case 0xA3: AND(m_e) break;
        case 0xA4: AND(m_h) break;
        case 0xA5: AND(m_l) break;
        case 0xA6: AND_MEM(LOAD_HL) break;
        case 0xA7: AND(m_a) break;

        case 0xEE: XOR_MEM(m_pc) ++m_pc; break;
        case 0xA8: XOR(m_b) break;
        case 0xA9: XOR(m_c) break;
        case 0xAA: XOR(m_d) break;
        case 0xAB: XOR(m_e) break;
        case 0xAC: XOR(m_h) break;
        case 0xAD: XOR(m_l) break;
        case 0xAE: XOR_MEM(LOAD_HL) break;
        case 0xAF: XOR(m_a) break;

        case 0xF6: OR_MEM(m_pc) ++m_pc; break;
        case 0xB0: OR(m_b) break;
        case 0xB1: OR(m_c) break;
        case 0xB2: OR(m_d) break;
        case 0xB3: OR(m_e) break;
        case 0xB4: OR(m_h) break;
        case 0xB5: OR(m_l) break;
        case 0xB6: OR_MEM(LOAD_HL) break;
        case 0xB7: OR(m_a) break;

        case 0xFE: CP_MEM(m_pc) ++m_pc; break;
        case 0xB8: CP(m_b) break;
        case 0xB9: CP(m_c) break;
        case 0xBA: CP(m_d) break;
        case 0xBB: CP(m_e) break;
        case 0xBC: CP(m_h) break;
        case 0xBD: CP(m_l) break;
        case 0xBE: CP_MEM(LOAD_HL) break;
        case 0xBF: CP(m_a) break;

            // jumps

        case 0xC7: RST(0xC7) break;
        case 0xCF: RST(0xCF) break;
        case 0xD7: RST(0xD7) break;
        case 0xDF: RST(0xDF) break;
        case 0xE7: RST(0xE7) break;
        case 0xEF: RST(0xEF) break;
        case 0xF7: RST(0xF7) break;
        case 0xFF: RST(0xFF) break;

        case 0xCD: CALL break;
        case 0xC4: CALL_IF(!ZERO_FLAGGED) break;
        case 0xCC: CALL_IF(ZERO_FLAGGED) break;
        case 0xD4: CALL_IF(!CARRY_FLAGGED) break;
        case 0xDC: CALL_IF(CARRY_FLAGGED) break;

        case 0xC3: JP break;
        case 0xE9: m_pc = LOAD_HL; break;
        case 0xC2: JP_IF(!ZERO_FLAGGED) break;
        case 0xCA: JP_IF(ZERO_FLAGGED) break;
        case 0xD2: JP_IF(!CARRY_FLAGGED) break;
        case 0xDA: JP_IF(CARRY_FLAGGED) break;

        case 0xC9: RET break;
        case 0xD9: RET m_core.ei_now(); break; // RETI
        case 0xC0: RET_IF(!ZERO_FLAGGED) break;
        case 0xC8: RET_IF(ZERO_FLAGGED) break;
        case 0xD0: RET_IF(!CARRY_FLAGGED) break;
        case 0xD8: RET_IF(CARRY_FLAGGED) break;

        case 0x18: JR break;
        case 0x20: JR_IF(!ZERO_FLAGGED) break;
        case 0x28: JR_IF(ZERO_FLAGGED) break;
        case 0x30: JR_IF(!CARRY_FLAGGED) break;
        case 0x38: JR_IF(CARRY_FLAGGED) break;

            // stack (push & pop)

        case 0xC5: INC_CYCLES; PUSH_BYTE(m_b) PUSH_BYTE(m_c) break; // PUSH BC (16 cycles)
        case 0xD5: INC_CYCLES; PUSH_BYTE(m_d) PUSH_BYTE(m_e) break; // PUSH DE (16 cycles)
        case 0xE5: INC_CYCLES; PUSH_BYTE(m_h) PUSH_BYTE(m_l) break; // PUSH HL (16 cycles)
        case 0xF5: INC_CYCLES; PUSH_BYTE(m_a) { uint8 f; STORE_FLAGS_TO(f) PUSH_BYTE(f) } break; // PUSH AF (16 cycles)

        case 0xC1: POP_BYTE(m_c) POP_BYTE(m_b) break; // POP BC (12 cycles)
        case 0xD1: POP_BYTE(m_e) POP_BYTE(m_d) break; // POP DE (12 cycles)
        case 0xE1: POP_BYTE(m_l) POP_BYTE(m_h) break; // POP HL (12 cycles)
        case 0xF1: { uint8 f; POP_BYTE(f) LOAD_FLAGS_FROM(f) } POP_BYTE(m_a) break; // POP AF (12 cycles)

            // misc

        case 0x07: RLCA break;
        case 0x0F: RRCA break;
        case 0x17: RLA break;
        case 0x1F: RRA break;

        case 0x00: break; // NOP
        case 0x10: m_core.stop(); break; // STOP
        case 0x2F: m_a = ~m_a; m_hcs_flags = gb_hcs_subtract; m_hcs_operand = 1; break; // CPL
        case 0x37: m_carry_indicator = 0x100; m_hcs_flags = m_hcs_operand = 0; break;   // SCF
        case 0x3F: m_carry_indicator ^= 0x100; m_hcs_flags = m_hcs_operand = 0; break;  // CCF
        case 0x76: m_next_byte_twice = m_core.halt() && !m_core.is_cgb(); break;        // HALT
        case 0xF3: m_core.di(); break; // DI
        case 0xFB: m_core.ei_delayed(); break; // EI

        case 0x27: // DAA
        {
            uint8 f;
            STORE_FLAGS_TO(f);
            // calculate correction based on carry flags
            uint8 correction = ((f & gb_carry_flag) > 0)? 0x60 : 0;
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

        case 0xCB:
        {
            POP_BYTE_AT_PC(opcode);

            switch (opcode)
            {
                // rotates & shifts

                case 0x00: RLC(m_b) break;
                case 0x01: RLC(m_c) break;
                case 0x02: RLC(m_d) break;
                case 0x03: RLC(m_e) break;
                case 0x04: RLC(m_h) break;
                case 0x05: RLC(m_l) break;
                case 0x06: RLC_MEM_HL break;
                case 0x07: RLC(m_a) break;

                case 0x08: RRC(m_b) break;
                case 0x09: RRC(m_c) break;
                case 0x0A: RRC(m_d) break;
                case 0x0B: RRC(m_e) break;
                case 0x0C: RRC(m_h) break;
                case 0x0D: RRC(m_l) break;
                case 0x0E: RRC_MEM_HL break;
                case 0x0F: RRC(m_a) break;

                case 0x10: RL(m_b) break;
                case 0x11: RL(m_c) break;
                case 0x12: RL(m_d) break;
                case 0x13: RL(m_e) break;
                case 0x14: RL(m_h) break;
                case 0x15: RL(m_l) break;
                case 0x16: RL_MEM_HL break;
                case 0x17: RL(m_a) break;

                case 0x18: RR(m_b) break;
                case 0x19: RR(m_c) break;
                case 0x1A: RR(m_d) break;
                case 0x1B: RR(m_e) break;
                case 0x1C: RR(m_h) break;
                case 0x1D: RR(m_l) break;
                case 0x1E: RR_MEM_HL break;
                case 0x1F: RR(m_a) break;

                case 0x20: SLA(m_b) break;
                case 0x21: SLA(m_c) break;
                case 0x22: SLA(m_d) break;
                case 0x23: SLA(m_e) break;
                case 0x24: SLA(m_h) break;
                case 0x25: SLA(m_l) break;
                case 0x26: SLA_MEM_HL break;
                case 0x27: SLA(m_a) break;

                case 0x28: SRA(m_b) break;
                case 0x29: SRA(m_c) break;
                case 0x2A: SRA(m_d) break;
                case 0x2B: SRA(m_e) break;
                case 0x2C: SRA(m_h) break;
                case 0x2D: SRA(m_l) break;
                case 0x2E: SRA_MEM_HL break;
                case 0x2F: SRA(m_a) break;

                case 0x30: SWAP(m_b) break;
                case 0x31: SWAP(m_c) break;
                case 0x32: SWAP(m_d) break;
                case 0x33: SWAP(m_e) break;
                case 0x34: SWAP(m_h) break;
                case 0x35: SWAP(m_l) break;
                case 0x36: SWAP_MEM_HL break;
                case 0x37: SWAP(m_a) break;

                case 0x38: SRL(m_b) break;
                case 0x39: SRL(m_c) break;
                case 0x3A: SRL(m_d) break;
                case 0x3B: SRL(m_e) break;
                case 0x3C: SRL(m_h) break;
                case 0x3D: SRL(m_l) break;
                case 0x3E: SRL_MEM_HL break;
                case 0x3F: SRL(m_a) break;

                    // bit test

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
                case 0x7F: BIT(m_a, 0x7F); break;

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
                case 0xBF: m_a &= ~CB_BIT(0xBF); break;

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
}
