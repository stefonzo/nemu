#include "cpu.hpp"
#include <iostream>

class RAM16K{
private:
    uint8_t ram[0xFFFF];
public:
    RAM16K() { for (unsigned i = 0; i < 0xFFFF; i++) ram[i] = 0x00; };
    void write(uint16_t address, uint8_t data) { ram[address] = data; };
    uint8_t read(uint16_t address) { return ram[address]; };
    void LoadRom(std::string path) {};
};

class cpu : public mos6502 {
private:
    RAM16K& ram;
    void write(uint16_t address, uint8_t value) {
        ram.write(address, value);
    }
    uint8_t read(uint16_t address) {
        return ram.read(address);
    }
public:
    cpu(RAM16K& ram) : ram(ram) {}
    void PrintState();
};

void cpu::PrintState() {
    printf("PC     A      X      Y       STATUS        SP       \n");
    printf("----------------------------------------------------\n");
    printf("%.4x   %.2x     %.2x     %.2x      %.2x            %.2x\n\n", GetPC(), GetA(), GetX(), GetY(), GetFlag(), GetSP());
    printf("Cycles: %llu\n", GetCycles());
    printf("Instructions Executed: %llu\n\n", GetInstructions());
}

int main() {
    RAM16K ram;
    ram.write(0xFFFC, 0x00); //reset vector sets pc to 0x0600
    ram.write(0xFFFD, 0x06);
    //---------------------------
    // Inline assembly testing...
    //---------------------------
    
    ram.write(0x0600, 0x18); //CLC
    ram.write(0x0601, 0xA9); //LDA #03
    ram.write(0x0602, 0x08);
    ram.write(0x0603, 0x69); //ADC #03
    ram.write(0x0604, 0x08);
    ram.write(0x0605, 0xE8); //INX
    ram.write(0x0606, 0xE0); //CPX #08
    ram.write(0x0607, 0x07);
    ram.write(0x0608, 0xD0); //BNE
    ram.write(0x0609, 0xF9);
    ram.write(0x060A, 0x8D); //STA $3000
    ram.write(0x060B, 0x00);
    ram.write(0x060C, 0x30);
    
    cpu c(ram);
    
    //steping through program with emulator (TODO: add stepping to loop)
    c.Reset();
    c.PrintState();
    
    uint64_t expected_instructions = 31;
    while (c.GetInstructions() < expected_instructions) {
        c.StepInstruction();
        c.PrintState();
        printf("Ram at pc: %x\n\n", ram.read(c.GetPC()));
    }
    printf("Ram at $3000: %x\n", ram.read(0x3000));
    return 0;
}
