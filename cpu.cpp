#include "cpu.hpp"
//flag helper functions
void mos6502::ZeroCheck(uint16_t value) {
    if (value == 0) {
        flag |= Z;
    } else {
        flag &= ~(Z);
    }
}
void mos6502::CarryCheck(uint16_t value) {
    if (value & 0xFF00) {
        flag |= C;
    } else {
        flag &= ~(C);
    }
}
void mos6502::OverflowCheck(uint16_t value){
    if (((uint8_t)value & N) ^ (a & N)) {
        flag |= V;
    } else {
        flag &= ~(V);
    }
}
void mos6502::SignCheck(uint16_t value) {
    if (value & 0x0080) {
        flag |= N;
    } else {
        flag &= ~(N);
    }
}
//Address helper functions
void mos6502::ImmediateAddress() {
    address = pc++;
}
void mos6502::AbsoluteAddress() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
}
void mos6502::ZeroPageAddress() {
    address = (uint16_t)read(pc++);
}
void mos6502::AbsoluteXAddress() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    uint16_t startpage = address & 0xFF00;
    address += x;
    if (startpage != (address & 0xFF00)) cycles_count++; //page check from rubbermallet
}
void mos6502::AbsoluteYAddress() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    uint16_t startpage = address & 0xFF00;
    address += y;
    if (startpage != (address & 0xFF00)) cycles_count++;
}
void mos6502::ZeroPageXAddress() {
    address = ((uint16_t)read(pc++) + x) & 0x00FF;
}
void mos6502::ZeroPageYAddress() {
    address = ((uint16_t)read(pc++) + y) & 0x00FF;
}
void mos6502::ZeroPageXIndirectAddress() {
    address = ((uint16_t)read(pc++) + x) & 0x00FF;
    uint8_t la = read(address);
    uint8_t ha = read((address+1) & 0x00FF); //indirect address is in zero page so need to ensure that wrap around occurs
    address = (uint16_t(ha) << 8) | la;
}
void mos6502::ZeroPageYIndirectAddress(bool addExtraCycle) {
    address = ((uint16_t)read(pc++)); //ofc there's one assembly instruction that has a constant number of cycles that uses this addressing mode...
    uint8_t la = read(address);
    uint8_t ha = read((address+1) & 0x00FF);
    address = (uint16_t(ha) << 8) | la;
    uint16_t startpage = address & 0xFF00;
    address += y;
    if (addExtraCycle && (startpage != (address & 0xFF00))) cycles_count++;
}
//6502 instructions
void mos6502::ADC() {
    uint8_t m = read(address);
    uint8_t c = flag & C;
    uint16_t sum = a + m + c;
    CarryCheck(sum);
    ZeroCheck(sum);
    SignCheck(sum);
    OverflowCheck(sum);
    a = (uint8_t)sum;
    cycles_count += 2;
}
void mos6502::BCC() {
    
}
void mos6502::BCS() {
    
}
void mos6502::BEQ() {
    
}
void mos6502::CLC() {
    flag &= 0xFE;
    cycles_count += 2;
}
void mos6502::CLD() {
    flag &= 0xF7;
    cycles_count += 2;
}
void mos6502::CLI() {
    flag &= 0xFB;
}
void mos6502::CLV() {
    flag &= 0xBF;
}
void mos6502::DEX() {
    x -= 1;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
}
void mos6502::DEY() {
    y -= 1;
    SignCheck((uint16_t)y);
    ZeroCheck((uint16_t)y);
}
void mos6502::INX() {
    x += 1;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
}
void mos6502::INY() {
    y += 1;
    SignCheck((uint16_t)y);
    ZeroCheck((uint16_t)y);
}
void mos6502::JMP() {
    pc = address;
}
void mos6502::LDA() {
    ImmediateAddress();
    a = read(address);
}
void mos6502::NOP() {
    //do nothing
}
void mos6502::PHA() {
    write(SB + sp--, a);
}
void mos6502::PHP() {
    write(SB + sp--, flag | 0x10);
}
void mos6502::PLA() {
    sp++;
    a = read(SB + sp);
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::PLP() {
    sp++;
    flag = (read(SB + sp) & 0xCF) | 0x20;}
void mos6502::RTS() {
    
}
void mos6502::SEC() {
    flag |= C;
}
void mos6502::SED() {
    flag |= D;
}
void mos6502::SEI() {
    flag |= I;}
void mos6502::STA() {
    write(address, a);
}

void mos6502::TAX() {
    x = a;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
}
void mos6502::TAY() {
    y = a;
    SignCheck((uint16_t)y);
    ZeroCheck((uint16_t)y);}
void mos6502::TSX() {
    x = sp;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
}
void mos6502::TXA() {
    a = x;
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::TXS() {
    sp = x;
}
void mos6502::TYA() {
    a = y;
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
uint8_t mos6502::FetchInstruction() {
    return read(pc++);
}
void mos6502::InitializeOpcodeTable() {
    //ADC immediate
    opcodeTableInstruction.address_mode = &mos6502::ImmediateAddress;
    opcodeTableInstruction.instruction = &mos6502::ADC;
    opcodeTableInstruction.cycles = 2;
    opcodeTable[0x69] = opcodeTableInstruction;
}
mos6502::mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t)) {
    read = Read;
    write = Write;
    cycles_count = 0, executed_instructions = 0;
    InitializeOpcodeTable();
}
void mos6502::StepInstruction() {
    uint8_t opcode = FetchInstruction();
    Instruction step_instruction = opcodeTable[opcode];
    (this->*step_instruction.address_mode)();
    (this->*step_instruction.instruction)();
    cycles_count += step_instruction.cycles;
    executed_instructions += 1;
}
