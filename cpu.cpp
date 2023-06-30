#include "cpu.hpp"
//flag helper functions
void mos6502::ZeroCheck(uint16_t value) {
    if (value & 0x00FF) {
        flag &= ~(Z);
    } else {
        flag |= Z;
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
void mos6502::RelativeAddress() {
    int8_t offset = read(pc++);
    uint16_t startpage = address & 0xFF00;
    pc += offset;
    if ((pc & 0xFF00) != startpage) cycles_count++;
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
    uint16_t test = a - x;
    ZeroCheck((uint16_t)test);
    SignCheck((uint16_t)test);
    if ( m <= a)
        flag |= C;
    else
        flag &= ~(C);
}
void mos6502::CPY() {
    uint8_t m = read(address);
    uint16_t test = a - y;
    ZeroCheck((uint16_t)test);
    SignCheck((uint16_t)test);
    if ( m <= a)
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
    //ADC instructions
    
}
mos6502::mos6502(std::function<uint8_t(uint16_t)> read_func, std::function<void(uint16_t, uint8_t)> write_func) {
    read = read_func;
    write = write_func;
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
