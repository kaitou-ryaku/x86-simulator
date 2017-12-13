#ifndef __COMMON__
#define __COMMON__
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <inttypes.h>

#define MEMORY_COUNT     (1024) // 1KB
#define OPECODE_NUM      (16*16)
#define INIT_EIP_ADDRESS 0

#define _STR(x)        #x
#define _STR2(x)       _STR(x)
#define ERROR_LOCATION fprintf(stderr,"EXIT FAILURE at ");fprintf(stderr, __FILE__ ":" _STR2(__LINE__) ": %s()\n",__func__);
#define EXIT_NOMSG     {ERROR_LOCATION                    ;exit(EXIT_FAILURE);}
#define EXIT_MSG(x)    {ERROR_LOCATION fprintf(stderr,x)  ;exit(EXIT_FAILURE);}
#define EXIT_VAR(x,y)  {ERROR_LOCATION fprintf(stderr,x,y);exit(EXIT_FAILURE);}

typedef struct {
  uint32_t *eax;
  uint32_t *ecx;
  uint32_t *edx;
  uint32_t *ebx;
  uint32_t *esp;
  uint32_t *ebp;
  uint32_t *esi;
  uint32_t *edi;
  uint32_t *eip;
  uint32_t *eflags;
  uint8_t  *memory;
} EMULATOR;

typedef struct {
  // オペコードに関する構造体配列
  // 配列の添字をオペコードとする
  char     *mnemonic;
  void     (*func)();
} OPECODE;

#endif
