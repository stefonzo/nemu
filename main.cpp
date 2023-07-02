#include "cpu.hpp"
#include <iostream>

class RAM16K{
private:
    uint8_t ram[0xFFFF];
public:
    RAM16K() { for (unsigned i = 0; i < 0xFFFF; i++) ram[i] = 0xA9; };
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
    printf("%.4x   %.2x     %.2x     %.2x      %.2x            %.2x\n", GetPC(), GetA(), GetX(), GetY(), GetFlag(), GetSP());
    printf("Cycles: %llu\n", GetCycles());
    printf("Instructions Exectuted: %llu\n\n", GetInstructions());
}

int main() {
    unsigned window_width = 500;
    unsigned window_height = 500;
    RAM16K ram;
    ram.write(0xFFFC, 0x00); //reset vector sets pc to 0x0200 (after zero page and stack page)
    ram.write(0xFFFD, 0x02);
    //---------------------------
    // Inline assembly testing...
    //---------------------------
    //load zero page with test data
    ram.write(0x0000, 0x0F);
    ram.write(0x0001, 0x08);
    //beginning of program
    ram.write(0x0200, 0xA9); // LDA #
    ram.write(0x0201, 0xFF);
    ram.write(0x0202, 0xA5); // LDA $nn
    ram.write(0x0203, 0x01);
    
    cpu c(ram);
    
    //steping through program with emulator (TODO: add stepping to loop)
    c.Reset();
    c.PrintState();
    c.StepInstruction();
    c.PrintState();
    c.StepInstruction();
    c.PrintState();
    return 0;
}
