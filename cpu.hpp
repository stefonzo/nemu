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
        -should I put addressing functions into jump table or brute force all 151 instructions?
        -figure out relative addressing mode for branch instructions
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
    typedef void (mos6502::*InstrMethod)(); //function pointer for jump table
    std::unordered_map<uint8_t, InstrMethod> opcodeTable;
    void ZeroCheck(uint16_t value);
    void CarryCheck(uint16_t value);
    void OverflowCheck(uint16_t value);
    void SignCheck(uint16_t value);
    void ImmediateAddress();
    void AbsoluteAddress();
    void ZeroPageAddress();
    void AbsoluteXAddress();
    void AbsoluteYAddress();
    void ZeroPageXAddress();
    void ZeroPageYAddress();
    void ZeroPageXIndirectAddress();
    void ZeroPageYIndirectAddress(bool addExtraCycle);
                                    //Assembly:       Opcode   Cycles   Bytes
    void InitializeOpcodeTable();   //-----------------------------------------
    void ADC_IMMEDIATE();           //ADC #$nn        $69      2        2
    void ADC_ABSOLUTE();            //ADC $nnnn       $6D      4        3
    void ADC_ZEROPAGE();            //ADC $nn         $65      2        2
    void ADC_X_ABSOLUTE();          //ADC $nnnn, x    $7D      4+p      3
    void ADC_Y_ABSOLUTE();          //ADC $nnnn, y    $79      4+p      3
    void ADC_X_ZEROPAGE();          //ADC $nn, x      $75      4        2
    void ADC_X_ZEROPAGE_INDIRECT(); //ADC ($nn, x)    $61      6        2
    void ADC_Y_ZEROPAGE_INDIRECT(); //ADC ($nn), y    $71      5+p      2
    void BCC();                     //BCC $nnnn       $90      2+t+p    2
    void BCS();                     //BCS $nnnn       $B0      2+t+p    2
    void BEQ();                     //BEQ $nnnn       $F0      2+t+p    2
    void CLC();                     //CLC             $18      2        1
    void CLD();                     //CLD             $D8      2        1
    void CLI();                     //CLI             $58      2        1
    void CLV();                     //CLV             $B8      2        1
    void DEX();                     //DEX             $CA      2        1
    void DEY();                     //DEY             $88      2        1
    void INX();                     //INX             $E8      2        1
    void INY();                     //INY             $C8      2        1
    void JMP_ABSOLUTE();            //JMP $nnnn       $4C      3        3
    void JMP_ABSOLUTE_INDIRECT();   //JMP ($nnnn)     $6C      5        3
    void LDA_IMMEDIATE();           //LDA #$nn        $A9      2        2
    void LDA_ABSOLUTE();            //LDA $nnnn       $AD      4        3
    void LDA_X_ABSOLUTE();          //LDA $nnnn, x    $BD      4+p      3
    void LDA_Y_ABSOLUTE();          //LDA $nnnn, y    $B9      4+p      3
    void LDA_ZEROPAGE();            //LDA $nn         $A5      3        2
    void LDA_X_ZEROPAGE();          //LDA $nn, x      $B5      4        2
    void NOP();                     //NOP             $EA      2        1
    void PHA();                     //PHA             $48      3        1
    void PHP();                     //PHP             $08      3        1
    void PLA();                     //PLA             $68      4        1
    void PLP();                     //PLP             $28      4        1
    void RTS();                     //RTS             $60      6        1
    void SEC();                     //SEC             $38      2        1
    void SED();                     //SED             $F8      2        1
    void SEI();                     //SEI             $78      2        1
    void TAX();                     //TAX             $AA      2        1
    void TAY();                     //TAY             $A8      2        1
    void TSX();                     //TSX             $BA      2        1
    void TXA();                     //TXA             $8A      2        1
    void TXS();                     //TXS             $9A      2        1
    void TYA();                     //TYA             $98      2        1
public:
    mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t));
    void Reset();
    void NMI();
    void IRQ();
    uint64_t getCycles() const {return cycles_count;}
};
