#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>

#define NES6502
// Please check out the following code/resource, been my main reference so far
// https://github.com/gianlucag/mos6502
// rubbermallet.org/fake6502.c
// https://www.pagetable.com/c64ref/6502/

/*
    TODO:
        -figure out relative addressing mode for branch instructions
        -implement interrupt code
        -fill out opcode table
        -figure out test to see if page has been crossed in memory access (used test from rubbermallet, need to convince myself it works)
        -finish instructions
        -testing (https://www.nesdev.org/wiki/Visual6502wiki/6502TestPrograms)
 */

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
    uint8_t (*read)(uint16_t);
    void (*write)(uint16_t, uint8_t);
    typedef void (mos6502::*InstructionMethod)(); //function pointer for jump table
    typedef void (mos6502::*AddressMethod)();
    struct Instruction { //idea from Gian's program
        AddressMethod address_mode;
        InstructionMethod instruction;
        uint8_t cycles;
    };
    Instruction opcodeTableInstruction;
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
    void ZeroPageAddress();
    void AbsoluteXAddress();
    void AbsoluteYAddress();
    void ZeroPageXAddress();
    void ZeroPageYAddress();
    void ZeroPageXIndirectAddress();
    void ZeroPageYIndirectAddress(bool addExtraCycle);
    void RelativeAddress();
    //instruction functions
    void ADC();
    void AND();
    void ASL();
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
    void NOP();
    void ORA();
    void PHA();
    void PHP();
    void PLA();
    void PLP();
    void ROL();
    void ROR();
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
