#pragma once
#include "cpu.hpp"

/*
    PPU registers are mirrored every 8 bytes from $2008 $3FFF
*/

class ppu {
    private:
        // PPU registers        Address    Bits
        //                      ----------------------
        uint8_t PPUCTRL; //     $2000      VPHB SINN
        uint8_t PPUMASK; //     $2001      BGRs bMmG
        uint8_t PPUSTATUS; //   $2002      VSO- ----
        uint8_t OAMADDR; //     $2003      aaaa aaaa
        uint8_t OAMDATA; //     $2004      dddd dddd
        uint8_t PPUSCROLL; //   $2005      xxxx xxxx
        uint8_t PPUADDR; //     $2006      aaaa aaaa
        uint8_t PPUDATA; //     $2007      dddd dddd
        uint8_t OAMDMA; //      $4014      aaaa aaaa
        
        uint16_t v; //VRAM address
        uint16_t t; // temp VRAM address
        uint8_t x; // fine X scroll
        uint8_t w; // first or second write toggle
        uint8_t PPUBusLatch;
    public:
};