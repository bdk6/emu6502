//////////////////////////////////////////////////////////////////////////////
/// @file emu6502.c
/// @brief Emulator for 6502 processor
/// @copyright 2025 William R Cooke
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <stdint.h>

#include "disasm.h"
#include "hex_loader.h"


// Register definitions
uint16_t PC;
uint8_t A;
uint8_t X;
uint8_t Y;
uint8_t S;
uint8_t P;

// P register flag definitions
 enum flag
{
  FL_N = (1<<7),        // Negative 
  FL_V = (1<<6),        // Overflow
  FL_UNUSED = (1<<5),   // Unused
  FL_B = (1<<4),        // Break
  FL_D = (1<<3),        // Decimal
  FL_I = (1<<2),        // Interrupt
  FL_Z = (1<<1),        // Zero
  FL_C = (1<<0),        // Carry
}  flags;
#define FL_UNUSED_MASK = (1<<5 || 1<<4)

// We need some memory -- 64K
uint8_t MEM[65536];

// Type for all the opcode function definitions
typedef uint8_t (*op_fn)(void);

// Forward Declarations
uint8_t op_illegal(void);
uint8_t op_adc(void);
uint8_t op_and(void);
uint8_t op_asl(void);
uint8_t op_bit(void);
uint8_t op_branch(void);
uint8_t op_brk(void);
uint8_t op_clc(void);
uint8_t op_cld(void);
uint8_t op_cli(void);
uint8_t op_clv(void);
uint8_t op_cmp(void);
uint8_t op_cpx(void);
uint8_t op_cpy(void);
uint8_t op_dec(void);
uint8_t op_dex(void);
uint8_t op_dey(void);
uint8_t op_eor(void);
uint8_t op_inc(void);
uint8_t op_inx(void);
uint8_t op_iny(void);
uint8_t op_jmp(void);
uint8_t op_jmpi(void);
uint8_t op_jsr(void);
uint8_t op_lda(void);
uint8_t op_ldx(void);
uint8_t op_ldy(void);
uint8_t op_lsr(void);
uint8_t op_nop(void);
uint8_t op_ora(void);
uint8_t op_pha(void);
uint8_t op_php(void);
uint8_t op_pla(void);
uint8_t op_plp(void);
uint8_t op_rol(void);
uint8_t op_ror(void);
uint8_t op_rti(void);
uint8_t op_rts(void);
uint8_t op_sbc(void);
uint8_t op_sec(void);
uint8_t op_sed(void);
uint8_t op_sei(void);
uint8_t op_sta(void);
uint8_t op_stx(void);
uint8_t op_sty(void);
uint8_t op_tax(void);
uint8_t op_tay(void);
uint8_t op_tsx(void);
uint8_t op_txa(void);
uint8_t op_txs(void);
uint8_t op_tya(void);

//////////////////////////////////////////////////////////////////////////////
/// @variable operations
/// @brief Holds fn ptrs for all possible opcodes.  Init to base NMOS 6502.
///        Can be added to for follow-on models.
//////////////////////////////////////////////////////////////////////////////
op_fn operations[256] =
  {
    op_brk,              // 00 brk
    op_ora,              // 01 ora (xx,X)
    op_illegal,          // 02
    op_illegal,          // 03
    op_illegal,          // 04
    op_ora,              // 05 ora xx
    op_asl,              // 06 asl xx
    op_illegal,          // 07 ill
    op_php,              // 08 php
    op_ora,              // 09 ora #xx
    op_asl,              // 0a asl A
    op_illegal,          // 0b ill
    op_illegal,          // 0c ill
    op_ora,              // 0d ora xxxx
    op_asl,              // 0e asl xxxx
    op_illegal,          // 0f ill
    op_branch,           // 10 BPL
    op_ora,              // 11 ora (xx),Y
    op_illegal,          // 12 ill
    op_illegal,          // 13 ill
    op_illegal,          // 14 ill
    op_ora,              // 15 ora xx,X
    op_asl,              // 16 asl xx,X
    op_illegal,          // 17 ill
    op_clc,              // 18 CLC
    op_ora,              // 19 ora xxxx,Y
    op_illegal,          // 1a ill
    op_illegal,          // 1b ill
    op_illegal,          // 1c ill
    op_ora,              // 1d ora xxxx,X
    op_asl,              // 1e asl xxxx,X
    op_illegal,          // 1f ill
    op_jsr,              // 20 jsr xxxx
    op_and,              // 21 and (xx,X)
    op_illegal,          // 22 ill
    op_illegal,          // 23 ill
    op_bit,              // 24 bit xx
    op_and,              // 25 and xx
    op_rol,              // 26 rol xx
    op_illegal,          // 27 ill
    op_plp,              // 28 PLP
    op_and,              // 29 and #xx
    op_rol,              // 2a rol A
    op_illegal,          // 2b
    op_bit,              // 2c bit xxxx
    op_and,              // 2d and xxxx
    op_rol,              // 2e rol xxxx
    op_illegal,          // 2f
    op_branch,           // 30  BMI
    op_and,              // 31 and (xx),Y
    op_illegal,          // 32
    op_illegal,          // 33
    op_illegal,          // 34
    op_and,              // 35 and xx,X
    op_rol,              // 36 rol xx,X
    op_illegal,          // 37
    op_sec,              // 38 SEC
    op_and,              // 39 and xxxx,Y
    op_illegal,          // 3a
    op_illegal,          // 3b
    op_illegal,          // 3c
    op_and,              // 3d and xxxx,X
    op_rol,              // 3e rol xxxx,X
    op_illegal,          // 3f
    op_rti,              // 40 rti
    op_eor,              // 41  EOR (xx,X)
    op_illegal,          // 42
    op_illegal,          // 43
    op_illegal,          // 44
    op_eor,              // 45 eor xx
    op_lsr,              // 46 lsr xx
    op_illegal,          // 47
    op_pha,              // 48 pha
    op_eor,              // 49 EOR #xx
    op_lsr,              // 4a lsr a
    op_illegal,          // 4b
    op_jmp,              // 4c jmp xxxx
    op_eor,              // 4d EOR xxxx
    op_lsr,              // 4e lsr xxxx
    op_illegal,          // 4f
    op_branch,           // 50 BVC
    op_eor,              // 51 EOR (xx),Y
    op_illegal,          // 52
    op_illegal,          // 53
    op_illegal,          // 54
    op_eor,              // 55 EOR xx,X
    op_lsr,              // 56 lsr xx,X
    op_illegal,          // 57
    op_cli,              // 58 CLI
    op_eor,              // 59 EOR xxxx,Y
    op_illegal,          // 5a
    op_illegal,          // 5b
    op_illegal,          // 5c
    op_eor,              // 5d EOR xxxx,X
    op_lsr,              // 5e lsr xxxx,X
    op_illegal,          // 5f
    op_rts,              // 60 rts
    op_adc,              // 61 adc (xx,X)
    op_illegal,          // 62
    op_illegal,          // 63
    op_illegal,          // 64
    op_adc,              // 65 adc xx
    op_ror,              // 66 ror xx
    op_illegal,          // 67
    op_pla,              // 68 PLA
    op_adc,              // 69 adc #xx
    op_ror,              // 6a ror A
    op_illegal,          // 6b
    op_jmpi,             // 6c jmp (xxxx)
    op_adc,              // 6d adc xxxx
    op_ror,              // 6e ror xxxx
    op_illegal,          // 6f
    op_branch,           // 70 BVS
    op_adc,              // 71 adc (xx),Y
    op_illegal,          // 72
    op_illegal,          // 73
    op_illegal,          // 74
    op_adc,              // 75 adc xx,X
    op_ror,              // 76 ror xx,X
    op_illegal,          // 77
    op_sei,              // 78 SEI
    op_adc,              // 79 adc xxxx,Y
    op_illegal,          // 7a
    op_illegal,          // 7b
    op_illegal,          // 7c
    op_adc,              // 7d adc xxxx,X
    op_ror,              // 7e ror xxxx,X
    op_illegal,          // 7f
    op_illegal,          // 80
    op_sta,              // 81 sta (xx,X)
    op_illegal,          // 82
    op_illegal,          // 83
    op_sty,              // 84 sty xx
    op_sta,              // 85 sta xx
    op_stx,              // 86 stx xx
    op_illegal,          // 87
    op_dey,              // 88 dey
    op_illegal,          // 89
    op_txa,              // 8a txa
    op_illegal,          // 8b
    op_sty,              // 8c sty xxxx
    op_sta,              // 8d sta xxxx
    op_stx,              // 8e stx xxxx
    op_illegal,          // 8f
    op_branch,           // 90 BCC
    op_sta,              // 91 sta (xx),Y
    op_illegal,          // 92
    op_illegal,          // 93
    op_sty,              // 94 sty xx,X
    op_sta,              // 95 sta xx,X
    op_stx,              // 96 stx xx,Y
    op_illegal,          // 97
    op_tya,              // 98 tya
    op_sta,              // 99 sta xxxx,Y
    op_txs,              // 9a txs
    op_illegal,          // 9b
    op_illegal,          // 9c
    op_sta,              // 9d sta xxxx,X
    op_illegal,          // 9e
    op_illegal,          // 9f
    op_ldy,              // a0 ldy #xx
    op_lda,              // a1 lda (xx),Y
    op_ldx,              // a2 ldx #xx
    op_illegal,          // a3
    op_ldy,              // a4 ldy xx
    op_lda,              // a5 lda xx
    op_ldx,              // a6 ldx xx
    op_illegal,          // a7
    op_tay,              // a8 tay
    op_lda,              // a9 lda #xx
    op_tax,              // aa tax
    op_illegal,          // ab
    op_ldy,              // ac ldy xxxx
    op_lda,              // ad lda xxxx
    op_ldx,              // ae ldx xxxx
    op_illegal,          // af
    op_branch,           // b0 BCS
    op_lda,              // b1 lda (xx),Y
    op_illegal,          // b2
    op_illegal,          // b3
    op_ldy,              // b4 ldy xx,X
    op_lda,              // b5 lda xx,X
    op_ldx,              // b6 ldx xx,Y
    op_illegal,          // b7
    op_clv,              // b8 CLV
    op_lda,              // b9 lda xxxx,Y
    op_tsx,              // ba tsx
    op_illegal,          // bb
    op_ldy,              // bc ldy xxxx,X
    op_lda,              // bd lda xxxx,X
    op_ldx,              // be ldx xxxx,Y
    op_illegal,          // bf
    op_cpy,              // c0 cpy #xx
    op_cmp,              // c1 cmp (xx,X)
    op_illegal,          // c2
    op_illegal,          // c3
    op_cpy,              // c4 cpy xx
    op_cmp,              // c5 cmp xx
    op_dec,              // c6 dec xx
    op_illegal,          // c7
    op_iny,              // c8 iny
    op_cmp,              // c9 cmp #xx
    op_dex,              // ca dex
    op_illegal,          // cb
    op_cpy,              // cc  cpy xxxx
    op_cmp,              // cd cmp xxxx
    op_dec,              // ce dec xxxx
    op_illegal,          // cf
    op_branch,           // d0  BNE
    op_cmp,              // d1 cmp (xx),Y
    op_illegal,          // d2
    op_illegal,          // d3
    op_illegal,          // d4
    op_cmp,              // d5 cmp xx,X
    op_dec,              // d6 dec xx,X
    op_illegal,          // d7
    op_cld,              // d8 CLD
    op_cmp,              // d9 cmp xxxx,Y
    op_illegal,          // da
    op_illegal,          // db
    op_illegal,          // dc
    op_cmp,              // dd cmp xxxx,X
    op_dec,              // de dec xxxx,X
    op_illegal,          // df
    op_cpx,              // e0 cpx #xx
    op_sbc,              // e1 sbc (xx,X)
    op_illegal,          // e2
    op_illegal,          // e3
    op_cpx,              // e4 cpx  xx
    op_sbc,              // e5 sbc xx
    op_inc,              // e6 inc xx
    op_illegal,          // e7
    op_inx,              // e8 inx
    op_sbc,              // e9 sbc #xx
    op_illegal,          // ea
    op_illegal,          // eb
    op_cpx,              // ec cpx xxxx
    op_sbc,              // ed sbc xxxx
    op_inc,              // ee inc xxxx
    op_illegal,          // ef
    op_branch,           // f0  BEQ
    op_sbc,              // f1 sbc (xx),Y
    op_illegal,          // f2
    op_illegal,          // f3
    op_illegal,          // f4
    op_sbc,              // f5 sbc xx,X
    op_inc,              // f6 inc xx,X
    op_illegal,          // f7
    op_sed,              // f8 SED
    op_sbc,              // f9 sbc xxxx,Y
    op_illegal,          // fa
    op_illegal,          // fb
    op_illegal,          // fc
    op_sbc,              // fd sbc xxxx,X
    op_inc,              // fe inc xxxx,X
    op_illegal           // ff
  };

//////////////////////////////////////////////////////////////////////////////
/// @fn read_word
/// @brief Reads a little-endian word from memory.
/// @param[in] addr  The address to read from
/// @return Word from addr and addr+1
//////////////////////////////////////////////////////////////////////////////
uint16_t read_word(uint16_t addr)
{
  uint16_t rtn;

  rtn = MEM[addr] + (MEM[addr +1] << 8);
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn write_word
/// @brief Writes a little-endian word to memory
/// @param[in] addr  The address to write to.
/// @param[in] word  The word to write to addr and addr+1.
//////////////////////////////////////////////////////////////////////////////
void write_word(uint16_t addr, uint16_t word)
{
  MEM[addr] = (uint8_t) (word & 0xff);
  MEM[addr + 1] = (uint8_t) ((word >> 8) & 0xff);
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_xxxx   Functions to process each instruction.
/// @brief A number of very similar functions to process each instruction type.
///   Uses global registers and memory to read and process instructions.
/// @return Number of cycles used by the instruction.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_illegal(void)
{
  printf("ILLEGAL inst %02x at %04x\n", MEM[PC], PC);
  PC++;
  return 1;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_adc
/// @brief Simulate adc instruction.
/// Imm: 69 ZP: 65 ZPx: 75 ABS: 6d ABSx:7d ABSy: 79 INx 61 INDy 71  -- NVZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_adc(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;   // second operand (A is first)
  uint16_t address = 0;
  switch(MEM[PC++])
    {
    case 0x69:   // adc #xx
      address = PC++;
      rtn = 2;
      break;
    case 0x65:   // adc xx
      address = MEM[PC++];
      rtn = 3;
      break;
    case 0x75:   // adc xx,X
      address = MEM[PC++];
      address += X;
      address &= 0xff;
      rtn = 4;
      break;
    case 0x6d:   // adc xxxx
      address = MEM[PC++];
      address |= (MEM[PC++] << 8);
      rtn = 4;
      break;
    case 0x7d:   // adc xxxx,X
      address = MEM[PC++];
      address |= (MEM[PC++] << 8);
      if( (address + X & 0xff00) != (address & 0xff00) )
      {
              rtn = 5;
      }
      else
      {
              rtn = 4;
      }
      address += X;
      
      break;
    case 0x79:   // adc xxxx,Y
      address = MEM[PC++];
      address |= (MEM[PC++] << 8);
      rtn = 4;
      if( (address + Y && 0xff00) != (address & 0xff00) )
      {
              rtn++;
      }
      address += Y;
      break;
    case 0x61:   // adc (xx,X)
      address = MEM[PC++];
      address = (address + X) & 0xff;
      address = read_word(address);
      rtn = 6;
      break;
    case 0x71:   // adc (xx),Y
      address = MEM[PC++];
      address = read_word(address) + Y;
      rtn = 5; // TODO check page boundary
    default:
      break;
    }
  // clear used flags
  P &= ~(FL_C | FL_V | FL_V | FL_Z);
  b = MEM[address];
  if(P & FL_D) // decimal mode
  {
          // TODO: decimal mode
  }
  else
  {
          b += A;
  }
  if(P & FL_C)
    {
      b++;
    }
  // set flags
  if(b > 255)
    {
      P |= FL_C;
    }
  // TODO verify this
  if(b > 127)
    {
      P |= FL_N;
    }
  if((b & 0xff) == 0)
    {
      P |= FL_Z;
    }
  // TODO V flag

  A = b & 0xff;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_and
/// @brief Simulate and instruction.
/// Imm: 29 ZP: 25 ZPx: 35 ABS: 2d ABSx:3d ABSy: 39 INx 21 INDy 31  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_and(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  uint8_t b = 0;
  switch(MEM[PC++])
    {
    case 0x29:   // and #xx
            b = MEM[PC++];
            rtn = 2;
	break;
    case 0x25:   // and xx
            address = MEM[PC++];
            b = MEM[address];
            rtn = 3;
      break;
    case 0x35:   // and xx,X
            address = MEM[PC++];
            address = address + X & 0xff;
            b = MEM[address];
            rtn = 4;
      break;
    case 0x2d:   // and xxxx
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
            b = MEM[address];
            rtn = 4;
      break;
    case 0x3d:   // and xxxx,X
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
            rtn = 4;
            if( (address + X & 0xff00) != (address & 0xff00) )
            {
                    rtn ++;
            }
            address += X;
            b = MEM[address];
      break;
    case 0x39:   // and xxxx,Y
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
            rtn = 4;
            if( (address + Y & 0xff00) != (address & 0xff00) )
            {
                    rtn++;
            }
            address += Y;
            b = MEM[address];
      break;
    case 0x21:   // and (xx,X)
      break;
    case 0x31:   // and (xx),Y
      break;
    default:
      break;
    }
  A &= b;
  // TODO flags
      
      
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_asl
/// @brief Simulate asl instruction.
/// A: 0a Zp: 06 ZPx: 16 ABS: 0e ABSx: 1e    -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_asl(void)
{
  uint8_t rtn = 0;
  uint8_t op = MEM[PC++];
  uint8_t b = 0;
  uint16_t addr = 0;
  P &= ~(FL_N | FL_Z | FL_C);  // clear affected flags
  
  if(op == 0x0a)    // asl A, handle as a special case
  {
            if(A & 0x80)
            {
                    P |= FL_C;
            }
            A <<= 1;
            if(A == 0)
            {
                    P |= FL_Z;
            }
            if(A & 0x80)
            {
                    P |= FL_N;
            }
            rtn = 2;
  }
  else
  {
    switch(op)
    {
    case 0x06:   // asl xx
       addr = MEM[PC++];
       rtn = 5;
       break;
    case 0x16:   // asl xx,X
       addr = MEM[PC++];
       addr = (addr + X) & 0xff;
       rtn = 6;
       break;
    case 0x0e:   // asl xxxx
       addr = MEM[PC++];
       addr |= MEM[PC++] << 8;
       rtn = 6;
       break;
    case 0x1e:   // asl xxxx,X
      addr = MEM[PC++];
      addr |= MEM[PC++] << 8;
      rtn = 7;
      break;
    default:
      break;
    }
    // do op
    b = MEM[addr];
    MEM[addr] = b;
    // set flags
    if(b & 0x80)
    {
      P |= FL_C;
    }
    b <<= 8;
    if(b & 0x80)
    {
      P |= FL_N;
    }
    if(b == 0)
    {
      P |= FL_Z;
    }
  }  // switch / not A
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_bit
/// @brief Simulate bit instruction.
/// ZP: 24 ABS: 2c  -- NVZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_bit(void)
{
  uint8_t rtn = 0;
  uint8_t b = 0;
  uint16_t addr = 0;

  P &= ~(FL_N | FL_V | FL_Z);  // clear affected flags
  
  switch(MEM[PC++])
    {
    case 0x24:   // bit xx
            addr = MEM[PC++];
            b = MEM[addr];
            rtn = 3;
      break;
    case 0x2c:   // bit xxxx
            addr = MEM[PC++];
            addr |= MEM[PC++] << 8;
            b = MEM[PC];
            rtn = 4;
      break;
    default:
      break;
    }
  b &= A;
  if(b == 0)
  {
          P |= FL_Z;
  }
  if(b & 0x80)
  {
          P |= FL_N;
  }
  if(b & 0x40)
  {
          P |= FL_V;
  }
  
  
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_branch
/// @brief Simulate branch instructions.
/// BPL:10 BMI:30 BVC:50 BVS:70 BCC:90 BCS:b0 BNE:d0 BEQ:f0  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_branch(void)
{
  uint8_t rtn = 2;
  uint8_t op = MEM[PC++];
  uint8_t disp = MEM[PC++];
  
  // have to cast to get negative displacements
  int16_t addend = (int16_t) (int8_t)disp;
  uint16_t new_pc = (uint16_t)(PC + addend);
  uint8_t  crosspage = 0;
  if( (PC & 0xff00) != (new_pc & 0xff00) )
  {
          crosspage = 1;
  }
  
  switch(op)
    {
    case 0x10:   // BPL
      if( (P & FL_N) == 0)
	PC = new_pc;
	break;
	
    case 0x30:   // BMI
      if( (P & FL_N) != 0)
	PC = new_pc;
	break;
	
    case 0x50:   // BVC
      if( (P & FL_V) == 0)
	PC = new_pc;
      break;
      
    case 0x70:   // BVS
      if( (P & FL_V) != 0)
	PC = new_pc;
      break;
      
    case 0x90:   // BCC
      if( (P & FL_C) == 0)
	PC = new_pc;
      break;
      
    case 0xb0:   // BCS
      if( (P & FL_C) != 0)
	PC = new_pc;
      break;
      
    case 0xd0:   // BNE
      if( (P & FL_Z) != 0)
	PC = new_pc;
      break;
      
    case 0xf0:   // BEQ
      if( (P & FL_Z) == 0)
	PC = new_pc;
      break;
    default:
      break;
    }
  if( PC == new_pc)
    {
      rtn++;    // three cycles for taken branch
      rtn += crosspage; // four if crossed page
    }
  
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_brk
/// @brief Simulate brk instruction.
/// BRK:00                                                -- B
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_brk(void)
{
  uint8_t rtn = 2;
  printf("brk instr %2x at %4x\n", MEM[PC], PC);
  PC += 2;  // Single byte instruction but pushes PC+2!
 
  MEM[0x100 | S--] = (uint8_t)(PC >> 8);
  MEM[0x100 | S--] = (uint8_t)(PC & 0xff);
  MEM[0x100 | S--] = (uint8_t)(P | FL_B | FL_I);
  uint16_t address = MEM[0xfffe];
  address |= MEM[0xffff] << 8;
  PC = address;

  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_clc
/// @brief Simulate clc instruction.
/// CLC: 18  -- C
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_clc(void)
{
  uint8_t rtn = 2;
  P &= ~ FL_C;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_cld
/// @brief Simulate cld instruction.
/// CLD: d8  -- D
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_cld(void)
{
  uint8_t rtn = 2;
  P &= ~ FL_D;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_cli
/// @brief Simulate cli instruction.
/// CLI: 58  -- I
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_cli(void)
{
  uint8_t rtn = 2;
  P &= ~ FL_I;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_clv
/// @brief Simulate clv instruction.
/// CLV: 88 -- V
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_clv(void)
{
  uint8_t rtn = 2;
  P &= ~FL_V;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_cmp
/// @brief Simulate cmp instruction.
/// I: c9 ZP: c5 ZPx: d5 ABS: cd ABSx: dd ABSy: d9 Ix: c1 Iy: d1 -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_cmp(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  // clear used flags
  P &= ~(FL_N | FL_Z | FL_C);
  
  switch(MEM[PC++])
    {
    case 0xc9:   // cmp #xx
      address = PC++;
      rtn = 2;
      break;
    case 0xc5:   // cmp xx
      address = MEM[PC++];
      rtn = 3;
      break;
    case 0xd5:   // cmp xx,X
            address = (MEM[PC++] + X) & 0xff;
            rtn = 4;
      break;
    case 0xcd:   // cmp xxxx
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
            rtn = 4;
      break;
    case 0xdd:   // cmp xxxx,X
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
            address += X;
            rtn = 4;
            // TODO check pg cross
      break;
    case 0xd9:   // cmp xxxx,Y
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
            address += Y;
            rtn = 4;
            // TODO check pg cross
      break;
    case 0xc1:   // cmp (xx,X)
            address = MEM[PC++];
            address = (address + X) & 0xff;
            address = read_word(address);
            rtn = 6;
      break;
    case 0xd1:   // cmp (xx),Y
            address = MEM[PC++];
            address = read_word(address) + Y;
            rtn = 5;
            // TODO check pg cross
      break;
    default:
      break;
    }
  b = MEM[address];
  if(A > b)
  {
          P |= FL_C;
  }
  b = A-b;
  if(b & 0x80)
  {
          P |= FL_N;
  }
  if(b == 0)
  {
          P |= FL_Z;
  }
  
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_cpx
/// @brief Simulate cpx instruction.
/// Imm: e0 ZP: e4 ABC: ec                -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_cpx(void)
{
  uint8_t rtn = 0;
  uint8_t b = 0;
  uint8_t address = 0;

  // clear used flags
  P &= ~(FL_N | FL_Z | FL_C);
  
  switch(MEM[PC++])
    {
    case 0xe0:   // cpx #xx
      break;
    case 0xe4:   // cpx xx
      break;
    case 0xec:   // cpx xxxx
      break;
    default:
      break;
    }

  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_cpy
/// @brief Simulate cpy instruction.
/// Imm: c0 ZP: c4 ABC: cc                      -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_cpy(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
    {
    case 0xc0:   // cpy #xx
      break;
    case 0xc4:   // cpy xx
      break;
    case 0xcc:   // cpy xxxx
      break;
    default:
      break;
    }

  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_dec
/// @brief Simulate dec instruction.
/// ZP: c6 ZPx: d6 ABS: ce ABSx: de                     -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_dec(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  uint16_t b = 0;
  switch(MEM[PC])
    {
    case 0xc6:   // dec xx
      address = PC++;
      break;
    case 0xd6:   // dec xx,X
      break;
    case 0xce:   // dec xxxx
      break;
    case 0xde:   // dec xxxx,X
      break;
    default:
      break;
    }
  b = MEM[address];
  b--;
  // TODO flags
  b &= 0xff;
  MEM[address] = (uint8_t) b;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_dex
/// @brief Simulate dex instruction.
/// IMPL:   -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_dex(void)
{
  X--;
  if(X == 0)
    {
      P |= FL_Z;
    }
  if((X & (1<<7)) != 0)
    {
      P |= FL_N;
    }
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_dey
/// @brief Simulate dey instruction.
/// IMPL:   -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_dey(void)
{
  Y--;
    if(Y == 0)
    {
      P |= FL_Z;
    }
  if((Y & (1<<7)) != 0)
    {
      P |= FL_N;
    }
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_eor
/// @brief Simulate eor instruction.
/// Imm: 49 ZP: 45 ZPx: 55 ABS: 4d ABSx: 5d ABSy:59 INIX: 41 IXIY: 51 -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_eor(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
    {
    case 0x49:   // eor #xx
      break;
    case 0x45:   // eor xx
      break;
    case 0x55:   // eor xx,X
      break;
    case 0x4d:   // eor xxxx
      break;
    case 0x5d:   // eor xxxx,X
      break;
    case 0x59:   // eor xxxx,Y
      break;
    case 0x41:   // eor (xx,X)
      break;
    case 0x51:   // eor (xx),Y
      break;
    default:
      break;
    }
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_inc
/// @brief Simulate inc instruction.
/// ZP: e6 ZPx: f6 ABS: ee ABSx fe  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_inc(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
    {
    case 0xe6:   // inc xx
      break;
    case 0xf6:   // inc xx,X
      break;
    case 0xee:   // inc xxxx
      break;
    case 0xfe:   // inc xxxx,X
      break;
    default:
      break;
    }
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_inx
/// @brief Simulate inx instruction.
/// inx  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_inx(void)
{
  X++;
  if(X == 0)
    {
      P |= FL_Z;
    }
  if((X & (1 << 7)) != 0)
    {
      P |= FL_N;
    }
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_iny
/// @brief Simulate iny instruction.
/// INY  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_iny(void)
{
  Y++;
  if(Y == 0)
    {
      P |= FL_Z;
    }
  if((Y & (1 << 7)) != 0)
    {
      P |= FL_N;
    }
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_jmp
/// @brief Simulate jmp instruction.
/// ABS: 4c  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_jmp(void)
{
  uint8_t rtn = 4;  // TODO:  get real value
  PC++;
  PC = read_word(PC);
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_jmpi
/// @brief Simulate jmp indirect instruction.
/// INDIRECT: 6c  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
// jmp (xxxx) 6c
uint8_t op_jmpi(void)
{
  // Make sure to check last page byte
  uint8_t rtn = 4;   // TODO get real value
  PC = read_word( read_word(PC + 1));
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_jsr
/// @brief Simulate jsr instruction.
/// ABS:   
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_jsr(void)
{
  uint8_t rtn = 6;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_lda
/// @brief Simulate lda instruction.
/// IMM: a9 ZP: a5 ZPx: b5 ABS: ad ABSx: bd ABSy: b9  IX: a1 IY: b1-- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_lda(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0xa9:   // lda #xx
      address = PC++;
      break;
    case 0xa5:   // lda xx
      address = MEM[PC++];
      break;
    case 0xb5:   // lda xx,X
      break;
    case 0xad:   // lda xxxx
      break;
    case 0xbd:   // lda xxxx,X
      break;
    case 0xb9:   // lda xxxx,Y
      break;
    case 0xa1:   // lda (xx,X)
      break;
    case 0xb1:   // lda (xx),Y
      break;
    default:
      break;
    }
  A = MEM[address];
  if(A == 0)
    {
      P |= FL_Z;
    }
  if((A & (1 << 7)) != 0)
    {
      P |= FL_N;
    }
    
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_ldx
/// @brief Simulate ldx instruction.
/// Imm: a2 ZP: a6 ZPy: b6 ABS: ae ABSy: be  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_ldx(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0xa2:   // ldx #xx
      address = PC++;
      break;
    case 0xa6:   // ldx xx
      address = MEM[PC++];
      break;
    case 0xb6:   // ldx xx,Y
      break;
    case 0xae:   // ldx xxxx
      break;
    case 0xbe:   // ldx xxxx,Y
      break;
    default:
      break;
    }
  X = MEM[address];
  // TODO flags
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_ldy
/// @brief Simulate ldy instruction.
/// Imm: a0 ZP: a4 ZPx: b4 ABS: ac ABSx: bc  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_ldy(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0xa0:   // ldy #xx
      address = PC++;
      break;
    case 0xa4:   // ldy xx
      address = MEM[PC++];
      break;
    case 0xb4:   // ldy xx,X
      break;
    case 0xac:   // ldy xxxx
      break;
    case 0xbc:   // ldy xxxx,X
      break;
    default:
      break;
    }
  Y = MEM[address];
  // TODO flags
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_lsr
/// @brief Simulate lsr instruction.
/// A: 4a ZP: 46 ZPx: 56 ABS: 4e ABSx: 5e  -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_lsr(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  // TODO:  A is tricky
  switch(MEM[PC])
    {
    case 0x4a:   // lsr A
      break;
    case 0x46:   // lsr xx
      break;
    case 0x56:   // lsr xx,X
      break;
    case 0x4e:   // lsr xxxx
      break;
    case 0x5e:   // lsr xxxx,X
      break;
    default:
      break;
    }
  return rtn;
}
//////////////////////////////////////////////////////////////////////////////
/// @fn op_nop
/// @brief Simulate nop instruction.
/// IMPLIED:  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_nop(void)
{
  PC++;
  return 2;
}


//////////////////////////////////////////////////////////////////////////////
/// @fn op_ora
/// @brief Simulate ora instruction.
/// Imm: 09 ZP: 05 ZPx: 15 ABS: 0d ABSx: 1d ABSy: 19 IX: 01 IY: 11  -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_ora(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0x09:   // ora #xx
      address = PC++;
      break;
    case 0x05:   // ora xx
      address = MEM[PC++];
      break;
    case 0x15:   // ora xx,X
      break;
    case 0x0d:   // ora xxxx
      break;
    case 0x1d:   // ora xxxx,X
      break;
    case 0x19:   // ora xxxx,Y
      break;
    case 0x01:   // ora (xx,X)
      break;
    case 0x11:   // ora (xx),Y
      break;
    default:
      break;
    }
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_rol
/// @brief Simulate rol instruction.
/// A: 2a ZP: 26 ZPx: 36 ABS: 2e ABSx: 3e  -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_rol(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  // TODO:  A is tricky
  switch(MEM[PC])
    {
    case 0x2a:   // rol A
      break;
    case 0x26:   // rol xx
      break;
    case 0x36:   // rol xx,X
      break;
    case 0x2e:   // rol xxxx
      break;
    case 0x3e:   // rol xxxx,X
      break;
    default:
      break;
    }
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_pha
/// @brief Simulate pha instruction.
/// IMPL: 48  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_pha(void)
{
  MEM[0x100 | S] = A;
  S--;
  return 3;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_php
/// @brief Simulate php instruction.
/// IMPL: 08  
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_php(void)
{
  // TODO careful with B and unused flags
  return 3;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_pla
/// @brief Simulate pla instruction.
/// IMPL: 68                                         
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_pla(void)
{
  A = MEM[0x100 | S];
  S++;
  // TODO flags
  return 4;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_plp
/// @brief Simulate plp  instruction.
/// IMPL: 28
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
// plp -- TODO: flags for all stack inst
uint8_t op_plp(void)
{
  // TODO careful with B and unused flags
  return 4;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_ror
/// @brief Simulate ror instruction.
/// A: 6a ZP: 66 ZPx: 76 ABS: 63 ABSx: 7e  -- NZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_ror(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  // TODO A is tricky
  switch(MEM[PC])
    {
    case 0x6a:   // ror A
      break;
    case 0x66:   // ror xx
      break;
    case 0x76:   // ror xx,X
      break;
    case 0x6e:   // ror xxxx
      break;
    case 0x7e:   // ror xxxx,X
      break;
    default:
      break;
    }
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_rti
/// @brief Simulate rti instruction.
/// IMPL: 40  -- all
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_rti(void)
{
        // pop p, pcl, pch
  return 6;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_rts
/// @brief Simulate rts instruction.
/// IMPL: 60  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_rts(void)
{
  uint16_t address = MEM[0x100 | S];
  S++;
  address |= (MEM[0x100 | S] << 8);
  S++;
  PC = address;
  return 6;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_sbc
/// @brief Simulate sbc instruction.
/// Imm: e9 ZP: e5 ZPx: f5 ABS: ed ABSx:fd ABSy: f9 INx e1 INDy f1  -- NVZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_sbc(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0xe9:   // sbc #xx
      address = PC++;
      break;
    case 0xe5:   // sbc xx
      address = MEM[PC++];
      break;
    case 0xf5:   // sbc xx,X
      break;
    case 0xed:   // sbc xxxx
      break;
    case 0xfd:   // sbc xxxx,X
      break;
    case 0xf9:   // sbc xxxx,Y
      break;
    case 0xe1:   // sbc (xx,X)
      break;
    case 0xf1:   // sbc (xx),Y
      break;
    default:
      break;
    }
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_sec
/// @brief Simulate sec instruction.
/// IMPL: 38  -- C
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_sec(void)
{
  uint8_t rtn = 2;
  P |= FL_C;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_set
/// @brief Simulate adc instruction.
/// IMPL: f8   -- D
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_sed(void)
{
  uint8_t rtn = 2;
  P |= FL_D;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_sei
/// @brief Simulate sei instruction.
/// IMPL: 78  -- I
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
// sei -- I
uint8_t op_sei(void)
{
  uint8_t rtn = 2;
  P |= FL_I;
  PC++;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_sta
/// @brief Simulate sta instruction.
/// ZP: 85 ZPx: 95 ABS: 8d ABSx: 9d ABSy: 99 IX: 81 IY: 91 -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_sta(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0x85:   // sta xx
      address = MEM[PC++];
      break;
    case 0x95:   // sta xx,X
      break;
    case 0x8d:   // sta xxxx
      break;
    case 0x9d:   // sta xxxx,X
      break;
    case 0x99:   // sta xxxx,Y
      break;
    case 0x81:   // sta (xx,X)
      break;
    case 0x91:   // sta (xx),Y
      break;
    default:
      break;
    }
  MEM[address] = A;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_stx
/// @brief Simulate stx instruction.
/// ZP: 86 ZPy: 96 ABS: 8e  -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_stx(void)
{
  uint8_t rtn = 4;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0x86:   // stx xx
      address = MEM[PC++];
      rtn = 3;
      break;
    case 0x96:   // stx xx,Y
            address = (MEM[PC++] + Y) & 0xff;
      break;
    case 0x8e:   // stx xxxx
            address = MEM[PC++];
            address += MEM[PC++] << 8;
      break;
    default:
      break;
    }
  MEM[address] = X;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_sty
/// @brief Simulate sty instruction.
/// ZP: 84 ZPx: 94 ABS: 8c   -- NONE
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_sty(void)
{
  uint8_t rtn = 4;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0x84:   // sty xx
      address = MEM[PC++];
      rtn = 3;
      break;
    case 0x94:   // sty xx,X
            address = MEM[PC++] + X & 0xff;
      break;
    case 0x8c:   // sty xxxx
            address = MEM[PC++];
            address |= MEM[PC++] << 8;
      break;
    default:
      break;
    }
  MEM[address] = Y;
  return rtn;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_tax
/// @brief Simulate tax instruction.
/// IMPL:  aa                            -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_tax(void)
{
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_tay
/// @brief Simulate tay instruction.
/// IMPL: a8                              -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_tay(void)
{
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_tsx
/// @brief Simulate tsx instruction.
/// IMPL: ba          -- TODO
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_tsx(void)
{
        X = S;
        // TODO: flags
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_txa
/// @brief Simulate txa instruction.
/// IMPL: 8a                                 -- NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_txa(void)
{
        A = X;
        // TODO flags
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_txs
/// @brief Simulate txs instruction.
/// IMPL:                            -- TODO:  flags
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_txs(void)
{
        S = X;
  return 2;
}

//////////////////////////////////////////////////////////////////////////////
/// @fn op_tya
/// @brief Simulate tya instruction.
/// IMPL: 98                          -- TODO:  NZ
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
uint8_t op_tya(void)
{
        A = Y;
        // TODO: flags
  return 2;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/// @fn op_init
/// @brief Prepare the virtual machine to run.
//////////////////////////////////////////////////////////////////////////////
void init(void)
{
  for(int i = 0; i < 65536; i++)
    {
      MEM[i] = 0;
    }

  write_word(0xfffc, 0x200);  // reset vector

}

//////////////////////////////////////////////////////////////////////////////
/// @fn show_regs
/// @brief Display current registers, instruction, etc.
//////////////////////////////////////////////////////////////////////////////
void show_regs(void)
{
  printf("A:%02x X:%02x Y:%02x P:%02x S:%02x PC:%04x\n",
	 A, X, Y, P, S, PC);
}

//////////////////////////////////////////////////////////////////////////////
/// @fn run
/// @brief Run the virtual machine.
//////////////////////////////////////////////////////////////////////////////
int run(void)
{
  uint32_t cycles = 0;
  PC = read_word(0xfffc);
  while(cycles < 100)
    {
      // fetch
      uint8_t op = MEM[PC++];
      // execute
      cycles += operations[op]();
      show_regs();
    }
  return 0;
}

  

//////////////////////////////////////////////////////////////////////////////
/// @fn op_adc
/// @brief Simulate adc instruction.
/// Imm: 69 ZP: 65 ZPx: 75 ABS: 6d ABSx:7d ABSy: 79 INx 61 INDy 71  -- NVZC
/// @return Number of cycles used.
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
  int rtn = 0;

  init();
  run();


  uint8_t x = 0xff;
  int16_t y = (int8_t) x;
  printf("u: %d  s: %d\n", x, y);
  
  return rtn;
}
