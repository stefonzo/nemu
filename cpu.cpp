#include "cpu.hpp"
//flag helper functions
void mos6502::ZeroCheck(uint16_t value) { // 8 bit register values are typecast to uint16_t before being checked by flag helper functions (useful for CarryCheck)
    if (value & 0x00FF) { // if value = 0 then condition is false and zero flag is set
        flag &= ~(Z); // value != 0 so flag is anded with inverse of the zero mask so all bits of the flag are ignored except for the zero bit which is set to 0
    } else {
        flag |= Z;
    }
}
void mos6502::CarryCheck(uint16_t value) {
    if (value & 0xFF00) { // if two 8 bit numbers sum overflows bit 9 will be set to 1, if the 9th bit of value is 1 then condition is true and the carry flag bit is set to 1 (otherwise it is set to 0)
        flag |= C;
    } else {
        flag &= ~(C);
    }
}
void mos6502::OverflowCheck(uint16_t value){
    if (((uint8_t)value & N) ^ (a & N)) { //had to work out math/logic for this conditional test on paper, you and the two values to capture the N bit which is what you need to test in the overflow check. Xoring the bits enables you to determine if they are different (ie: if they have changed after a operation)
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
void mos6502::AbsoluteXAddressConstant() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    address += x;
}
void mos6502::AbsoluteYAddressConstant() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    address += y;
}
void mos6502::AbsoluteXAddressVariable() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    uint16_t startpage = address & 0xFF00;
    address += x;
    if (startpage != (address & 0xFF00)) cycles_count++; //page check from rubbermallet
}
void mos6502::AbsoluteYAddressVariable() {
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    address = (uint16_t(ha) << 8) | la;
    uint16_t startpage = address & 0xFF00;
    address += y;
    if (startpage != (address & 0xFF00)) cycles_count++;
}
void mos6502::AbsoluteIndirectAddress() { //need to implement page wraparound bug!
    uint8_t la = read(pc++);
    uint8_t ha = read(pc++);
    uint16_t pointer = (uint16_t(ha) << 8) | la; //low order byte of the effective address
    uint8_t ea_low = read(pointer);
    uint8_t ea_high = read((pointer & 0xFF00) | ((pointer + 1) & 0x00FF)); //gian/chatgpt showed me this algorithm to simulate the pagewrapping bug. I still need to figure out how it works
    address = (uint16_t(ea_high) << 8) | ea_low;
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
void mos6502::ZeroPageYIndirectAddressConstant() {
    address = ((uint16_t)read(pc++)); //ofc there's one assembly instruction that has a constant number of cycles that uses this addressing mode...
    uint8_t la = read(address);
    uint8_t ha = read((address+1) & 0x00FF);
    address = (uint16_t(ha) << 8) | la;
    address += y;
}
void mos6502::ZeroPageYIndirectAddressVariable() {
    address = ((uint16_t)read(pc++)); //ofc there's one assembly instruction that has a constant number of cycles that uses this addressing mode...
    uint8_t la = read(address);
    uint8_t ha = read((address+1) & 0x00FF);
    address = (uint16_t(ha) << 8) | la;
    uint16_t startpage = address & 0xFF00;
    //add page check
    address += y;
    if (startpage != (address & 0xFF00)) cycles_count++;
}
void mos6502::RelativeAddress() { //fix this function then code should work
    int8_t offset = read(pc++);
    uint16_t startpage = address & 0xFF00;
    address = pc + offset;
    if ((address & 0xFF00) != startpage) cycles_count++;
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
}
void mos6502::AND() {
    uint8_t m = read(address);
    a = m & a;
    ZeroCheck((uint16_t)a);
    SignCheck((uint16_t)a);
}
void mos6502::ASL() {
    uint8_t m = read(address);
    if (m & N) //put bit 7 of operand into carry
        flag |= C;
    else
        flag &= ~(C);
    m = m << 1;
    //set bit 0 of m to 0
    m &= ~(C);
    SignCheck((uint16_t)m);
    ZeroCheck((uint16_t)m);
    write(address, m);
}
void mos6502::ASL_ACC() {
    if (a & N)
        flag |= C;
    else
        flag &= ~(C);
    a = a << 1;
    a &= ~(C);
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::BCC() {
    if (!(flag & C)) { //branch if carry bit is 0
        pc = address;
        cycles_count++; //extra cycle is required if a branch is taken
    }
}
void mos6502::BCS() {
    if (flag & C) { //branch if carry bit is 1
        pc = address;
        cycles_count++;
    }
}
void mos6502::BEQ() {
    if (flag & Z) { //branch if zero bit is 1
        pc = address;
        cycles_count++;
    }
}
void mos6502::BIT() {
    uint8_t m = read(address);
    m &= a;
    SignCheck((uint16_t)m);
    ZeroCheck((uint16_t)m);
}
void mos6502::BMI() {
    if (flag & N) { //branch if negative bit is 1
        pc = address;
        cycles_count++;
    }
}
void mos6502::BNE() {
    if (!(flag & Z)) { //branch if zero bit is 0
        pc = address;
        cycles_count++;
    }
}
void mos6502::BPL() {
    if (!(flag & N)) { //branch if negative bit is 0
        pc = address;
        cycles_count++;
    }
}
void mos6502::BRK() {
    pc++; //increment pc so that we can return to address after brk instruction when interrupt is finished
    uint8_t pcl = (uint8_t)pc; //get pc into bytes and push them to the stack
    uint8_t pch = pc >> 8;
    write(SB + sp--, pch);
    write(SB + sp--, pcl);
    write(SB + sp--, flag | 0x30); //should I set the unused bit (0x20)?
    flag |= I; //set interrupt bit of flag to 1
    uint8_t i_lo = read(IRQ_LOW);
    uint8_t i_hi = read(IRQ_HIGH);
    pc = (uint16_t(i_hi) << 8) | i_lo;
}
void mos6502::BVC() {
    if (!(flag & V)) { //branch if overflow big is 0
        pc = address;
        cycles_count++;
    }
}
void mos6502::BVS() {
    if (flag & V) { //branch if overflow bit is 1
        pc = address;
        cycles_count++;
    }
}
void mos6502::CLC() {
    flag &= 0xFE;
}
void mos6502::CLD() {
    flag &= 0xF7;
}
void mos6502::CLI() {
    flag &= 0xFB;
}
void mos6502::CLV() {
    flag &= 0xBF;
}
void mos6502::CMP() {
    uint8_t m = read(address);
    uint16_t test = a - m;
    ZeroCheck((uint16_t)test);
    SignCheck((uint16_t)test);
    if ( m <= a)
        flag |= C;
    else
        flag &= ~(C);
}
void mos6502::CPX() {
    uint8_t m = read(address);
    uint16_t test = x - m;
    ZeroCheck((uint16_t)test);
    SignCheck((uint16_t)test);
    if ( m <= x)
        flag |= C;
    else
        flag &= ~(C);
}
void mos6502::CPY() {
    uint8_t m = read(address);
    uint16_t test = y - m;
    ZeroCheck((uint16_t)test);
    SignCheck((uint16_t)test);
    if ( m <= y)
        flag |= C;
    else
        flag &= ~(C);
}
void mos6502::DEC() {
    uint8_t m = read(address);
    m -= 1;
    write(address, m);
    SignCheck((uint16_t)m);
    ZeroCheck((uint16_t)m);
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
void mos6502::EOR() {
    uint8_t m = read(address);
    a = a ^ m;
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::INC() {
    uint8_t m = read(address);
    m += 1;
    write(address, m);
    SignCheck((uint16_t)m);
    ZeroCheck((uint16_t)m);
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
void mos6502::JMP() { //need to simulate page wraparound bug for indirect addressing...
    pc = address;
}
void mos6502::JSR() {
    //push pc onto stack
    pc--;
    uint8_t pcl = (uint8_t)pc;
    uint8_t pch = pc >> 8;
    write(SB + sp--, pch);
    write(SB + sp--, pcl); //hope I have these bytes in the right order xD
    pc = address;
}
void mos6502::LDA() {
    a = read(address);
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::LDX() {
    x = read(address);
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
}
void mos6502::LDY() {
    y = read(address);
    SignCheck((uint16_t)y);
    ZeroCheck((uint16_t)y);
}
void mos6502::LSR() {
    uint8_t m = read(address);
    if (m & C) //put bit 0 of operand into carry
        flag |= C;
    else
        flag &= ~(C);
    m = m >> 1;
    m &= ~(N); //set bit 0 of m to 0
    SignCheck((uint16_t)m);
    ZeroCheck((uint16_t)m);
    write(address, m);
}
void mos6502::LSR_ACC() {
    if (a & C)
        flag |= C;
    else
        flag &= ~(C);
    a = a >> 1;
    a &= ~(N);
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::NOP() {
    //do nothing
}
void mos6502::ORA() {
    uint8_t m = read(address);
    a = a | m;
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::PHA() {
    write(SB + sp--, a);
}
void mos6502::PHP() {
    write(SB + sp--, flag | B | 0x20); //set constant?
}
void mos6502::PLA() {
    sp++;
    a = read(SB + sp);
    SignCheck((uint16_t)a);
    ZeroCheck((uint16_t)a);
}
void mos6502::PLP() {
    sp++;
    flag = (read(SB + sp) & 0xCF) | 0x20;
}
void mos6502::ROL() {
    uint8_t m = read(address);
    uint8_t c = flag & C; //get carry bit
    uint8_t msb = m & N; //get msb
    m = m << 1;
    if (c) m |= C; //move carry bit to lsb
    ZeroCheck((uint16_t)m);
    SignCheck((uint16_t)m);
    if (msb) //move msb to carry bit
        flag |= C;
    else
        flag &= ~(C);
    write(address, m);
}
void mos6502::ROL_ACC() {
    uint8_t c = flag & C; //get carry bit
    uint8_t msb = a & N; //get msb
    a = a << 1;
    if (c) a |= C; //move carry bit to lsb
    ZeroCheck((uint16_t)a);
    SignCheck((uint16_t)a);
    if (msb) //move msb to carry bit
        flag |= C;
    else
        flag &= ~(C);
}
void mos6502::ROR() {
    uint8_t m = read(address);
    uint8_t c = flag & C; //get carry bit
    uint8_t lsb = m & C; //get lsb
    m = m >> 1;
    if (c) m |= N;//move carry bit to msb
    ZeroCheck((uint16_t)m);
    SignCheck((uint16_t)m);
    if (lsb) //move lsb to carry bit
        flag |= C;
    else
        flag &= ~(C);
    write(address, m);
}
void mos6502::ROR_ACC() {
    uint8_t c = flag & C; //get carry bit
    uint8_t lsb = a & C; //get lsb
    a = a >> 1;
    if (c) a |= N;//move carry bit to msb
    ZeroCheck((uint16_t)a);
    SignCheck((uint16_t)a);
    if (lsb) //move lsb to carry bit
        flag |= C;
    else
        flag &= ~(C);
}
void mos6502::RTI() {
    //from stack need to read flag and pc
    sp++;
    flag = read(SB + sp);
    sp++;
    uint8_t la = read(SB + sp);
    sp++;
    uint8_t ha = read(SB + sp);
    pc = (uint16_t(ha) << 8) | la;
}
void mos6502::RTS() {
    //load pc lo and pc hi from stack into pc
    sp++;
    uint8_t la = read(SB + sp);
    sp++;
    uint8_t ha = read(SB + sp);
    pc = (uint16_t(ha) << 8) | la;
    //increment pc so it points to instruction following the JSR
    pc++;
}
void mos6502::SBC() {
    uint8_t m = read(address);
    m ^= 0xFF; //treating subtraction how hardware handles it (a - m) = (a + ^m + c)
    uint8_t c = flag & C;
    uint16_t difference = a + m + c;
    CarryCheck(difference);
    OverflowCheck(difference);
    SignCheck(difference);
    ZeroCheck(difference);
    a = (uint8_t)difference;
}
void mos6502::SEC() {
    flag |= C;
}
void mos6502::SED() {
    flag |= D;
}
void mos6502::SEI() {
    flag |= I;
}
void mos6502::STA() {
    write(address, a);
}
void mos6502::STX() {
    write(address, x);
}
void mos6502::STY() {
    write(address, y);
}
void mos6502::TAX() {
    x = a;
    SignCheck((uint16_t)x);
    ZeroCheck((uint16_t)x);
}
void mos6502::TAY() {
    y = a;
    SignCheck((uint16_t)y);
    ZeroCheck((uint16_t)y);
}
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
    Instruction instruction;
    //initalize opcode table with all nops so any undocumented instructions are treated as such
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::NOP;
    instruction.cycles = 2;
    for (uint8_t i = 0; i < 0xFF; i++) {
        opcodeTable[i] = instruction;
    }
    //ADC instructions
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 2;
    opcodeTable[0x69] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 4;
    opcodeTable[0x6D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 4;
    opcodeTable[0x7D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 4;
    opcodeTable[0x79] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 3;
    opcodeTable[0x65] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 4;
    opcodeTable[0x75] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 6;
    opcodeTable[0x61] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::ADC;
    instruction.cycles = 5;
    opcodeTable[0x71] = instruction;
    
    //AND instructions
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 2;
    opcodeTable[0x29] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 4;
    opcodeTable[0x2D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 4;
    opcodeTable[0x3D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 4;
    opcodeTable[0x39] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 3;
    opcodeTable[0x25] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 4;
    opcodeTable[0x35] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 6;
    opcodeTable[0x21] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::AND;
    instruction.cycles = 5;
    opcodeTable[0x31] = instruction;
    
    //ASL instructions
    instruction.address_mode = &mos6502::AccumulatorAddress;
    instruction.instruction = &mos6502::ASL_ACC;
    instruction.cycles = 2;
    opcodeTable[0x0A] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::ASL;
    instruction.cycles = 6;
    opcodeTable[0x0E] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::ASL;
    instruction.cycles = 7;
    opcodeTable[0x1E] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::ASL;
    instruction.cycles = 5;
    opcodeTable[0x06] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::ASL;
    instruction.cycles = 6;
    opcodeTable[0x16] = instruction;
    //branch instructions
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BCC;
    instruction.cycles = 2;
    opcodeTable[0x90] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BCS;
    instruction.cycles = 2;
    opcodeTable[0xB0] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BEQ;
    instruction.cycles = 2;
    opcodeTable[0xF0] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BMI;
    instruction.cycles = 2;
    opcodeTable[0x30] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BNE;
    instruction.cycles = 2;
    opcodeTable[0xD0] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BPL;
    instruction.cycles = 2;
    opcodeTable[0x10] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BVC;
    instruction.cycles = 2;
    opcodeTable[0x50] = instruction;
    
    instruction.address_mode = &mos6502::RelativeAddress;
    instruction.instruction = &mos6502::BVS;
    instruction.cycles = 2;
    opcodeTable[0x70] = instruction;
    
    //BIT instruction
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::BIT;
    instruction.cycles = 4;
    opcodeTable[0x2C] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::BIT;
    instruction.cycles = 3;
    opcodeTable[0x24] = instruction;
    
    //BRK instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::BRK;
    instruction.cycles = 7;
    opcodeTable[0x00] = instruction;
    
    //Clear flag instructions
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::CLC;
    instruction.cycles = 2;
    opcodeTable[0x18] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::CLD;
    instruction.cycles = 2;
    opcodeTable[0xD8] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::CLI;
    instruction.cycles = 2;
    opcodeTable[0x58] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::CLV;
    instruction.cycles = 2;
    opcodeTable[0xB8] = instruction;
    
    //CMP instruction
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 2;
    opcodeTable[0xC9] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 4;
    opcodeTable[0xCD] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 4;
    opcodeTable[0xDD] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 4;
    opcodeTable[0xD9] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 3;
    opcodeTable[0xC5] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 4;
    opcodeTable[0xD5] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 6;
    opcodeTable[0xC1] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::CMP;
    instruction.cycles = 5;
    opcodeTable[0xD1] = instruction;
    
    //CPX instruction
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::CPX;
    instruction.cycles = 2;
    opcodeTable[0xE0] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::CPX;
    instruction.cycles = 4;
    opcodeTable[0xEC] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::CPX;
    instruction.cycles = 3;
    opcodeTable[0xE4] = instruction;
    
    //CPY instruction
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::CPY;
    instruction.cycles = 2;
    opcodeTable[0xC0] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::CPY;
    instruction.cycles = 4;
    opcodeTable[0xCC] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::CPY;
    instruction.cycles = 3;
    opcodeTable[0xC4] = instruction;
    
    //DEC instruction
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::DEC;
    instruction.cycles = 6;
    opcodeTable[0xCE] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::DEC;
    instruction.cycles = 7;
    opcodeTable[0xDE] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::DEC;
    instruction.cycles = 5;
    opcodeTable[0xC6] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::DEC;
    instruction.cycles = 6;
    opcodeTable[0xD6] = instruction;
    
    //DEX instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::DEX;
    instruction.cycles = 2;
    opcodeTable[0xCA] = instruction;
    
    //DEY instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::DEY;
    instruction.cycles = 2;
    opcodeTable[0x88] = instruction;
    
    //EOR instruction
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 2;
    opcodeTable[0x49] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 4;
    opcodeTable[0x4D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 4;
    opcodeTable[0x5D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 4;
    opcodeTable[0x59] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 3;
    opcodeTable[0x45] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 4;
    opcodeTable[0x55] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 6;
    opcodeTable[0x41] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::EOR;
    instruction.cycles = 5;
    opcodeTable[0x51] = instruction;
    
    //INC instruction
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::INC;
    instruction.cycles = 6;
    opcodeTable[0xEE] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::INC;
    instruction.cycles = 7;
    opcodeTable[0xFE] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::INC;
    instruction.cycles = 5;
    opcodeTable[0xE6] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::INC;
    instruction.cycles = 6;
    opcodeTable[0xF6] = instruction;
    
    //INX instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::INX;
    instruction.cycles = 2;
    opcodeTable[0xE8] = instruction;
    
    
    //INY instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::INY;
    instruction.cycles = 2;
    opcodeTable[0xC8] = instruction;
    
    //JMP instruction
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::JMP;
    instruction.cycles = 3;
    opcodeTable[0x4C] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteIndirectAddress;
    instruction.instruction = &mos6502::JMP;
    instruction.cycles = 5;
    opcodeTable[0x6C] = instruction;
    
    //JSR instruction
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::JSR;
    instruction.cycles = 6;
    opcodeTable[0x20] = instruction;
    
    //Load instructions
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 2;
    opcodeTable[0xA9] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 4;
    opcodeTable[0xAD] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 4;
    opcodeTable[0xBD] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 4;
    opcodeTable[0xB9] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 3;
    opcodeTable[0xA5] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 4;
    opcodeTable[0xB5] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 6;
    opcodeTable[0xB1] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::LDA;
    instruction.cycles = 5;
    opcodeTable[0xB1] = instruction;
    
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::LDX;
    instruction.cycles = 2;
    opcodeTable[0xA2] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::LDX;
    instruction.cycles = 4;
    opcodeTable[0xAE] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::LDX;
    instruction.cycles = 4;
    opcodeTable[0xBE] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::LDX;
    instruction.cycles = 3;
    opcodeTable[0xA6] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYAddress;
    instruction.instruction = &mos6502::LDX;
    instruction.cycles = 2;
    opcodeTable[0xB6] = instruction;
    
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::LDY;
    instruction.cycles = 2;
    opcodeTable[0xA0] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::LDY;
    instruction.cycles = 4;
    opcodeTable[0xAC] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::LDY;
    instruction.cycles = 4;
    opcodeTable[0xBC] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::LDY;
    instruction.cycles = 3;
    opcodeTable[0xA4] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::LDY;
    instruction.cycles = 4;
    opcodeTable[0xB4] = instruction;
    
    //LSR instruction
    instruction.address_mode = &mos6502::AccumulatorAddress;
    instruction.instruction = &mos6502::LSR_ACC;
    instruction.cycles = 2;
    opcodeTable[0x4A] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::LSR;
    instruction.cycles = 6;
    opcodeTable[0x4E] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::LSR;
    instruction.cycles = 7;
    opcodeTable[0x5E] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::LSR;
    instruction.cycles = 5;
    opcodeTable[0x46] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::LSR;
    instruction.cycles = 6;
    opcodeTable[0x56] = instruction;
    
    //NOP instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::NOP;
    instruction.cycles = 2;
    opcodeTable[0xEA] = instruction;
    
    //ORA instruction
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 2;
    opcodeTable[0x09] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 4;
    opcodeTable[0x0D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 4;
    opcodeTable[0x1D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 4;
    opcodeTable[0x19] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 3;
    opcodeTable[0x05] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 4;
    opcodeTable[0x15] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 6;
    opcodeTable[0x01] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::ORA;
    instruction.cycles = 5;
    opcodeTable[0x11] = instruction;
    
    //PHA instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::PHA;
    instruction.cycles = 3;
    opcodeTable[0x48] = instruction;
    
    //PHP instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::PHP;
    instruction.cycles = 3;
    opcodeTable[0x08] = instruction;
    
    //PLA instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::PLA;
    instruction.cycles = 4;
    opcodeTable[0x68] = instruction;
    
    //PLP instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::PLP;
    instruction.cycles = 4;
    opcodeTable[0x28] = instruction;
    
    //ROL instruction
    instruction.address_mode = &mos6502::AccumulatorAddress;
    instruction.instruction = &mos6502::ROL_ACC;
    instruction.cycles = 2;
    opcodeTable[0x2A] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::ROL;
    instruction.cycles = 6;
    opcodeTable[0x2E] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::ROL;
    instruction.cycles = 7;
    opcodeTable[0x3E] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::ROL;
    instruction.cycles = 5;
    opcodeTable[0x26] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::ROL;
    instruction.cycles = 6;
    opcodeTable[0x36] = instruction;
    
    //ROR instruction
    instruction.address_mode = &mos6502::AccumulatorAddress;
    instruction.instruction = &mos6502::ROR_ACC;
    instruction.cycles = 2;
    opcodeTable[0x6A] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::ROR;
    instruction.cycles = 6;
    opcodeTable[0x6E] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::ROR;
    instruction.cycles = 7;
    opcodeTable[0x7E] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::ROR;
    instruction.cycles = 5;
    opcodeTable[0x66] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::ROR;
    instruction.cycles = 6;
    opcodeTable[0x76] = instruction;
    
    //RTI instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::RTI;
    instruction.cycles = 6;
    opcodeTable[0x40] = instruction;
    
    //RTS instruction
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::RTS;
    instruction.cycles = 6;
    opcodeTable[0x60] = instruction;
    
    //Store instructions
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 4;
    opcodeTable[0x8D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressConstant;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 5;
    opcodeTable[0x9D] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressConstant;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 5;
    opcodeTable[0x99] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 3;
    opcodeTable[0x85] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 4;
    opcodeTable[0x95] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 6;
    opcodeTable[0x81] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressConstant;
    instruction.instruction = &mos6502::STA;
    instruction.cycles = 6;
    opcodeTable[0x91] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::STX;
    instruction.cycles = 4;
    opcodeTable[0x8E] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::STX;
    instruction.cycles = 3;
    opcodeTable[0x86] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYAddress;
    instruction.instruction = &mos6502::STX;
    instruction.cycles = 4;
    opcodeTable[0x96] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::STY;
    instruction.cycles = 4;
    opcodeTable[0x8C] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::STY;
    instruction.cycles = 3;
    opcodeTable[0x84] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYAddress;
    instruction.instruction = &mos6502::STY;
    instruction.cycles = 4;
    opcodeTable[0x94] = instruction;
    
    //Set flag instructions
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::SEC;
    instruction.cycles = 2;
    opcodeTable[0x38] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::SED;
    instruction.cycles = 2;
    opcodeTable[0xF8] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::SEI;
    instruction.cycles = 2;
    opcodeTable[0x78] = instruction;
    
    //SBC instructions
    instruction.address_mode = &mos6502::ImmediateAddress;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 2;
    opcodeTable[0xE9] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteAddress;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 4;
    opcodeTable[0xED] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteXAddressVariable;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 4;
    opcodeTable[0xFD] = instruction;
    
    instruction.address_mode = &mos6502::AbsoluteYAddressVariable;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 4;
    opcodeTable[0xF9] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageAddress;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 3;
    opcodeTable[0xE5] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXAddress;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 4;
    opcodeTable[0xF5] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageXIndirectAddress;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 6;
    opcodeTable[0xE1] = instruction;
    
    instruction.address_mode = &mos6502::ZeroPageYIndirectAddressVariable;
    instruction.instruction = &mos6502::SBC;
    instruction.cycles = 5;
    opcodeTable[0xF1] = instruction;
    
    //Transfer instructions
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::TAX;
    instruction.cycles = 2;
    opcodeTable[0xAA] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::TAY;
    instruction.cycles = 2;
    opcodeTable[0xA8] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::TSX;
    instruction.cycles = 2;
    opcodeTable[0xBA] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::TXA;
    instruction.cycles = 2;
    opcodeTable[0x8A] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::TXS;
    instruction.cycles = 2;
    opcodeTable[0x9A] = instruction;
    
    instruction.address_mode = &mos6502::ImpliedAddress;
    instruction.instruction = &mos6502::TYA;
    instruction.cycles = 2;
    opcodeTable[0x98] = instruction;
}
mos6502::mos6502() {
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
void mos6502::Reset() {
    a = 0;
    x = 0;
    y = 0;
    flag = 0x20;
    sp = 0xFD;
    //load reset vector into program counter
    uint8_t reset_lo = read(RESET_LOW);
    uint8_t reset_hi = read(RESET_HIGH);
    pc = (uint16_t(reset_hi) << 8) | reset_lo;
}
void mos6502::NMI() {
    //push pc onto the stack
    uint8_t pcl = (uint8_t)pc; //get pc into bytes and push them to the stack
    uint8_t pch = pc >> 8;
    write(SB + sp--, pch);
    write(SB + sp--, pcl);
    //push flag onto the stack
    write(SB + sp--, flag | 0x10); //should I set the unused bit (0x20)?
    //disable interrupts
    flag &= ~(I);
    //load NMI vector
    uint8_t NMI_lo = read(NMI_LOW);
    uint8_t NMI_hi = read(NMI_HIGH);
    pc = (uint16_t(NMI_hi) << 8) | NMI_lo;
}
void mos6502::IRQ() {
    //push pc onto the stack
    uint8_t pcl = (uint8_t)pc; //get pc into bytes and push them to the stack
    uint8_t pch = pc >> 8;
    write(SB + sp--, pch);
    write(SB + sp--, pcl);
    //push flag onto the stack
    write(SB + sp--, flag | 0x10); //should I set the unused bit (0x20)?
    //disable interrupts
    flag &= ~(I);
    //load IRQ vector
    uint8_t IRQ_lo = read(IRQ_LOW);
    uint8_t IRQ_hi = read(IRQ_HIGH);
    pc = (uint16_t(IRQ_hi) << 8) | IRQ_lo;
}
