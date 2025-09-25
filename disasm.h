//////////////////////////////////////////////////////////////////////////////
/// @file disasm.h
/// @brief Header for disasm.c, 6502 disassembler used with emu6502
/// @copyright 2025 William R Cooke
/////////////////////////////////////////////////////////////////////////////
#ifndef _DISASM_H_
#define _DISASM_H_
#include <stdio.h>
#include <stdint.h>

uint16_t disasm_byte(uint8_t* mem, uint16_t pc);

int disasm(uint8_t* mem, uint16_t start, uint16_t stop);

#endif // _DISASM_H_
