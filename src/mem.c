#include "../include/common.h"
#include "../include/misc.h"
#include "../include/mem.h"

uint32_t readmem_uint32(uint32_t addr, const EMULATOR emu) {/*{{{*/
  // nasm 0x12345678 ---> m[add],m[add+1],m[add+2],m[add+3] = 78,56,34,12
  // emulator.memory[addr~addr+3]=78,56,34,12 ---> ret = 87,65,43,21
  // printf("%X", ret) ---> 12345678
  uint32_t ret = 0x0;
  for (int i=0; i<4; i++) {
    ret = ret << 8;
    ret += emu.memory[addr + 0x3 - i];
  }
  return ret;
}/*}}}*/
uint8_t  readmem_uint8( uint32_t addr, const EMULATOR emu) {/*{{{*/
  return emu.memory[addr];
}/*}}}*/

uint32_t readmem_next_uint32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_uint32(*(emu -> eip), *emu);
  *(emu -> eip) += 4;
  return imm32;
}/*}}}*/
uint8_t  readmem_next_uint8( EMULATOR *emu) {/*{{{*/
  uint8_t  imm8  = readmem_uint8( *(emu -> eip), *emu);
  *(emu -> eip) += 1;
  return imm8;
}/*}}}*/

void writemem_uint32(uint32_t addr, const uint32_t src, EMULATOR *emu) {/*{{{*/
  uint32_t tmp = src;
  for (int i=0; i<4; i++) {
    writemem_uint8( addr+i, (tmp & 0xFF), emu);
    tmp = tmp >> 8;
  }
}/*}}}*/
void writemem_uint8( uint32_t addr, const uint8_t  src, EMULATOR *emu) {/*{{{*/
  emu -> memory[addr] = src;
}/*}}}*/

void print_memory(const uint8_t memory[MEMORY_COUNT], const int bin_size, const int esp) {/*{{{*/

  const int width = 0x10;
  const int if_binary_editor = 1;

  int addr = INIT_EIP_ADDRESS;
  while (addr < INIT_EIP_ADDRESS + bin_size) {
    if (addr % width == 0)         printf("%06X:", addr);
    printf(" ");
    print_uint8(memory[addr], 16, if_binary_editor);
    if (addr % width == width - 1) printf("\n");
    addr++;
  }

  printf("\n");
  printf("PROGRAM GLOBAL_VARIABLE / STACK\n");

  addr = (esp / width) * width; // esp以下のwidth倍数の最大値
  while (addr < MEMORY_COUNT) {
    if (addr % width == 0)         printf("%06X:", addr);
    printf(" ");
    print_uint8(memory[addr], 16, if_binary_editor);
    if (addr % width == width - 1) printf("\n");
    addr++;
  }
}/*}}}*/
