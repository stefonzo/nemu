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
    
}
void mos6502::ZeroPageYIndirectAddress() {
    
}
void mos6502::ADC_IMMEDIATE() {
    ImmediateAddress();
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
void mos6502::ADC_ZEROPAGE() {
    ZeroPageAddress();
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
void mos6502::ADC_ABSOLUTE() {
    AbsoluteAddress();
    uint8_t m = read(address);
    uint8_t c = flag & C;
    uint16_t sum = a + m + c;
    CarryCheck(sum);
    ZeroCheck(sum);
    SignCheck(sum);
    OverflowCheck(sum);
    a = (uint8_t)sum;
    cycles_count += 4;
}
void mos6502::ADC_X_ABSOLUTE() {
    AbsoluteXAddress();
    uint8_t m = read(address);
    uint8_t c = flag & C;
    uint16_t sum = a + m + c;
    CarryCheck(sum);
    ZeroCheck(sum);
    SignCheck(sum);
    OverflowCheck(sum);
    a = (uint8_t)sum;
    cycles_count += 4;
}
void mos6502::ADC_Y_ABSOLUTE() {
    AbsoluteYAddress();
    uint8_t m = read(address);
    uint8_t c = flag & C;
    uint16_t sum = a + m + c;
    CarryCheck(sum);
    ZeroCheck(sum);
    SignCheck(sum);
    OverflowCheck(sum);
    a = (uint8_t)sum;
    cycles_count += 4;
}
void mos6502::ADC_X_ZEROPAGE() {
    ZeroPageXAddress();
    uint8_t m = read(address);
    uint8_t c = flag & C;
    uint16_t sum = a + m + c;
    CarryCheck(sum);
    ZeroCheck(sum);
    SignCheck(sum);
    OverflowCheck(sum);
    a = (uint8_t)sum;
    cycles_count += 4;
}
void mos6502::ADC_X_ZEROPAGE_INDIRECT() {
    
}
void mos6502::ADC_Y_ZEROPAGE_INDIRECT() {
    
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
    cycles_count += 2;
}
void mos6502::CLV() {
    flag &= 0xBF;
    cycles_count += 2;
}
void mos6502::DEX() {
    x -= 1;
    //check N flag
    SignCheck((uint16_t)x);
    //check Z flag
    ZeroCheck((uint16_t)x);
    cycles_count += 2;
}
void mos6502::DEY() {
    y -= 1;
    //check N flag
    SignCheck((uint16_t)y);
    //check Z flag
    ZeroCheck((uint16_t)y);
    cycles_count += 2;
}
void mos6502::INX() {
    x += 1;
    if (x & N) {
        flag |= N;
    } else {
        flag &= ~(N);
    }
    //check Z flag
    if (x == 0x00) {
        flag |= Z;
    } else {
        flag &= ~(Z);
    }
    cycles_count += 2;
}
void mos6502::INY() {
    y += 1;
    if (y & N) {
        flag |= N;
    } else {
        flag &= ~(N);
    }
    //check Z flag
    if (y == 0x00) {
        flag |= Z;
    } else {
        flag &= ~(Z);
    }
    cycles_count += 2;
}
void mos6502::JMP_ABSOLUTE() {
    AbsoluteAddress();
    pc = address;
    cycles_count += 3;
}
void mos6502::JMP_ABSOLUTE_INDIRECT() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    la = read(address++);
    ha = read(address);
    address = (uint16_t(ha) << 8) | la;
    pc = address;
    cycles_count += 5;
}
void mos6502::LDA_IMMEDIATE() {
    ImmediateAddress();
    a = read(address);
    cycles_count += 2;
}
void mos6502::LDA_ABSOLUTE() {
    AbsoluteAddress();
    a = read(address);
    cycles_count += 4;
}
void mos6502::LDA_X_ABSOLUTE() {
    AbsoluteXAddress();
    a = read(address);
    cycles_count += 4;
}
void mos6502::LDA_Y_ABSOLUTE() {
    AbsoluteYAddress();
    a = read(address);
    cycles_count += 4;
}
void mos6502::LDA_ZEROPAGE() {
    ZeroPageAddress();
    a = read(address);
    cycles_count += 3;
}
void mos6502::LDA_X_ZEROPAGE() {
    address = (uint16_t)read(pc++);
    address += x;
    address &= 0x00FF; //zero page wrap around
    a = read(address);
    cycles_count += 4;
}
void mos6502::NOP() {
    cycles_count += 2;
}
void mos6502::PHA() {
    write(SB + sp--, a);
    cycles_count += 3;
}
void mos6502::PHP() {
    write(SB + sp--, flag | 0x10);
    cycles_count += 3;
}
void mos6502::PLA() {
    sp++;
    a = read(SB + sp);
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
    cycles_count += 4;
}
void mos6502::PLP() {
    sp++;
    flag = (read(SB + sp) & 0xCF) | 0x20;
    cycles_count +=4;
}
void mos6502::RTS() {
    
}
void mos6502::SEC() {
    flag |= C;
    cycles_count += 2;
}
void mos6502::SED() {
    flag |= D;
    cycles_count += 2;
}
void mos6502::SEI() {
    flag |= I;
    cycles_count += 2;
}
void mos6502::TAX() {
    x = a;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
    cycles_count += 2;
}
void mos6502::TAY() {
    y = a;
    SignCheck((uint16_t)y);
    ZeroCheck((uint16_t)y);
    cycles_count += 2;
}
void mos6502::TSX() {
    x = sp;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
    cycles_count += 2;
}
void mos6502::TXA() {
    a = x;
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
    cycles_count += 2;
}
void mos6502::TXS() {
    sp = x;
    cycles_count += 2;
}
void mos6502::TYA() {
    a = y;
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
    cycles_count += 2;
}
void mos6502::InitializeOpcodeTable() {
    opcodeTable[0x69] = &mos6502::ADC_IMMEDIATE;
    opcodeTable[0x65] = &mos6502::ADC_ZEROPAGE;
    opcodeTable[0x6D] = &mos6502::ADC_ABSOLUTE;
    opcodeTable[0x7D] = &mos6502::ADC_X_ABSOLUTE;
    opcodeTable[0x79] = &mos6502::ADC_Y_ABSOLUTE;
    opcodeTable[0x75] = &mos6502::ADC_X_ZEROPAGE;
    opcodeTable[0x61] = &mos6502::ADC_X_ZEROPAGE_INDIRECT;
    opcodeTable[0x71] = &mos6502::ADC_Y_ZEROPAGE_INDIRECT;
    opcodeTable[0x90] = &mos6502::BCC;
    opcodeTable[0xB0] = &mos6502::BCS;
    opcodeTable[0xF0] = &mos6502::BEQ;
    opcodeTable[0x18] = &mos6502::CLC;
    opcodeTable[0xD8] = &mos6502::CLD;
    opcodeTable[0x58] = &mos6502::CLI;
    opcodeTable[0xB8] = &mos6502::CLV;
    opcodeTable[0xCA] = &mos6502::DEX;
    opcodeTable[0x88] = &mos6502::DEY;
    opcodeTable[0xE8] = &mos6502::INX;
    opcodeTable[0xC8] = &mos6502::INY;
    opcodeTable[0x4C] = &mos6502::JMP_ABSOLUTE;
    opcodeTable[0x6C] = &mos6502::JMP_ABSOLUTE_INDIRECT;
    opcodeTable[0xA9] = &mos6502::LDA_IMMEDIATE;
    opcodeTable[0xAD] = &mos6502::LDA_ABSOLUTE;
    opcodeTable[0xBD] = &mos6502::LDA_X_ABSOLUTE;
    opcodeTable[0xB9] = &mos6502::LDA_Y_ABSOLUTE;
    opcodeTable[0xA5] = &mos6502::LDA_ZEROPAGE;
    opcodeTable[0xB5] = &mos6502::LDA_X_ZEROPAGE;
    opcodeTable[0xEA] = &mos6502::NOP;
    opcodeTable[0x48] = &mos6502::PHA;
    opcodeTable[0x08] = &mos6502::PHP;
    opcodeTable[0x68] = &mos6502::PLA;
    opcodeTable[0x28] = &mos6502::PLP;
    opcodeTable[0x60] = &mos6502::RTS;
    opcodeTable[0x38] = &mos6502::SEC;
    opcodeTable[0xF8] = &mos6502::SED;
    opcodeTable[0x78] = &mos6502::SEI;
    opcodeTable[0xAA] = &mos6502::TAX;
    opcodeTable[0xA8] = &mos6502::TAY;
    opcodeTable[0xBA] = &mos6502::TSX;
    opcodeTable[0x8A] = &mos6502::TXA;
    opcodeTable[0x9A] = &mos6502::TXS;
    opcodeTable[0x98] = &mos6502::TYA;
}
mos6502::mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t)) {
    read = Read;
    write = Write;
    cycles_count = 0, executed_instructions = 0;
    InitializeOpcodeTable();
}
