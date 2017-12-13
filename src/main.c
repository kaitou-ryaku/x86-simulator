#include "../include/common.h"
#include "../include/misc.h"
#include "../include/emulator.h"
#include "../include/ope.h"
#include "../include/mem.h"

int main(int argc, char* argv[]) {
  // CPUと空のメモリを作成
  EMULATOR emu;
  initialize_EMULATOR(&emu);

  // メモリ配置 [0 ~ bin_size-1 :機械語][bin_size ~ MEMORY_COUNT :スタック]
  if (argc < 2) EXIT_MSG("Binary Image File Not Found.\n");
  FILE *bin;
  bin = fopen(argv[1], "rb");
  if (bin == NULL) EXIT_VAR("Binary Image File ```%s''' Cannot Be Opened.", argv[1]);

  // 機械語のサイズを取得
  fseek(bin, 0, SEEK_END);
  const int bin_size = ftell(bin);
  rewind(bin);

  // 機械語をメモリ上のINIT_EIP_ADDRESSの位置から先に設置
  fread(emu.memory + INIT_EIP_ADDRESS, 1, bin_size, bin);
  fclose(bin);

  // opecodeを作成
  OPECODE ope[OPECODE_NUM];
  initialize_OPECODE(ope);

  for (int i = 0; i < 100000; i++) {
    print_EMULATOR(emu);
    print_memory(emu.memory, bin_size, *(emu.esp));

    const int opecode = readmem_next_uint8(&emu);

    printf("\nCount: %06d    Operation: %06X : 0x%02X (%s)\n\n", i+1, *(emu.eip), opecode, ope[opecode].mnemonic);
    ope[opecode].func(&emu);

    if (opecode == 0xf4) break;
  }

  free_EMULATOR(&emu);
  return 0;
}
