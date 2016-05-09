#include "Emulate.h"

#include <functional>
#include <stdint.h>
#include "Fifo.h"
#include "Memory.h"

Emulator::Emulator(RegisterFile *registers)
    : registers(registers)
{
    modrm_decoder = std::make_unique<ModRMDecoder>(
        [&]{ return this->fetch_byte(); },
        this->registers
    );
}

size_t Emulator::emulate()
{
    instr_length = 0;

    opcode = fetch_byte();
    switch (opcode) {
    // mov
    case 0x88: mov88(); break;
    case 0x89: mov89(); break;
    case 0x8a: mov8a(); break;
    case 0x8b: mov8b(); break;
    case 0xc6: movc6(); break;
    case 0xc7: movc7(); break;
    case 0xb0 ... 0xb7: movb0_b7(); break;
    case 0xb8 ... 0xbf: movb8_bf(); break;
    case 0xa0: mova0(); break;
    case 0xa1: mova1(); break;
    case 0xa2: mova2(); break;
    case 0xa3: mova3(); break;
    case 0x8e: mov8e(); break;
    case 0x8c: mov8c(); break;
    // push
    case 0xff: pushff(); break;
    case 0x50 ... 0x57: push50_57(); break;
    case 0x06: // fallthrough
    case 0x0e: // fallthrough
    case 0x16: // fallthrough
    case 0x1e: pushsr(); break;
    // pop
    case 0xf8: popf8(); break;
    case 0x58 ... 0x5f: pop58_5f(); break;
    case 0x07: // fallthrough
    case 0x0f: // fallthrough
    case 0x17: // fallthrough
    case 0x1f: popsr(); break;
    // xchg
    case 0x86: xchg86(); break;
    case 0x87: xchg87(); break;
    case 0x90 ... 0x97: xchg90_97(); break;
    // in
    case 0xe4: ine4(); break;
    case 0xe5: ine5(); break;
    case 0xec: inec(); break;
    case 0xed: ined(); break;
    // out
    case 0xe6: oute6(); break;
    case 0xe7: oute7(); break;
    case 0xee: outee(); break;
    case 0xef: outef(); break;
    // xlat
    case 0xd7: xlatd7(); break;
    // lea
    case 0x8d: lea8d(); break;
    // lds
    case 0xc5: ldsc5(); break;
    // les
    case 0xc4: lesc4(); break;
    // lahf
    case 0x9f: lahf9f(); break;
    // sahf
    case 0x9e: sahf9e(); break;
    // pushf
    case 0x9c: pushf9c(); break;
    // popf
    case 0x9d: popf9d(); break;
    // add / adc
    case 0x80: add_adc_80(); break;
    case 0x81: add_adc_81(); break;
    case 0x82: add_adc_82(); break;
    case 0x83: add_adc_83(); break;
    // add
    case 0x00: add00(); break;
    case 0x01: add01(); break;
    case 0x02: add02(); break;
    case 0x03: add03(); break;
    case 0x04: add04(); break;
    case 0x05: add05(); break;
    // adc
    case 0x10: adc10(); break;
    case 0x11: adc11(); break;
    case 0x12: adc12(); break;
    case 0x13: adc13(); break;
    case 0x14: adc14(); break;
    case 0x15: adc15(); break;
    }

    return instr_length;
}

// mov m/r, r (8-bit)
void Emulator::mov88()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    auto source = modrm_decoder->reg();
    auto val = registers->get(source);

    write_data<uint8_t>(val);
}

// mov m/r, r (16-bit)
void Emulator::mov89()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    auto source = modrm_decoder->reg();
    auto val = registers->get(source);

    write_data<uint16_t>(val);
}

// mov r, m/r (8-bit)
void Emulator::mov8a()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t val = read_data<uint8_t>();

    auto dest = modrm_decoder->reg();
    registers->set(dest, val);
}

// mov r, m/r (16-bit)
void Emulator::mov8b()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t val = read_data<uint16_t>();

    auto dest = modrm_decoder->reg();
    registers->set(dest, val);
}

// mov r/m, immediate (reg == 0), 8-bit
void Emulator::movc6()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 0) {
        uint8_t immed = fetch_byte();
        write_data<uint8_t>(immed);
    }
}

// mov r/m, immediate (reg == 0), 16-bit
void Emulator::movc7()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() == 0)
        write_data<uint16_t>(fetch_16bit());
}

// mov r, immediate, 8-bit
void Emulator::movb0_b7()
{
    uint8_t immed = fetch_byte();
    auto reg = static_cast<GPR>(static_cast<int>(AL) + (opcode & 0x7));
    registers->set(reg, immed);
}

// mov r, immediate, 16-bit
void Emulator::movb8_bf()
{
    uint16_t immed = fetch_16bit();
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    registers->set(reg, immed);
}

// mov al, m, 8-bit
void Emulator::mova0()
{
    auto displacement = fetch_16bit();
    auto addr = get_phys_addr(registers->get(DS), displacement);
    auto val = mem->read<uint8_t>(addr);
    registers->set(AL, val);
}

// mov ax, m, 16-bit
void Emulator::mova1()
{
    auto displacement = fetch_16bit();
    auto addr = get_phys_addr(registers->get(DS), displacement);
    auto val = mem->read<uint16_t>(addr);
    registers->set(AX, val);
}

// mov m, al 8-bit
void Emulator::mova2()
{
    auto displacement = fetch_16bit();
    auto val = registers->get(AL);
    auto addr = get_phys_addr(registers->get(DS), displacement);
    mem->write<uint8_t>(addr, val);
}

// mov m, al 16-bit
void Emulator::mova3()
{
    auto displacement = fetch_16bit();
    auto val = registers->get(AX);
    auto addr = get_phys_addr(registers->get(DS), displacement);
    mem->write<uint16_t>(addr, val);
}

// mov sr, r/m
void Emulator::mov8e()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() & (1 << 4))
        return;

    uint16_t val = read_data<uint16_t>();
    auto segnum = modrm_decoder->raw_reg();
    auto reg = static_cast<GPR>(static_cast<int>(ES) + segnum);

    registers->set(reg, val);
}

// mov r/m, sr
void Emulator::mov8c()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() & (1 << 4))
        return;

    auto segnum = modrm_decoder->raw_reg();
    auto reg = static_cast<GPR>(static_cast<int>(ES) + segnum);
    uint16_t val = registers->get(reg);

    write_data<uint16_t>(val);
}

// push r/m
void Emulator::pushff()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 6)
        return;

    auto val = read_data<uint16_t>();
    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// push r
void Emulator::push50_57()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    auto val = registers->get(reg);

    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// push sr
void Emulator::pushsr()
{
    auto reg = static_cast<GPR>(static_cast<int>(ES) + ((opcode >> 3) & 0x3));
    auto val = registers->get(reg);

    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// pop r/m
void Emulator::popf8()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0)
        return;

    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);
    registers->set(SP, registers->get(SP) + 2);

    write_data<uint16_t>(val);
}

// pop r
void Emulator::pop58_5f()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);

    registers->set(SP, registers->get(SP) + 2);
    registers->set(reg, val);
}

// pop sr
void Emulator::popsr()
{
    auto reg = static_cast<GPR>(static_cast<int>(ES) + ((opcode >> 3) & 0x3));
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);

    registers->set(reg, val);
    registers->set(SP, registers->get(SP) + 2);
}

// xchg r, r/m, 8-bit
void Emulator::xchg86()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    auto v1 = read_data<uint8_t>();
    auto v2 = registers->get(modrm_decoder->reg());

    write_data<uint8_t>(v2);
    registers->set(modrm_decoder->reg(), v1);
}

// xchg r, r/m, 16-bit
void Emulator::xchg87()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    auto v1 = read_data<uint16_t>();
    auto v2 = registers->get(modrm_decoder->reg());

    write_data<uint16_t>(v2);
    registers->set(modrm_decoder->reg(), v1);
}

// xchg accumulator, r
void Emulator::xchg90_97()
{
    auto reg = static_cast<GPR>(static_cast<int>(AX) + (opcode & 0x7));

    auto v1 = registers->get(AX);
    auto v2 = registers->get(reg);

    registers->set(AX, v2);
    registers->set(reg, v1);
}

// in al, data8
void Emulator::ine4()
{
    auto port_num = fetch_byte();

    registers->set(AL, io->read<uint8_t>(port_num));
}

// in ax, data8
void Emulator::ine5()
{
    auto port_num = fetch_byte();

    registers->set(AX, io->read<uint16_t>(port_num));
}

// in al, dx
void Emulator::inec()
{
    registers->set(AL, io->read<uint8_t>(registers->get(DX)));
}

// in ax, dx
void Emulator::ined()
{
    registers->set(AX, io->read<uint16_t>(registers->get(DX)));
}

// out data8, al
void Emulator::oute6()
{
    auto port_num = fetch_byte();

    io->write<uint8_t>(port_num, registers->get(AL));
}

// out data8, ax
void Emulator::oute7()
{
    auto port_num = fetch_byte();

    io->write<uint16_t>(port_num, registers->get(AX));
}

// out dx, al
void Emulator::outee()
{
    io->write<uint8_t>(registers->get(DX), registers->get(AL));
}

// out dx, ax
void Emulator::outef()
{
    io->write<uint16_t>(registers->get(DX), registers->get(AX));
}

// xlat
void Emulator::xlatd7()
{
    auto v = registers->get(AL);
    auto table_addr = registers->get(BX);
    auto xlated_val = mem->read<uint8_t>(get_phys_addr(registers->get(DS),
                                                       table_addr + v));
    registers->set(AL, xlated_val);
}

// lea r, r/m
void Emulator::lea8d()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    registers->set(modrm_decoder->reg(), modrm_decoder->effective_address());
}

// lds r, m
void Emulator::ldsc5()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->rm_type() == OP_REG)
        return;

    auto p32 = get_phys_addr(registers->get(DS), modrm_decoder->effective_address());
    auto displacement = mem->read<uint16_t>(p32);
    auto seg = mem->read<uint16_t>(p32 + 2);

    registers->set(modrm_decoder->reg(), displacement);
    registers->set(DS, seg);
}

// les r, m
void Emulator::lesc4()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->rm_type() == OP_REG)
        return;

    auto p32 = get_phys_addr(registers->get(DS), modrm_decoder->effective_address());
    auto displacement = mem->read<uint16_t>(p32);
    auto seg = mem->read<uint16_t>(p32 + 2);

    registers->set(modrm_decoder->reg(), displacement);
    registers->set(ES, seg);
}

// lahf
void Emulator::lahf9f()
{
    auto flags = registers->get_flags();
    registers->set(AH, flags & 0xff);
}

// sahf
void Emulator::sahf9e()
{
    auto new_flags = registers->get(AH);
    auto old_flags = registers->get_flags();
    registers->set_flags((old_flags & 0xff00) | new_flags);
}

// pushf
void Emulator::pushf9c()
{
    auto val = registers->get_flags();
    registers->set(SP, registers->get(SP) - 2);
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    mem->write<uint16_t>(addr, val);
}

// popf
void Emulator::popf9d()
{
    auto addr = get_phys_addr(registers->get(SS), registers->get(SP));
    auto val = mem->read<uint16_t>(addr);
    registers->set_flags(val);

    registers->set(SP, registers->get(SP) + 2);
}

template <typename T>
T Emulator::do_alu(uint16_t v1, uint16_t v2, uint16_t carry_in,
                   std::function<uint32_t(uint32_t, uint32_t, uint32_t)> alu_op)
{
    uint16_t flags = registers->get_flags();

    flags &= ~(AF | CF | OF | PF | SF | ZF);

    uint32_t result32 = alu_op(static_cast<uint32_t>(v1),
                               static_cast<uint32_t>(v2),
                               static_cast<uint32_t>(carry_in));
    bool af = !!(((v1 & 0xf) + (v2 & 0xf)) & (1 << 4));

    auto sign_bit = (8 * sizeof(T)) - 1;
    auto carry_bit = (8 * sizeof(T));

    if (af)
        flags |= AF;
    if (result32 & (1 << carry_bit))
        flags |= CF;
    if (result32 & (1 << sign_bit))
        flags |= SF;
    if ((result32 & static_cast<T>(-1)) == 0)
        flags |= ZF;
    if (!__builtin_parity(result32 & static_cast<T>(-1)))
        flags |= PF;
    if ((v1 & (1 << sign_bit)) == (v2 & (1 << sign_bit)) &&
        (v1 & (1 << sign_bit)) != (result32 & (1 << sign_bit)))
        flags |= OF;

    registers->set_flags(flags);

    return result32;
}

template <typename T>
T Emulator::do_add(uint16_t v1, uint16_t v2, uint16_t carry_in)
{
    return do_alu<T>(v1, v2, carry_in,
        [](uint32_t a, uint32_t b, uint32_t c) -> uint32_t {
            return a + b + c;
        });
}

// add r, r/m, 8-bit
void Emulator::add00()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());

    auto result = do_add<uint8_t>(v1, v2);

    write_data<uint8_t>(result & 0xff);
}

// add r, r/m, 16-bit
void Emulator::add01()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());

    auto result = do_add<uint16_t>(v1, v2);

    write_data<uint16_t>(result & 0xffff);
}

// add r/m, r, 8-bit
void Emulator::add02()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());

    auto result = do_add<uint8_t>(v1, v2);

    registers->set(modrm_decoder->reg(), result & 0xff);
}

// add r/m, r, 16-bit
void Emulator::add03()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());

    auto result = do_add<uint16_t>(v1, v2);

    registers->set(modrm_decoder->reg(), result & 0xffff);
}

void Emulator::add_adc_80()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2)
        return;

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = fetch_byte();
    bool carry_in = modrm_decoder->raw_reg() == 2 ?
        !!(registers->get_flags() & CF) : 0;

    auto result = do_add<uint8_t>(v1, v2, carry_in);

    write_data<uint8_t>(result & 0xff);
}

void Emulator::add_adc_82()
{
    // The 's' bit has no effect for 8-bit add immediate.
    add_adc_80();
}

// add r/m, immediate, 16-bit
void Emulator::add_adc_81()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2)
        return;

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = fetch_16bit();
    bool carry_in = modrm_decoder->raw_reg() == 2 ?
        !!(registers->get_flags() & CF) : 0;

    auto result = do_add<uint16_t>(v1, v2, carry_in);

    write_data<uint16_t>(result & 0xffff);
}

// add r/m, immediate, 8-bit, sign-extended
void Emulator::add_adc_83()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    if (modrm_decoder->raw_reg() != 0 &&
        modrm_decoder->raw_reg() != 2)
        return;

    uint16_t v1 = read_data<uint16_t>();
    int16_t immed = fetch_byte();
    immed <<= 8;
    immed >>= 8;
    bool carry_in = modrm_decoder->raw_reg() == 2 ?
        !!(registers->get_flags() & CF) : 0;

    auto result = do_add<uint16_t>(v1, immed, carry_in);

    write_data<uint16_t>(result & 0xffff);
}

void Emulator::add04()
{
    auto v1 = registers->get(AL);
    auto v2 = fetch_byte();
    auto result = do_add<uint8_t>(v1, v2);

    registers->set(AL, result);
}

void Emulator::add05()
{
    auto v1 = registers->get(AX);
    auto v2 = fetch_16bit();
    auto result = do_add<uint16_t>(v1, v2);

    registers->set(AX, result);
}

// adc r, r/m, 8-bit
void Emulator::adc10()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    auto result = do_add<uint8_t>(v1, v2, carry_in);

    write_data<uint8_t>(result & 0xff);
}

// adc r, r/m, 16-bit
void Emulator::adc11()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    auto result = do_add<uint16_t>(v1, v2, carry_in);

    write_data<uint16_t>(result & 0xffff);
}

// adc r/m, r, 8-bit
void Emulator::adc12()
{
    modrm_decoder->set_width(OP_WIDTH_8);
    modrm_decoder->decode();

    uint8_t v1 = read_data<uint8_t>();
    uint8_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    auto result = do_add<uint8_t>(v1, v2, carry_in);

    registers->set(modrm_decoder->reg(), result & 0xff);
}

// adc r/m, r, 16-bit
void Emulator::adc13()
{
    modrm_decoder->set_width(OP_WIDTH_16);
    modrm_decoder->decode();

    uint16_t v1 = read_data<uint16_t>();
    uint16_t v2 = registers->get(modrm_decoder->reg());
    bool carry_in = !!(registers->get_flags() & CF);

    auto result = do_add<uint16_t>(v1, v2, carry_in);

    registers->set(modrm_decoder->reg(), result & 0xffff);
}

void Emulator::adc14()
{
    auto v1 = registers->get(AL);
    auto v2 = fetch_byte();
    bool carry_in = !!(registers->get_flags() & CF);
    auto result = do_add<uint8_t>(v1, v2, carry_in);

    registers->set(AL, result);
}

void Emulator::adc15()
{
    auto v1 = registers->get(AX);
    auto v2 = fetch_16bit();
    bool carry_in = !!(registers->get_flags() & CF);
    auto result = do_add<uint16_t>(v1, v2, carry_in);

    registers->set(AX, result);
}

uint8_t Emulator::fetch_byte()
{
    ++instr_length;

    return instr_stream->pop();
}

uint16_t Emulator::fetch_16bit()
{
    uint16_t immed = (static_cast<uint16_t>(fetch_byte()) |
                      (static_cast<uint16_t>(fetch_byte()) << 8));
    return immed;
}

template <typename T>
void Emulator::write_data(T val, bool stack)
{
    if (modrm_decoder->rm_type() == OP_REG) {
        auto dest = modrm_decoder->rm_reg();
        registers->set(dest, val);
    } else {
        auto ea = modrm_decoder->effective_address();
        auto segment = modrm_decoder->uses_bp_as_base() || stack ? SS : DS;
        auto addr = get_phys_addr(registers->get(segment), ea);
        mem->write<T>(addr, val);
    }
}

template <typename T>
T Emulator::read_data(bool stack)
{
    if (modrm_decoder->rm_type() == OP_MEM) {
        auto displacement = modrm_decoder->effective_address();
        auto segment = modrm_decoder->uses_bp_as_base() || stack ? SS : DS;
        auto addr = get_phys_addr(registers->get(segment), displacement);

        return mem->read<T>(addr);
    } else {
        auto source = modrm_decoder->rm_reg();
        return registers->get(source);
    }
}
