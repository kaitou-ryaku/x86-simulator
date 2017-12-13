#include "../include/common.h"
#include "../include/misc.h"
#include "../include/emulator.h"
#include "../include/mem.h"
#include "../include/flag.h"

static void print_E_register(char *name, const uint32_t *value);

void initialize_EMULATOR(EMULATOR *emu) {/*{{{*/
  emu -> eax    = malloc(sizeof(uint32_t)); *(emu -> eax)    = 0;
  emu -> ecx    = malloc(sizeof(uint32_t)); *(emu -> ecx)    = 0;
  emu -> edx    = malloc(sizeof(uint32_t)); *(emu -> edx)    = 0;
  emu -> ebx    = malloc(sizeof(uint32_t)); *(emu -> ebx)    = 0;

  emu -> esp    = malloc(sizeof(uint32_t)); *(emu -> esp)    = MEMORY_COUNT;
  emu -> ebp    = malloc(sizeof(uint32_t)); *(emu -> ebp)    = 0;
  emu -> esi    = malloc(sizeof(uint32_t)); *(emu -> esi)    = 0;
  emu -> edi    = malloc(sizeof(uint32_t)); *(emu -> edi)    = 0;

  emu -> eip    = malloc(sizeof(uint32_t)); *(emu -> eip)    = INIT_EIP_ADDRESS;
  emu -> eflags = malloc(sizeof(uint32_t)); *(emu -> eflags) = 0;

  const size_t memsize = MEMORY_COUNT*sizeof(uint8_t);
  emu -> memory = malloc(memsize);
  for (int i=0; i<MEMORY_COUNT; i++) emu -> memory[i] = 0x0;
}/*}}}*/
void free_EMULATOR(EMULATOR *emu) {/*{{{*/
  free(emu -> eax);
  free(emu -> ecx);
  free(emu -> edx);
  free(emu -> ebx);
  free(emu -> esp);
  free(emu -> ebp);
  free(emu -> esi);
  free(emu -> edi);
  free(emu -> eip);
  free(emu -> eflags);
  free(emu -> memory);
}/*}}}*/
void print_EMULATOR(const EMULATOR emu) {/*{{{*/
  // フラグ
  print_flag(emu);

  printf("---- Plain Order Bit Expression            Binary Editor Expression              Hex Dump      Integer      Decimal\n");
  // 汎用レジスタ
  print_E_register("EAX", emu.eax);
  print_E_register("ECX", emu.ecx);
  print_E_register("EDX", emu.edx);
  print_E_register("EBX", emu.ebx);

  // セグメント、スタックポインタ
  print_E_register("ESP", emu.esp);
  print_E_register("EBP", emu.ebp);
  print_E_register("ESI", emu.esi);
  print_E_register("EDI", emu.edi);

  // カウンタ
  print_E_register("EIP", emu.eip);
}/*}}}*/
static void print_E_register(char *name, const uint32_t *value) {/*{{{*/
  int if_binary_editor = 0;
  printf("%s: ", name);
  print_uint32(*value, 2, if_binary_editor);

  if_binary_editor = 1;
  printf(": ");
  print_uint32(*value, 2, if_binary_editor);
  printf(": ");
  print_uint32(*value,16, if_binary_editor);
  printf(": ");
  printf("0x%08X ", *value);
  printf(": ");
  printf("%d\n", (int)*value);
}/*}}}*/
