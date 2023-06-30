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

int main() {
    unsigned window_width = 500;
    unsigned window_height = 500;
    RAM16K ram;
    return 0;
}
