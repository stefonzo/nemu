#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>

#define NES6502
// Please check out the following code/resource, been my main references so far
// https://github.com/gianlucag/mos6502 (initial c++ program I saw online)
// rubbermallet.org/fake6502.c (was sourced by the above programmer and was the first program I tried to "pull apart")
// https://www.pagetable.com/c64ref/6502/ (instruction & addressing info)
// https://www.pagetable.com/?p=410 (NMI, IRQ, RESET info)
// https://chat.openai.com/share/bca77b5d-b2c1-47c9-bcc4-1af5962ec19c (not always correct but saves me time googling stuff)
/*
    TODO:
        -thoroughly comment code (this program is intended for educational purposes)
        -put stack operations into private methods
        -add getter methods for 6502 registers
        -fill out opcode table
        -implement RAM datatype
            -needs to be able to load a romfile
        -testing (https://www.nesdev.org/wiki/Visual6502wiki/6502TestPrograms)
            -find suitable test rom
        -implement BCD arithmetic (I don't really want to do that...)
 */
//RESET address
#define RESET_LOW 0xFFFC
#define RESET_HIGH 0xFFFD
//IRQ interrupt address
#define IRQ_LOW 0xFFFE
#define IRQ_HIGH 0xFFFF
//NMI interrupt address
#define NMI_LOW 0xFFFA
#define NMI_HIGH 0xFFFB
//stack is page 1
#define SB 0x01FF // (SB = stack base)
//Flag masks
#define N 0x80 // negative
#define V 0x40 // overflow
#define B 0x10 // break
#define D 0x08 // decimal mode
#define I 0x04 // interrupt
#define Z 0x02 // zero
#define C 0x01 // carry

class mos6502 {
private:
    //state variables for 6502 cpu registers
    uint16_t pc, address; //address is helper variable
    uint8_t a, x, y, sp, flag;
    uint64_t cycles_count, executed_instructions;
//    uint8_t (*read)(uint16_t);
//    void (*write)(uint16_t, uint8_t);
    virtual uint8_t read(uint16_t) { =0; };
    virtual void write(uint16_t, uint8_t) { = 0; };
    typedef void (mos6502::*InstructionMethod)(); //function pointer for jump table
    typedef void (mos6502::*AddressMethod)();
    struct Instruction { //idea from Gian's program
        AddressMethod address_mode;
        InstructionMethod instruction;
        uint8_t cycles;
    };
    std::unordered_map<uint8_t, Instruction> opcodeTable;
    void InitializeOpcodeTable();
    //flag helper functions
    void ZeroCheck(uint16_t value);
    void CarryCheck(uint16_t value);
    void OverflowCheck(uint16_t value);
    void SignCheck(uint16_t value);
    //address helper functions (need to figure out which mode has wrap around bug)
    void ImpliedAddress() {};
    void AccumulatorAddres() {};
    void ImmediateAddress();
    void AbsoluteAddress();
    void AbsoluteIndirectAddress();
    void ZeroPageAddress();
    void AbsoluteXAddressConstant();
    void AbsoluteYAddressConstant();
    void AbsoluteXAddressVariable();
    void AbsoluteYAddressVariable();
    void ZeroPageXAddress();
    void ZeroPageYAddress();
    void ZeroPageXIndirectAddress();
    void ZeroPageYIndirectAddressConstant();
    void ZeroPageYIndirectAddressVariable();
    void RelativeAddress();
    
    uint16_t ReadDoubleWord(); //might get rid of this function...
    //instruction functions
    void ADC();
    void AND();
    void ASL();
    void ASL_ACC(); //accumulator for operand instead of memory
    void BCC();
    void BCS();
    void BEQ();
    void BIT();
    void BMI();
    void BNE();
    void BPL();
    void BRK();
    void BVC();
    void BVS();
    void CLC();
    void CLD();
    void CLI();
    void CLV();
    void CMP();
    void CPX();
    void CPY();
    void DEC();
    void DEX();
    void DEY();
    void EOR();
    void INC();
    void INX();
    void INY();
    void JMP();
    void JSR();
    void LDA();
    void LDX();
    void LDY();
    void LSR();
    void LSR_ACC();
    void NOP();
    void ORA();
    void PHA();
    void PHP();
    void PLA();
    void PLP();
    void ROL();
    void ROL_ACC();
    void ROR();
    void ROR_ACC();
    void RTI();
    void RTS();
    void SBC();
    void SEC();
    void SED();
    void SEI();
    void STA();
    void STX();
    void STY();
    void TAX();
    void TAY();
    void TSX();
    void TXA();
    void TXS();
    void TYA();
    //fetch & execute helper functions
    uint8_t FetchInstruction();
public:
    mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t));
    void Reset();
    void NMI();
    void IRQ();
    void StepInstruction();
    uint64_t getCycles() const {return cycles_count;}
};
