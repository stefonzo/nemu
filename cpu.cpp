#include "cpu.hpp"

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
void mos6502::ADC_IMMEDIATE() {
    uint16_t address = pc++;
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
    uint16_t address = (uint16_t)read(pc++);
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
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    uint16_t address = (uint16_t(ha) << 8) | la;
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
    
}
void mos6502::ADC_Y_ABSOLUTE() {
    
}
void mos6502::ADC_X_ZEROPAGE() {
    
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
void mos6502::DEY() {
    y -= 1;
    //check N flag
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
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    uint16_t address = (uint16_t(ha) << 8) | la;
    pc = address;
    cycles_count += 3;
}
void mos6502::JMP_ABSOLUTE_INDIRECT() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    uint16_t jmp_address = (uint16_t(ha) << 8) | la;
    la = read(jmp_address++);
    ha = read(jmp_address);
    jmp_address = (uint16_t(ha) << 8) | la;
    pc = jmp_address;
    cycles_count += 5;
}
void mos6502::LDA_IMMEDIATE() {
    uint16_t address = pc++;
    a = read(address);
    cycles_count += 2;
}
void mos6502::LDA_ABSOLUTE() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    uint16_t address = (uint16_t(ha) << 8) | la;
    a = read(address);
    cycles_count += 4;
}
void mos6502::LDA_ZEROPAGE() {
    uint16_t address = (uint16_t)read(pc++);
    a = read(address);
    cycles_count += 3;
}
void mos6502::NOP() {
    cycles_count += 2;
}
void mos6502::PHA() {
    write(SB + sp--, a);
    cycles_count += 3;
}
void mos6502::PHP() {
    write(SB + sp--, a);
    cycles_count += 3;
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
    opcodeTable[0xA5] = &mos6502::LDA_ZEROPAGE;
    opcodeTable[0xEA] = &mos6502::NOP;
    opcodeTable[0x48] = &mos6502::PHA;
    opcodeTable[0x08] = &mos6502::PHP;
    
}

mos6502::mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t)) {
    read = Read;
    write = Write;
    cycles_count = 0, executed_instructions = 0;
    InitializeOpcodeTable();
}
