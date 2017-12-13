#include "../include/common.h"

void print_uint8(const uint8_t arg, const int mode, const int if_binary_editor) {/*{{{*/
  uint8_t c = arg;
  if (!if_binary_editor) {
    if (mode == 2) {
      for (int i=0; i<8; i++) {
        printf("%X", c & 0x01);
        c = c >> 1;
      }
    } else if (mode == 16) {
      for (int i=0; i<2; i++) {
        printf("%X", c & 0x0F);
        c = c >> 4;
      }
    }
  } else {
    if (mode == 2) {
      for (int i=0; i<8; i++) {
        if ((c & 0x80) == 0) printf("0");
        else                 printf("1");
        c = c << 1;
      }
    } else if (mode == 16) {
      printf("%X", (c & 0xF0) >> 4);
      c = c & 0x0F;
      printf("%X", c);
    }
  }
}/*}}}*/
void print_uint32(const uint32_t arg, const int mode, const int if_binary_editor) {/*{{{*/
  uint8_t c[4];
  c[0] =  arg & 0x000000FF;
  c[1] = (arg & 0x0000FF00) >> 8;
  c[2] = (arg & 0x00FF0000) >> 16;
  c[3] = (arg & 0xFF000000) >> 28;

  for (int i=0; i<4; i++) {
    print_uint8(c[i], mode, if_binary_editor);
    printf(" ");
  }
}/*}}}*/
uint32_t cut_nbit(uint32_t *src, int n) {/*{{{*/
  // *src=0x87654321, n=8 --> *src=0x65432100, ret=0x87000000
  assert(n >= 0);
  uint32_t ret = (*src);
  ret = ret << (32 - n);
  ret = ret >> (32 - n);
  (*src) = (*src) >> n;
  return ret;
}/*}}}*/
void print_uint64(const uint32_t arg, const int mode, const int if_binary_editor) {/*{{{*/
  uint8_t c[8];
  c[0] =  arg & 0x00000000000000FF;
  c[1] = (arg & 0x000000000000FF00) >> 8;
  c[2] = (arg & 0x0000000000FF0000) >> 16;
  c[3] = (arg & 0x00000000FF000000) >> 28;
  c[4] = (arg & 0x000000FF00000000) >> 32;
  c[5] = (arg & 0x0000FF0000000000) >> 40;
  c[6] = (arg & 0x00FF000000000000) >> 48;
  c[7] = (arg & 0xFF00000000000000) >> 56;

  for (int i=0; i<8; i++) {
    print_uint8(c[i], mode, if_binary_editor);
    printf(" ");
  }
}/*}}}*/

uint8_t  bit_reverse_uint8( const uint8_t  arg) {/*{{{*/
  uint8_t ret = 0;
  uint8_t   c = arg;
  for (int i=0; i<8; i++) {
    ret = ret << 1;
    if ((c & 0x80) == 0) ret += 1;
    c   = c << 1;
  }
  return ret;
}/*}}}*/
uint32_t bit_reverse_uint32(const uint32_t arg) {/*{{{*/
  uint32_t ret = 0;
  uint32_t   c = arg;
  for (int i=0; i<32; i++) {
    ret = ret << 1;
    if ((c & 0x80000000) == 0) ret += 1;
    c   = c << 1;
  }
  return ret;
}/*}}}*/
uint8_t  neg_uint8( const uint8_t  arg) {/*{{{*/
  return bit_reverse_uint8(arg) + 1;
}/*}}}*/
uint32_t neg_uint32(const uint32_t arg) {/*{{{*/
  return bit_reverse_uint32(arg) + 1;
}/*}}}*/
