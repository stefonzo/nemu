#include "cpu.hpp"
#include <iostream>
#include <functional>

class RAM16K{
private:
    uint8_t ram[0xFFFF];
public:
    RAM16K() { for (unsigned i = 0; i < 0xFFFF; i++) ram[i] = 0x00; };
    void write(uint16_t address, uint8_t data) { ram[address] = data; };
    uint8_t read(uint16_t address) { return ram[address]; };
    void LoadRom(std::string path) {};
};

int main() {
    unsigned window_width = 500;
    unsigned window_height = 500;
    RAM16K ram;
    //ram function pointers to pass to mos6502 constructor
//    uint8_t (RAM16K::*read)(uint16_t) = &RAM16K::read;
//    void (RAM16K::*write)(uint16_t, uint8_t) = &RAM16K::write;
    auto read = std::bind(&RAM16K::read, &ram, std::placeholders::_1);
    auto write = std::bind(&RAM16K::write, &ram, std::placeholders::_1, std::placeholders::_2);
    //mos6502 cpu(ram.read, ram.write);
    //initialize SDL and setup window
    return 0;
}
