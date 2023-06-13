#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>

#define NES6502
// Please check out the following code/resource, been my main reference so far
// rubbermallet.org/fake6502.c
// https://www.pagetable.com/c64ref/6502/

/*
    TODO:
        -put flag code into functions (can I cast x & y registers to input for helper functions?)
        -figure out test to see if page has been crossed in memory access
        -finish instructions
        -redo cycles (for now I think its fine but I need to do more research so I can figure out if it will be suitable for timing purposes)
        -testing
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
    uint16_t pc;
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
    void LDA_ZEROPAGE();            //LDA $nn         $A5      3        2
    void NOP();                     //NOP             $EA      2        1
    void PHA();                     //PHA             $48      3        1
    void PHP();                     //PHP             $08      3        1
    
public:
    mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t));
    void Reset6502();
    void NMI();
    uint64_t getCycles() const {return cycles_count;}
};
