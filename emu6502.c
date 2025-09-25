// emu6502
// 6502 emulator
// copyright 2025 William R Cooke
// 20250916

#include <stdio.h>
#include <stdint.h>


uint16_t PC;
uint8_t A;
uint8_t X;
uint8_t Y;
uint8_t S;
uint8_t P;

// TODO get bit positions
 enum flag
  {
    FL_N = (1<<7),
    FL_V = (1<<6),
    FL_UNUSED = (1<<5),
    FL_B = (1<<4),
    FL_D = (1<<3),
    FL_I = (1<<2),
    FL_Z = (1<<1),
    FL_C = (1<<0),
  }  flags;
#define FL_UNUSED_MASK = (1<<5 || 1<<4)

uint8_t MEM[65536];

typedef uint8_t (*op_fn)(void);


// op functions
uint8_t op_illegal(void);
//uint8_t op_nop(void);
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

// Function pointer table
// Initialize to base NMOS 6502
// Can be added to later for other models
op_fn operations[256] =
  {
    op_brk,          // 00   brk
    op_ora,          // 01 ora (xx,X)
    op_illegal,          // 02
    op_illegal,          // 03
    op_illegal,          // 04
    op_ora,          // 05 ora xx
    op_asl,          // 06
    op_illegal,          // 07
    op_php,          // 08 php
    op_ora,          // 09 ora #xx
    op_asl,          // 0a
    op_illegal,          // 0b
    op_illegal,          // 0c
    op_ora,          // 0d ora xxxx
    op_asl,          // 0e
    op_illegal,          // 0f
    op_branch,           // 10  BPL
    op_ora,          // 11 ora (xx),Y
    op_illegal,          // 12
    op_illegal,          // 13
    op_illegal,          // 14
    op_ora,          // 15 ora xx,X
    op_asl,          // 16
    op_illegal,          // 17
    op_clc,          // 18 CLC
    op_ora,          // 19 ora xxxx,Y
    op_illegal,          // 1a
    op_illegal,          // 1b
    op_illegal,          // 1c
    op_ora,          // 1d ora xxxx,X
    op_asl,          // 1e
    op_illegal,          // 1f
    op_jsr,          // 20 jsr xxxx
    op_and,          // 21
    op_illegal,          // 22
    op_illegal,          // 23
    op_bit,          // 24 bit xx
    op_and,          // 25
    op_rol,          // 26 rol xx
    op_illegal,          // 27
    op_plp,          // 28 PLP
    op_and,          // 29
    op_rol,          // 2a rol A
    op_illegal,          // 2b
    op_bit,          // 2c bit xxxx
    op_and,          // 2d
    op_rol,          // 2e rol xxxx
    op_illegal,          // 2f
    op_branch,           // 30  BMI
    op_and,          // 31
    op_illegal,          // 32
    op_illegal,          // 33
    op_illegal,          // 34
    op_and,          // 35
    op_rol,          // 36 rol xx,X
    op_illegal,          // 37
    op_sec,          // 38 SEC
    op_and,          // 39
    op_illegal,          // 3a
    op_illegal,          // 3b
    op_illegal,          // 3c
    op_and,          // 3d
    op_rol,          // 3e rol xxxx,X
    op_illegal,          // 3f
    op_rti,          // 40 rti
    op_eor,          // 41  EOR (xx,X)
    op_illegal,          // 42
    op_illegal,          // 43
    op_illegal,          // 44
    op_eor,          // 45 eor xx
    op_lsr,          // 46 lsr xx
    op_illegal,          // 47
    op_pha,          // 48 pha
    op_eor,          // 49 EOR #xx
    op_lsr,          // 4a lsr a
    op_illegal,          // 4b
    op_jmp,          // 4c jmp xxxx
    op_eor,          // 4d EOR xxxx
    op_lsr,          // 4e lsr xxxx
    op_illegal,          // 4f
    op_branch,           // 50  BVC
    op_eor,          // 51 EOR (xx),Y
    op_illegal,          // 52
    op_illegal,          // 53
    op_illegal,          // 54
    op_eor,          // 55  EOR xx,X
    op_lsr,          // 56 lsr xx,X
    op_illegal,          // 57
    op_cli,          // 58 CLI
    op_eor,          // 59 EOR xxxx,Y
    op_illegal,          // 5a
    op_illegal,          // 5b
    op_illegal,          // 5c
    op_eor,          // 5d EOR xxxx,X
    op_lsr,          // 5e lsr xxxx,X
    op_illegal,          // 5f
    op_rts,          // 60 rts
    op_adc,          // 61
    op_illegal,          // 62
    op_illegal,          // 63
    op_illegal,          // 64
    op_adc,          // 65
    op_ror,          // 66 ror xx
    op_illegal,          // 67
    op_pla,          // 68 PLA
    op_adc,          // 69
    op_ror,          // 6a ror A
    op_illegal,          // 6b
    op_jmpi,          // 6c jmp (xxxx)
    op_adc,          // 6d
    op_ror,          // 6e ror xxxx
    op_illegal,          // 6f
    op_branch,           // 70  BVS
    op_adc,          // 71
    op_illegal,          // 72
    op_illegal,          // 73
    op_illegal,          // 74
    op_adc,          // 75
    op_ror,          // 76 ror xx,X
    op_illegal,          // 77
    op_sei,          // 78 SEI
    op_adc,          // 79
    op_illegal,          // 7a
    op_illegal,          // 7b
    op_illegal,          // 7c
    op_adc,          // 7d
    op_ror,          // 7e ror xxxx,X
    op_illegal,          // 7f
    op_illegal,          // 80
    op_sta,          // 81 sta (xx,X)
    op_illegal,          // 82
    op_illegal,          // 83
    op_sty,          // 84 sty xx
    op_sta,          // 85 sta xx
    op_stx,          // 86 stx xx
    op_illegal,          // 87
    op_dey,          // 88 dey
    op_illegal,          // 89
    op_txa,          // 8a txa
    op_illegal,          // 8b
    op_sty,          // 8c sty xxxx
    op_sta,          // 8d sta xxxx
    op_stx,          // 8e stx xxxx
    op_illegal,          // 8f
    op_branch,           // 90   BCC
    op_sta,          // 91 sta (xx),Y
    op_illegal,          // 92
    op_illegal,          // 93
    op_sty,          // 94 sty xx,X
    op_sta,          // 95 sta xx,X
    op_stx,          // 96 stx xx,Y
    op_illegal,          // 97
    op_tya,          // 98 tya
    op_sta,          // 99 sta xxxx,Y
    op_txs,          // 9a txs
    op_illegal,          // 9b
    op_illegal,          // 9c
    op_sta,          // 9d sta xxxx,X
    op_illegal,          // 9e
    op_illegal,          // 9f
    op_ldy,          // a0 ldy #xx
    op_lda,          // a1 lda (xx),Y
    op_ldx,          // a2 ldx #xx
    op_illegal,          // a3
    op_ldy,          // a4 ldy xx
    op_lda,          // a5 lda xx
    op_ldx,          // a6 ldx xx
    op_illegal,          // a7
    op_tay,          // a8 tay
    op_lda,          // a9 lda #xx
    op_tax,          // aa tax
    op_illegal,          // ab
    op_ldy,          // ac ldy xxxx
    op_lda,          // ad lda xxxx
    op_ldx,          // ae ldx xxxx
    op_illegal,          // af
    op_branch,           // b0  // BCS
    op_lda,          // b1 lda (xx),Y
    op_illegal,          // b2
    op_illegal,          // b3
    op_ldy,          // b4 ldy xx,X
    op_lda,          // b5 lda xx,X
    op_ldx,          // b6 ldx xx,Y
    op_illegal,          // b7
    op_clv,          // b8 CLV
    op_lda,          // b9 lda xxxx,Y
    op_tsx,          // ba tsx
    op_illegal,          // bb
    op_ldy,          // bc ldy xxxx,X
    op_lda,          // bd lda xxxx,X
    op_ldx,          // be ldx xxxx,Y
    op_illegal,          // bf
    op_cpy,          // c0 cpy #xx
    op_cmp,          // c1 cmp (xx,X)
    op_illegal,          // c2
    op_illegal,          // c3
    op_cpy,          // c4 cpy xx
    op_cmp,          // c5 cmp xx
    op_dec,          // c6 dec xx
    op_illegal,          // c7
    op_iny,          // c8 iny
    op_cmp,          // c9 cmp #xx
    op_dex,          // ca dex
    op_illegal,          // cb
    op_cpy,          // cc  cpy xxxx
    op_cmp,          // cd cmp xxxx
    op_dec,          // ce dec xxxx
    op_illegal,          // cf
    op_branch,           // d0  BNE
    op_cmp,          // d1 cmp (xx),Y
    op_illegal,          // d2
    op_illegal,          // d3
    op_illegal,          // d4
    op_cmp,          // d5 cmp xx,X
    op_dec,          // d6 dec xx,X
    op_illegal,          // d7
    op_cld,          // d8 CLD
    op_cmp,          // d9 cmp xxxx,Y
    op_illegal,          // da
    op_illegal,          // db
    op_illegal,          // dc
    op_cmp,          // dd cmp xxxx,X
    op_dec,          // de dec xxxx,X
    op_illegal,          // df
    op_cpx,          // e0 cpx #xx
    op_sbc,          // e1 sbc (xx,X)
    op_illegal,          // e2
    op_illegal,          // e3
    op_cpx,          // e4 cpx  xx
    op_sbc,          // e5 sbc xx
    op_inc,          // e6 inc xx
    op_illegal,          // e7
    op_inx,          // e8 inx
    op_sbc,          // e9 sbc #xx
    op_illegal,          // ea
    op_illegal,          // eb
    op_cpx,          // ec cpx xxxx
    op_sbc,          // ed sbc xxxx
    op_inc,          // ee inc xxxx
    op_illegal,          // ef
    op_branch,           // f0  BEQ
    op_sbc,          // f1 sbc (xx),Y
    op_illegal,          // f2
    op_illegal,          // f3
    op_illegal,          // f4
    op_sbc,          // f5 sbc xx,X
    op_inc,          // f6 inc xx,X
    op_illegal,          // f7
    op_sed,          // f8 SED
    op_sbc,          // f9 sbc xxxx,Y
    op_illegal,          // fa
    op_illegal,          // fb
    op_illegal,          // fc
    op_sbc,          // fd sbc xxxx,X
    op_inc,          // fe inc xxxx,X
    op_illegal           // ff
  };



uint16_t read_word(uint16_t addr)
{
  uint16_t rtn;

  rtn = MEM[addr] + (MEM[addr +1] << 8);
  return rtn;
}

void write_word(uint16_t address, uint16_t word)
{
  MEM[address] = (uint8_t) (word & 0xff);
  MEM[address + 1] = (uint8_t) ((word >> 8) & 0xff);
}

uint8_t op_illegal(void)
{
  printf("ILLEGAL inst %02x at %04x\n", MEM[PC], PC);
  PC++;
  return 1;
}

// adc
// I 69, ZP 65, ZPx 75, ABS 6d, ABSx 7d, ABSy 79, INDx 61, INDy 71-NVZC
uint8_t op_adc(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;   // second operand (A is first)
  uint16_t address = 0;
  switch(MEM[PC++])
    {
    case 0x69:   // adc #xx
      address = PC++;
      break;
    case 0x65:   // adc xx
      address = MEM[PC++];
      break;
    case 0x75:   // adc xx,X
      address = MEM[PC++];
      address += X;
      address &= 0xff;
      break;
    case 0x6d:   // adc xxxx
      address = MEM[PC++];
      address |= (MEM[PC++] << 8);
      break;
    case 0x7d:   // adc xxxx,X
      address = MEM[PC++];
      address |= (MEM[PC++] << 8);
      address += X;
      break;
    case 0x79:   // adc xxxx,Y
      address = MEM[PC++];
      address |= (MEM[PC++] << 8);
      address += Y;
      break;
    case 0x61:   // adc (xx,X)
      address = MEM[PC++];
      address = (address + X) & 0xff;
      address = read_word(address);
      break;
    case 0x71:   // adc (xx),Y
      address = MEM[PC++];
      address = read_word(address) + Y;
    default:
      break;
    }
  // clear used flags
  P &= ~(FL_C | FL_V | FL_V | FL_Z);
  b = MEM[address];
  b += A;
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

// and
// I 29, ZP 25, ZPx 35, ABS 2d, ABSx 3d, ABSy 39, INDx 21, INDy 31 -- NZ
uint8_t op_and(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
    {
    case 0x29:   // and #xx
	break;
    case 0x25:   // and xx
      break;
    case 0x35:   // and xx,X
      break;
    case 0x2d:   // and xxxx
      break;
    case 0x3d:   // and xxxx,X
      break;
    case 0x39:   // and xxxx,Y
      break;
    case 0x21:   // and (xx,X)
      break;
    case 0x31:   // and (xx),Y
      break;
    default:
      break;
    }
      
      
  return rtn;
}

// asl
// A 0a, ZP 06, ZPx 16, ABS 0e, ABSx 1e -- NZC
uint8_t op_asl(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
    {
    case 0x0a:   // asl A
	break;
    case 0x06:   // asl xx
	break;
    case 0x16:   // asl xx,X
	break;
    case 0x0e:   // asl xxxx
	break;
    case 0x1e:   // asl xxxx,X
	break;
    default:
      break;
    }

  return rtn;
}

// BIT ZP 24, ABS 2c -- NVZ
uint8_t op_bit(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
    {
    case 0x24:   // bit xx
      break;
    case 0x2c:   // bit xxxx
      break;
    default:
      break;
    }
  
  return rtn;
}

// bpl, bmi, bvs, bvc, bcs, bcc, bne, beq -- none
uint8_t op_branch(void)
{
  uint8_t rtn = 2;
  uint8_t op = MEM[PC++];
  uint8_t disp = MEM[PC++];
  // have to cast to get negative displacements
  int16_t addend = (int16_t) (int8_t)disp;
  uint16_t new_pc = (uint16_t)(PC + addend);
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
    }
  // TODO fix this  Need to inow if cross page
  
  return rtn;
}

// brk 00 -- B
uint8_t op_brk(void)
{
  uint8_t rtn = 2;
  printf("brk instr %2x at %4x\n", MEM[PC], PC);
  PC ++;
  return rtn;   // TODO:  remove this line
  MEM[0x100 | S--] = (uint8_t)(PC >> 8);
  MEM[0x100 | S--] = (uint8_t)(PC & 0xff);
  MEM[0x100 | S--] = (uint8_t)(P | FL_B | FL_I);
  uint16_t address = MEM[0xfffe];
  address |= MEM[0xffff] << 8;
  PC = address;

  return rtn;
}

// clc -- C
uint8_t op_clc(void)
{
  uint8_t rtn = 2;
  P &= ~ FL_C;
  PC++;
  return rtn;
}

// cld --  D
uint8_t op_cld(void)
{
  uint8_t rtn = 2;
  P &= ~ FL_D;
  PC++;
  return rtn;
}

// cli -- I
uint8_t op_cli(void)
{
  uint8_t rtn = 2;
  P &= ~ FL_I;
  PC++;
  return rtn;
}

// clv -- V
uint8_t op_clv(void)
{
  uint8_t rtn = 2;
  P &= ~FL_V;
  PC++;
  return rtn;
}


// cmp I C9 ZP C5 ZPx D5 ABS cd abs x dd abs y d9
//     Ix c1 Iy d1
uint8_t op_cmp(void)
{
  uint8_t rtn = 0;
  uint16_t b = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0xc9:   // cmp #xx
      address = PC++;
      PC++;
      break;
    case 0xc5:   // cmp xx
      address = MEM[PC++];
      break;
    case 0xd5:   // cmp xx,X
      break;
    case 0xcd:   // cmp xxxx
      break;
    case 0xdd:   // cmp xxxx,X
      break;
    case 0xd9:   // cmp xxxx,Y
      break;
    case 0xc1:   // cmp (xx,X)
      break;
    case 0xd1:   // cmp (xx),Y
      break;
    default:
      break;
    }
  b = MEM[address];
  // TODO finish

  return rtn;
}

// cpx  I e0 ZP e4 ABS ec -- NZC
uint8_t op_cpx(void)
{
  uint8_t rtn = 0;
  switch(MEM[PC])
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

// cpy I c0 ZP c4 ABS cc
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

// dec ZP c6 ZP,X D6 ABS ce ABS,X de
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
// dex -- NZ
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

// dey -- NZ
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


// eor I 49 ZP 45 ZP,X 55 ABS 4D ABS,X 5D ABS,Y 59 IX 41 IY 57 --
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

// inc ZP e6 ZP,X f6 ABS ee ABS,X fe  -- NZ
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

// inx -- NZ
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

// iny -- NZ
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


// jmp xxxx   4c
uint8_t op_jmp(void)
{
  uint8_t rtn = 4;  // TODO:  get real value
  PC++;
  PC = read_word(PC);
  return rtn;
}

// jmp (xxxx) 6c
uint8_t op_jmpi(void)
{
  // Make sure to check last page byte
  uint8_t rtn = 4;   // TODO get real value
  PC = read_word( read_word(PC + 1));
  return rtn;
}

// jsr xxxx
uint8_t op_jsr(void)
{
  uint8_t rtn = 6;
  return rtn;
}

// lda I a9 ZP a5 ZPX b5 ABS ad ABS,X bd ABS,Y b9  -- NZ
//     IX a1 IY b1
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

// ldx I a2 ZP a6 ZPY b6 ABS ae ABSY be  -- NZ
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

// ldy I a0 ZP a4 ZPX b4 abs ac abx,x bc -- NZ
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

// lsr A 4a ZP 46 ZPX 56 ABS 4e ABSX 5e -- NZC
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



uint8_t op_nop(void)
{
  PC++;
  return 2;
}



// ora I 09 ZP 05 ZPX 15 ABS 0d ABSX 1D ABSY 19 IX 01 IY 11 -- NZ
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

// rol  A 2a ZP 26 ZPX 36 ABS 2e ABSX 3e -- NZC
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

// pha -- TODO:
uint8_t op_pha(void)
{
  MEM[0x100 | S] = A;
  S--;
  return 3;
}

// php -- TODO:
uint8_t op_php(void)
{
  // TODO careful with B and unused flags
  return 3;
}


// pla -- TODO:
uint8_t op_pla(void)
{
  A = MEM[0x100 | S];
  S++;
  // TODO flags
  return 4;
}

// plp -- TODO:
uint8_t op_plp(void)
{
  // TODO careful with B and unused flags
  return 4;
}

// ror A 6a  ZP 66 ZPX 76 ABS 6e ABSX 7e -- NZC
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

// rti -- all
uint8_t op_rti(void)
{
  return 6;
}

// rts -- none
uint8_t op_rts(void)
{
  uint16_t address = MEM[0x100 | S];
  S++;
  address |= (MEM[0x100 | S] << 8);
  S++;
  PC = address;
  return 6;
}

// sbc I e9 ZP e5 ZPX f5 ABS ed ABSX fd ABSY f9 IX e1 IY f1 -- NVZC
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


// sec -- C
uint8_t op_sec(void)
{
  uint8_t rtn = 2;
  P |= FL_C;
  PC++;
  return rtn;
}

// sed -- D
uint8_t op_sed(void)
{
  uint8_t rtn = 2;
  P |= FL_D;
  PC++;
  return rtn;
}

// sei -- I
uint8_t op_sei(void)
{
  uint8_t rtn = 2;
  P |= FL_I;
  PC++;
  return rtn;
}

// sta ZP 85 ZPX 95 ABS 8d ABSX 9d ABSY 99 IX 81 IY 91 -- NONE
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

// stx ZP 86 ZPY 96 ABS 8e -- NONE
uint8_t op_stx(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0x86:   // stx xx
      address = MEM[PC++];
      break;
    case 0x96:   // stx xx,Y
      break;
    case 0x8e:   // stx xxxx
      break;
    default:
      break;
    }
  MEM[address] = X;
  return rtn;
}

// sty ZP 84 ZPX 94 ABS 8c -- NONE
uint8_t op_sty(void)
{
  uint8_t rtn = 0;
  uint16_t address = 0;
  switch(MEM[PC])
    {
    case 0x84:   // sty xx
      address = MEM[PC++];
      break;
    case 0x94:   // sty xx,X
      break;
    case 0x8c:   // sty xxxx
      break;
    default:
      break;
    }
  MEM[address] = Y;
  return rtn;
}

// tax -- NZ
uint8_t op_tax(void)
{
  return 2;
}

// tay -- NZ
uint8_t op_tay(void)
{
  return 2;
}

// tsx -- TODO:
uint8_t op_tsx(void)
{
  return 2;
}

// txa -- NZ
uint8_t op_txa(void)
{
  return 2;
}

// txs -- TODO:
uint8_t op_txs(void)
{
  return 2;
}

// tya -- NZ
uint8_t op_tya(void)
{
  return 2;
}

//////////////////////////////////////////////////////
//////////////////////////////////////////////////////

void init(void)
{
  for(int i = 0; i < 65536; i++)
    {
      MEM[i] = 0;
    }

  write_word(0xfffc, 0x200);  // reset vector

  PC = read_word(0xfffc);
}

void show_regs(void)
{
  printf("A:%02x X:%02x Y:%02x P:%02x S:%02x PC:%04x\n",
	 A, X, Y, P, S, PC);
}

int run(void)
{
  uint32_t cycles = 0;
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
