#pragma once
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <unordered_map>

#define NES6502

//define constants for status flag?
//code is inspired froma github project that I need to properly source (identical constructor)
//some code for class comes from chatgpt, specifically

//Flag masks
#define N 0x80 //negative
#define V 0x40 //overflow
#define B 0x10 //break
#define D 0x8 //decimal mode
#define I 0x4 //interrupt
#define Z 0x2 //zero
#define C 0x1 //carry

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
    void CLC();                     //CLC             $18      2        1
    void CLD();                     //CLD             $D8      2        1
    void CLI();                     //CLI             $58      2        1
    void CLV();                     //CLV             $B8      2        1
    void DEX();                     //DEX             $CA      2        1
    void DEY();                     //DEY             $88      2        1
    void JMP_ABSOLUTE();            //JMP $nnnn       $4C      3        3
    void JMP_ABSOLUTE_INDIRECT();   //JMP ($nnnn)     $6C      5        3
    void LDA_IMMEDIATE();           //LDA #$nn        $A9      2        2
    void LDA_ABSOLUTE();            //LDA $nnnn       $AD      4        3
    void LDA_ZEROPAGE();            //LDA $nn         $A5      3        2
public:
    mos6502(uint8_t (*Read)(uint16_t), void (*Write)(uint16_t, uint8_t));
    void Reset6502();
    void NMI();
    uint64_t getCycles() const {return cycles_count;}
};
