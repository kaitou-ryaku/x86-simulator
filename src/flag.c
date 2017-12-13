#include "../include/common.h"
#include "../include/flag.h"
#include "../include/misc.h"

typedef struct {
  int carry;
  int zero;
  int sign;
  int overflow;
} FLAG;

static void write_flag(const FLAG flag, EMULATOR *emu);
static void write_each_flag(const int flag_num, const int flag, EMULATOR *emu);

void update_flag(const uint32_t rnd1, const uint32_t rnd2, EMULATOR *emu) {/*{{{*/
  // rnd1+rnd2でflagを更新
  // PFは未実装
  const int sign_1      = rnd1 >> 31;
  const int sign_2      = rnd2 >> 31;

  const uint64_t result = (uint64_t)rnd1 + (uint64_t)rnd2;
  const int sign_result = (result >> 31) & 1;

  FLAG flag = {0,0,0,0};
  flag.carry    = (result >> 32) & 1;
  flag.overflow = (sign_1 == sign_2 && sign_1 != sign_result);
  flag.sign     = (sign_result == 1);
  flag.zero     = (rnd1 + rnd2 == 0);

  printf("rnd1  :");
  print_uint32(rnd1, 2, 0);
  printf("--\n");

  printf("rnd2  :");
  print_uint32(rnd2, 2, 0);
  printf("--\n");

  printf("result:");
  print_uint64(result, 2, 0);
  printf("--\n");

  printf("flag.zero:%d\n", flag.zero);
  write_flag(flag, emu);
}/*}}}*/
static void write_flag(const FLAG flag, EMULATOR *emu) {/*{{{*/
  write_each_flag( FLAG_NAME_CARRY   , flag.carry   , emu);
  write_each_flag( FLAG_NAME_ZERO    , flag.zero    , emu);
  write_each_flag( FLAG_NAME_SIGN    , flag.sign    , emu);
  write_each_flag( FLAG_NAME_OVERFLOW, flag.overflow, emu);
}/*}}}*/
static void write_each_flag(const int flag_num, const int flag, EMULATOR *emu) {/*{{{*/
  // zero_1_zero = 00001000
  uint32_t zero_1_zero = 1;
  for (int i=0; i<flag_num; i++) zero_1_zero *= 2;

  if (flag == 1) {
    // eflags = eflags OR  00001000
    *(emu -> eflags) = *(emu -> eflags) | zero_1_zero;

  } else {
    // eflags = eflags AND 11110111
    uint32_t one_0_one = 0xffffffff - zero_1_zero;
    *(emu -> eflags) = *(emu -> eflags) & one_0_one;
  }
}/*}}}*/
int read_flag(const int flag_num, const EMULATOR emu) {/*{{{*/
  // zero_1_zero = 00001000
  uint32_t zero_1_zero = 1;
  for (int i=0; i<flag_num; i++) zero_1_zero *= 2;

  int ret;
  if ( (*(emu.eflags) & zero_1_zero) != 0) ret = 1;
  else                                     ret = 0;
  return ret;
}/*}}}*/
void print_flag(const EMULATOR emu) {/*{{{*/
  int carry    = read_flag( FLAG_NAME_CARRY   , emu);
  int zero     = read_flag( FLAG_NAME_ZERO    , emu);
  int sign     = read_flag( FLAG_NAME_SIGN    , emu);
  int overflow = read_flag( FLAG_NAME_OVERFLOW, emu);
  printf("CF:%d ZF:%d SF:%d OF:%d\n", carry, zero, sign, overflow);
}/*}}}*/
