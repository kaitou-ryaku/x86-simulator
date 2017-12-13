#include "../include/common.h"
#include "../include/misc.h"
#include "../include/ope.h"
#include "../include/mem.h"
#include "../include/flag.h"

// 構造体/*{{{*/
typedef struct {
  uint8_t modrm;
  uint8_t   mod;
  uint8_t raw_r;
  uint8_t raw_m;
  uint32_t   *R;
  uint32_t   *M;
} MODRM;
/*}}}*/
// 関数プロトタイプ/*{{{*/
static MODRM     read_modrm(EMULATOR *emu);
static uint8_t   modrm_mod( const uint8_t modrm);
static uint8_t   modrm_r(   const uint8_t modrm);
static uint8_t   modrm_m(   const uint8_t modrm);
static uint32_t *modrm_r32p(const uint8_t modrm, const EMULATOR *emu);
static uint32_t *modrm_m32p(const uint8_t modrm, const EMULATOR *emu);

static void not_defined_halt(EMULATOR *emu);
static void cut_esp_0x24(const uint32_t *eXX, EMULATOR *emu);
static uint32_t  modrm_M_imm_to_addr(const MODRM m, EMULATOR *emu);
static uint32_t  modrm_M_imm_to_src( const MODRM m, EMULATOR *emu);

static void add_Mimm_R( EMULATOR *emu); // add [M+imm], R      ; 01 b([00~11] R M) imm  (mod=11なら regM <- R)
static void or__Mimm_R( EMULATOR *emu); // or  [M+imm], R      ; 09 b([00~11] R M) imm  (mod=11なら regM <- R)
static void adc_Mimm_R( EMULATOR *emu); // adc [M+imm], R      ; 11 b([00~11] R M) imm  (mod=11なら regM <- R)
static void sbb_Mimm_R( EMULATOR *emu); // sbb [M+imm], R      ; 19 b([00~11] R M) imm  (mod=11なら regM <- R)
static void and_Mimm_R( EMULATOR *emu); // and [M+imm], R      ; 21 b([00~11] R M) imm  (mod=11なら regM <- R)
static void sub_Mimm_R( EMULATOR *emu); // sub [M+imm], R      ; 29 b([00~11] R M) imm  (mod=11なら regM <- R)
static void xor_Mimm_R( EMULATOR *emu); // xor [M+imm], R      ; 31 b([00~11] R M) imm  (mod=11なら regM <- R)
static void cmp_Mimm_R( EMULATOR *emu); // cmp [M+imm], R      ; 39 b([00~11] R M) imm  (mod=11なら regM <- R)

static void add_R_Mimm( EMULATOR *emu); // add R, [M+imm]      ; 03 b([00~10] R M) imm
static void or__R_Mimm( EMULATOR *emu); // or  R, [M+imm]      ; 0b b([00~10] R M) imm
static void adc_R_Mimm( EMULATOR *emu); // adc R, [M+imm]      ; 13 b([00~10] R M) imm
static void sbb_R_Mimm( EMULATOR *emu); // sbb R, [M+imm]      ; 1b b([00~10] R M) imm
static void and_R_Mimm( EMULATOR *emu); // and R, [M+imm]      ; 23 b([00~10] R M) imm
static void sub_R_Mimm( EMULATOR *emu); // sub R, [M+imm]      ; 2b b([00~10] R M) imm
static void xor_R_Mimm( EMULATOR *emu); // xor R, [M+imm]      ; 33 b([00~10] R M) imm
static void cmp_R_Mimm( EMULATOR *emu); // cmp R, [M+imm]      ; 3b b([00~10] R M) imm

static void add_eax_imm32(EMULATOR *emu); // add eax, 0x12345678 ; 05 78 56 34 12
static void or__eax_imm32(EMULATOR *emu); // or  eax, 0x12345678 ; 0d 78 56 34 12
static void adc_eax_imm32(EMULATOR *emu); // adc eax, 0x12345678 ; 15 78 56 34 12
static void sbb_eax_imm32(EMULATOR *emu); // sbb eax, 0x12345678 ; 1d 78 56 34 12
static void and_eax_imm32(EMULATOR *emu); // and eax, 0x12345678 ; 25 78 56 34 12
static void sub_eax_imm32(EMULATOR *emu); // sub eax, 0x12345678 ; 2d 78 56 34 12
static void xor_eax_imm32(EMULATOR *emu); // xor eax, 0x12345678 ; 35 78 56 34 12
static void cmp_eax_imm32(EMULATOR *emu); // cmp eax, 0x12345678 ; 3d 78 56 34 12

static void calR_M_imm32(EMULATOR *emu); // add M, 0x12345678   ; 81 b(11 000 M) 78 56 34 12 (M != eax)
static void calR_M_imm8( EMULATOR *emu); // add M, 0x12         ; 83 b(11 000 M) 12

static void inc_eax(EMULATOR *emu);    // inc  eax ; 40
static void inc_ecx(EMULATOR *emu);    // inc  ecx ; 41
static void inc_edx(EMULATOR *emu);    // inc  edx ; 42
static void inc_ebx(EMULATOR *emu);    // inc  ebx ; 43
static void inc_esp(EMULATOR *emu);    // inc  esp ; 44
static void inc_ebp(EMULATOR *emu);    // inc  ebp ; 45
static void inc_esi(EMULATOR *emu);    // inc  esi ; 46
static void inc_edi(EMULATOR *emu);    // inc  edi ; 47

static void dec_eax(EMULATOR *emu);    // dec  eax ; 48
static void dec_ecx(EMULATOR *emu);    // dec  ecx ; 49
static void dec_edx(EMULATOR *emu);    // dec  edx ; 4a
static void dec_ebx(EMULATOR *emu);    // dec  ebx ; 4b
static void dec_esp(EMULATOR *emu);    // dec  esp ; 4c
static void dec_ebp(EMULATOR *emu);    // dec  ebp ; 4d
static void dec_esi(EMULATOR *emu);    // dec  esi ; 4e
static void dec_edi(EMULATOR *emu);    // dec  edi ; 4f

static void push_eax(EMULATOR *emu);   // push eax ; 50
static void push_ecx(EMULATOR *emu);   // push ecx ; 51
static void push_edx(EMULATOR *emu);   // push edx ; 52
static void push_ebx(EMULATOR *emu);   // push ebx ; 53
static void push_esp(EMULATOR *emu);   // push esp ; 54
static void push_ebp(EMULATOR *emu);   // push ebp ; 55
static void push_esi(EMULATOR *emu);   // push esi ; 56
static void push_edi(EMULATOR *emu);   // push edi ; 57
static void push_imm32(EMULATOR *emu); // push 0x12345678 ; 68 78 56 34 12
static void push_imm8( EMULATOR *emu); // push 0x12       ; 6a 12

static void pop_eax( EMULATOR *emu); // pop  eax ; 58
static void pop_ecx( EMULATOR *emu); // pop  ecx ; 59
static void pop_edx( EMULATOR *emu); // pop  edx ; 5a
static void pop_ebx( EMULATOR *emu); // pop  ebx ; 5b
static void pop_esp( EMULATOR *emu); // pop  esp ; 5c
static void pop_ebp( EMULATOR *emu); // pop  ebp ; 5d
static void pop_esi( EMULATOR *emu); // pop  esi ; 5e
static void pop_edi( EMULATOR *emu); // pop  edi ; 5f

static void call_imm32(EMULATOR *emu); // call 0x5        ; e8 00 00 00 00 (普通に命令更新した後のeipからの差分をimmで与える)
static void ret_near(EMULATOR *emu);   // ret             ; c3 (pop eip)
static void jmp_imm32(EMULATOR *emu);  // jmp  0x12340005 ; e9 00 00 34 12 (普通に命令更新した後のeipからの差分をimmで与える)
static void jmp_imm8(EMULATOR *emu);   // jmp  0x5        ; eb 00          (普通に命令更新した後のeipからの差分をimmで与える)

static void jo__imm8(EMULATOR *emu);   // jo  imm8        ; 70 imm8
static void jno_imm8(EMULATOR *emu);   // jno imm8        ; 71 imm8
static void jb__imm8(EMULATOR *emu);   // jb  imm8        ; 72 imm8
static void jae_imm8(EMULATOR *emu);   // jae imm8        ; 73 imm8
static void je__imm8(EMULATOR *emu);   // je  imm8        ; 74 imm8
static void jne_imm8(EMULATOR *emu);   // jne imm8        ; 75 imm8
static void jbe_imm8(EMULATOR *emu);   // jbe imm8        ; 76 imm8
static void ja__imm8(EMULATOR *emu);   // ja  imm8        ; 77 imm8
static void js__imm8(EMULATOR *emu);   // js  imm8        ; 78 imm8
static void jns_imm8(EMULATOR *emu);   // jns imm8        ; 79 imm8
static void jp__imm8(EMULATOR *emu);   // jp  imm8        ; 7a imm8
static void jnp_imm8(EMULATOR *emu);   // jnp imm8        ; 7b imm8
static void jl__imm8(EMULATOR *emu);   // jl  imm8        ; 7c imm8
static void jge_imm8(EMULATOR *emu);   // jge imm8        ; 7d imm8
static void jle_imm8(EMULATOR *emu);   // jle imm8        ; 7e imm8
static void jg__imm8(EMULATOR *emu);   // jg  imm8        ; 7f imm8

static void jcc_imm32(EMULATOR *emu);  // jcc imm32       ; 0f [80~8f] imm32

static void mov_eax_imm32(EMULATOR *emu); // mov eax, imm32 ; b8 imm32
static void mov_ecx_imm32(EMULATOR *emu); // mov ecx, imm32 ; b9 imm32
static void mov_edx_imm32(EMULATOR *emu); // mov edx, imm32 ; ba imm32
static void mov_ebx_imm32(EMULATOR *emu); // mov ebx, imm32 ; bb imm32
static void mov_esp_imm32(EMULATOR *emu); // mov esp, imm32 ; bc imm32
static void mov_ebp_imm32(EMULATOR *emu); // mov ebp, imm32 ; bd imm32
static void mov_esi_imm32(EMULATOR *emu); // mov esi, imm32 ; be imm32
static void mov_edi_imm32(EMULATOR *emu); // mov edi, imm32 ; bf imm32

static void mov_R_Mimm( EMULATOR *emu);    // mov R      , [M+imm] ; 8b b([00-10] R   M) imm
static void mov_imm32_eax( EMULATOR *emu); // mov [imm32], eax     ; a3 imm32
static void mov_Mimm_R( EMULATOR *emu);    // mov [M+imm], R       ; 89 b([00-11] R   M) imm
static void mov_Mimm_imm32(EMULATOR *emu); // mov [M+imm], imm32   ; c7 b([00-10] 000 M) imm imm32

static void leave(EMULATOR *emu);          // leave ; c9
static void nop(EMULATOR *emu);            // nop   ; 90
static void hlt(EMULATOR *emu);            // hlt   ; f4
/*}}}*/

// modRM/*{{{*/
static MODRM read_modrm(EMULATOR *emu) {/*{{{*/
  MODRM ret;
  ret.modrm = readmem_next_uint8(emu);
  ret.mod   = modrm_mod( ret.modrm);
  ret.raw_r = modrm_r(   ret.modrm);
  ret.raw_m = modrm_m(   ret.modrm);
  ret.R     = modrm_r32p(ret.modrm, emu);
  ret.M     = modrm_m32p(ret.modrm, emu);
  return ret;
}/*}}}*/
static  uint8_t  modrm_mod(  const uint8_t modrm) {/*{{{*/
  // [m][r][mod] という順序でmodrmに入っている。
  // modを返す関数
  return modrm >> 6;
}/*}}}*/
static  uint8_t  modrm_r(  const uint8_t modrm) {/*{{{*/
  // [m][r][mod] という順序でmodrmに入っている。
  // rを返す関数
  uint8_t r8 = modrm >> 3;
  r8 = r8 & 0x7;
  return r8;
}/*}}}*/
static  uint8_t  modrm_m(  const uint8_t modrm) {/*{{{*/
  // [m][r][mod] という順序でmodrmに入っている。
  // mを返す関数
  uint8_t m8 = modrm;
  m8 = m8 & 0x7;
  return m8;
}/*}}}*/
static uint32_t *modrm_r32p(const uint8_t modrm, const EMULATOR *emu) {/*{{{*/
  // [m][r][mod] という順序でmodrmに入っている。
  // r=0~7がeax~ediに対応
  uint8_t r8 = modrm_r(modrm);
  uint32_t *ret = NULL;
  if      (r8 == 0x0) ret = emu -> eax;
  else if (r8 == 0x1) ret = emu -> ecx;
  else if (r8 == 0x2) ret = emu -> edx;
  else if (r8 == 0x3) ret = emu -> ebx;
  else if (r8 == 0x4) ret = emu -> esp;
  else if (r8 == 0x5) ret = emu -> ebp;
  else if (r8 == 0x6) ret = emu -> esi;
  else if (r8 == 0x7) ret = emu -> edi;
  return ret;
}/*}}}*/
static uint32_t *modrm_m32p(const uint8_t modrm, const EMULATOR *emu) {/*{{{*/
  // [m][r][mod] という順序でmodrmに入っている。
  // m=0~7がeax~ediに対応
  uint8_t m8 = modrm_m(modrm);
  uint32_t *ret = NULL;
  if      (m8 == 0x0) ret = emu -> eax;
  else if (m8 == 0x1) ret = emu -> ecx;
  else if (m8 == 0x2) ret = emu -> edx;
  else if (m8 == 0x3) ret = emu -> ebx;
  else if (m8 == 0x4) ret = emu -> esp;
  else if (m8 == 0x5) ret = emu -> ebp;
  else if (m8 == 0x6) ret = emu -> esi;
  else if (m8 == 0x7) ret = emu -> edi;
  return ret;
}/*}}}*/
/*}}}*/
static void cut_esp_0x24(const uint32_t *eXX, EMULATOR *emu) {/*{{{*/
  // [esp]が出現すると、機械語コードに0x24のimm8が混ざるので削除
  if (eXX == (emu -> esp)) {
    const int8_t should_be_24 = readmem_next_uint8(emu);
    if (should_be_24 != 0x24) EXIT_MSG("Error, [esp] appear and following imm8 is not 0x24\n");
  }
}/*}}}*/
static uint32_t modrm_M_imm_to_addr(const MODRM m, EMULATOR *emu) {/*{{{*/
  assert(m.mod < 3);
  cut_esp_0x24(m.M, emu);
  uint32_t mem;
  if      (m.mod == 0) mem = *m.M;
  else if (m.mod == 1) mem = *m.M + (int8_t)readmem_next_uint8( emu);
  else if (m.mod == 2) mem = *m.M +         readmem_next_uint32(emu);
  else EXIT_MSG("ModRM Error. ModRM's M must be address. Therefore mod should not be 3\n");
  return mem;
}/*}}}*/
static uint32_t modrm_M_imm_to_src( const MODRM m, EMULATOR *emu) {/*{{{*/
  const uint32_t mem = modrm_M_imm_to_addr(m, emu);
  const uint32_t src = readmem_uint32(mem, *emu);
  return src;
}/*}}}*/

void initialize_OPECODE(OPECODE ope[OPECODE_NUM]) {/*{{{*/
  // デフォルトの関数ポインタを not_defined_halt にする/*{{{*/
  for (int i=0; i<OPECODE_NUM; i++) {
    ope[i].mnemonic  = "NOT_DEFINED";
    ope[i].func      = not_defined_halt;
  }/*}}}*/
// calc/*{{{*/
  ope[0x01].mnemonic = "add [M+imm], R";
  ope[0x01].func     = add_Mimm_R;
  ope[0x09].mnemonic = "or  [M+imm], R";
  ope[0x09].func     = or__Mimm_R;
  ope[0x11].mnemonic = "adc [M+imm], R";
  ope[0x11].func     = adc_Mimm_R;
  ope[0x19].mnemonic = "sbb [M+imm], R";
  ope[0x19].func     = sbb_Mimm_R;
  ope[0x21].mnemonic = "and [M+imm], R";
  ope[0x21].func     = and_Mimm_R;
  ope[0x29].mnemonic = "sub [M+imm], R";
  ope[0x29].func     = sub_Mimm_R;
  ope[0x31].mnemonic = "xor [M+imm], R";
  ope[0x31].func     = xor_Mimm_R;
  ope[0x39].mnemonic = "cmp [M+imm], R";
  ope[0x39].func     = cmp_Mimm_R;

  ope[0x03].mnemonic = "add R, [M+imm]";
  ope[0x03].func     = add_R_Mimm;
  ope[0x0b].mnemonic = "or  R, [M+imm]";
  ope[0x0b].func     = or__R_Mimm;
  ope[0x13].mnemonic = "adc R, [M+imm]";
  ope[0x13].func     = adc_R_Mimm;
  ope[0x1b].mnemonic = "sbb R, [M+imm]";
  ope[0x1b].func     = sbb_R_Mimm;
  ope[0x23].mnemonic = "and R, [M+imm]";
  ope[0x23].func     = and_R_Mimm;
  ope[0x2b].mnemonic = "sub R, [M+imm]";
  ope[0x2b].func     = sub_R_Mimm;
  ope[0x33].mnemonic = "xor R, [M+imm]";
  ope[0x33].func     = xor_R_Mimm;
  ope[0x3b].mnemonic = "cmp R, [M+imm]";
  ope[0x3b].func     = cmp_R_Mimm;

  ope[0x05].mnemonic = "add eax, imm32";
  ope[0x05].func     = add_eax_imm32;
  ope[0x0d].mnemonic = "or  eax, imm32";
  ope[0x0d].func     = or__eax_imm32;
  ope[0x15].mnemonic = "adc eax, imm32";
  ope[0x15].func     = adc_eax_imm32;
  ope[0x1d].mnemonic = "sbb eax, imm32";
  ope[0x1d].func     = sbb_eax_imm32;
  ope[0x25].mnemonic = "and eax, imm32";
  ope[0x25].func     = and_eax_imm32;
  ope[0x2d].mnemonic = "sub eax, imm32";
  ope[0x2d].func     = sub_eax_imm32;
  ope[0x35].mnemonic = "xor eax, imm32";
  ope[0x35].func     = xor_eax_imm32;
  ope[0x3d].mnemonic = "cmp eax, imm32";
  ope[0x3d].func     = cmp_eax_imm32;

  ope[0x81].mnemonic = "calc reg, imm32";
  ope[0x81].func     = calR_M_imm32;

  ope[0x83].mnemonic = "calc reg, imm8";
  ope[0x83].func     = calR_M_imm8;
/*}}}*/
  // inc/*{{{*/
  ope[0x40].mnemonic = "inc eax";
  ope[0x40].func     = inc_eax;
  ope[0x41].mnemonic = "inc ecx";
  ope[0x41].func     = inc_ecx;
  ope[0x42].mnemonic = "inc edx";
  ope[0x42].func     = inc_edx;
  ope[0x43].mnemonic = "inc ebx";
  ope[0x43].func     = inc_ebx;
  ope[0x44].mnemonic = "inc esp";
  ope[0x44].func     = inc_esp;
  ope[0x45].mnemonic = "inc ebp";
  ope[0x45].func     = inc_ebp;
  ope[0x46].mnemonic = "inc esi";
  ope[0x46].func     = inc_esi;
  ope[0x47].mnemonic = "inc edi";
  ope[0x47].func     = inc_edi;
/*}}}*/
  // dec/*{{{*/
  ope[0x48].mnemonic = "dec eax";
  ope[0x48].func     = dec_eax;
  ope[0x49].mnemonic = "dec ecx";
  ope[0x49].func     = dec_ecx;
  ope[0x4a].mnemonic = "dec edx";
  ope[0x4a].func     = dec_edx;
  ope[0x4b].mnemonic = "dec ebx";
  ope[0x4b].func     = dec_ebx;
  ope[0x4c].mnemonic = "dec esp";
  ope[0x4c].func     = dec_esp;
  ope[0x4d].mnemonic = "dec ebp";
  ope[0x4d].func     = dec_ebp;
  ope[0x4e].mnemonic = "dec esi";
  ope[0x4e].func     = dec_esi;
  ope[0x4f].mnemonic = "dec edi";
  ope[0x4f].func     = dec_edi;
/*}}}*/
  // push/*{{{*/
  ope[0x50].mnemonic = "push eax";
  ope[0x50].func     = push_eax;
  ope[0x51].mnemonic = "push ecx";
  ope[0x51].func     = push_ecx;
  ope[0x52].mnemonic = "push edx";
  ope[0x52].func     = push_edx;
  ope[0x53].mnemonic = "push ebx";
  ope[0x53].func     = push_ebx;
  ope[0x54].mnemonic = "push esp";
  ope[0x54].func     = push_esp;
  ope[0x55].mnemonic = "push ebp";
  ope[0x55].func     = push_ebp;
  ope[0x56].mnemonic = "push esi";
  ope[0x56].func     = push_esi;
  ope[0x57].mnemonic = "push edi";
  ope[0x57].func     = push_edi;
  ope[0x68].mnemonic = "push imm32";
  ope[0x68].func     = push_imm32;
  ope[0x6a].mnemonic = "push imm8";
  ope[0x6a].func     = push_imm8;
/*}}}*/
  // pop/*{{{*/
  ope[0x58].mnemonic = "pop eax";
  ope[0x58].func     = pop_eax;
  ope[0x59].mnemonic = "pop ecx";
  ope[0x59].func     = pop_ecx;
  ope[0x5a].mnemonic = "pop edx";
  ope[0x5a].func     = pop_edx;
  ope[0x5b].mnemonic = "pop ebx";
  ope[0x5b].func     = pop_ebx;
  ope[0x5c].mnemonic = "pop esp";
  ope[0x5c].func     = pop_esp;
  ope[0x5d].mnemonic = "pop ebp";
  ope[0x5d].func     = pop_ebp;
  ope[0x5e].mnemonic = "pop esi";
  ope[0x5e].func     = pop_esi;
  ope[0x5f].mnemonic = "pop edi";
  ope[0x5f].func     = pop_edi;
/*}}}*/
  // jmp/*{{{*/
  ope[0xe9].mnemonic = "jmp imm32";
  ope[0xe9].func     = jmp_imm32;

  ope[0xeb].mnemonic = "jmp imm8";
  ope[0xeb].func     = jmp_imm8;

  ope[0x70].mnemonic = "jo imm8";
  ope[0x70].func     = jo__imm8;
  ope[0x71].mnemonic = "jno imm8";
  ope[0x71].func     = jno_imm8;
  ope[0x72].mnemonic = "jb imm8";
  ope[0x72].func     = jb__imm8;
  ope[0x73].mnemonic = "jae imm8";
  ope[0x73].func     = jae_imm8;
  ope[0x74].mnemonic = "je imm8";
  ope[0x74].func     = je__imm8;
  ope[0x75].mnemonic = "jne imm8";
  ope[0x75].func     = jne_imm8;
  ope[0x76].mnemonic = "jbe_imm8";
  ope[0x76].func     = jbe_imm8;
  ope[0x77].mnemonic = "ja imm8";
  ope[0x77].func     = ja__imm8;
  ope[0x78].mnemonic = "js imm8";
  ope[0x78].func     = js__imm8;
  ope[0x79].mnemonic = "jns imm8";
  ope[0x79].func     = jns_imm8;
  ope[0x7a].mnemonic = "jp imm8";
  ope[0x7a].func     = jp__imm8;
  ope[0x7b].mnemonic = "jnp imm8";
  ope[0x7b].func     = jnp_imm8;
  ope[0x7c].mnemonic = "jl imm8";
  ope[0x7c].func     = jl__imm8;
  ope[0x7d].mnemonic = "jge imm8";
  ope[0x7d].func     = jge_imm8;
  ope[0x7e].mnemonic = "jle imm8";
  ope[0x7e].func     = jle_imm8;
  ope[0x7f].mnemonic = "jg imm8";
  ope[0x7f].func     = jg__imm8;

  ope[0x0f].mnemonic = "jcc imm32";
  ope[0x0f].func     = jcc_imm32;
/*}}}*/
  // mov/*{{{*/
  ope[0xb8].mnemonic = "mov eax, imm32";
  ope[0xb8].func     = mov_eax_imm32;
  ope[0xb9].mnemonic = "mov ecx, imm32";
  ope[0xb9].func     = mov_ecx_imm32;
  ope[0xba].mnemonic = "mov edx, imm32";
  ope[0xba].func     = mov_edx_imm32;
  ope[0xbb].mnemonic = "mov ebx, imm32";
  ope[0xbb].func     = mov_ebx_imm32;
  ope[0xbc].mnemonic = "mov esp, imm32";
  ope[0xbc].func     = mov_esp_imm32;
  ope[0xbd].mnemonic = "mov ebp, imm32";
  ope[0xbd].func     = mov_ebp_imm32;
  ope[0xbe].mnemonic = "mov esi, imm32";
  ope[0xbe].func     = mov_esi_imm32;
  ope[0xbf].mnemonic = "mov edi, imm32";
  ope[0xbf].func     = mov_edi_imm32;

  ope[0x8b].mnemonic = "mov R, [M+imm]";
  ope[0x8b].func     = mov_R_Mimm;
  ope[0xa3].mnemonic = "mov [imm32], eax";
  ope[0xa3].func     = mov_imm32_eax;
  ope[0x89].mnemonic = "mov [M+imm], R";
  ope[0x89].func     = mov_Mimm_R;
  ope[0xc7].mnemonic = "mov [M+imm], imm32";
  ope[0xc7].func     = mov_Mimm_imm32;
/*}}}*/
  // others/*{{{*/
  ope[0xe8].mnemonic = "call imm32";
  ope[0xe8].func     = call_imm32;

  ope[0xc3].mnemonic = "ret";
  ope[0xc3].func     = ret_near;

  ope[0xc9].mnemonic = "leave";
  ope[0xc9].func     = leave;

  ope[0x90].mnemonic = "nop";
  ope[0x90].func     = nop;

  ope[0xf4].mnemonic = "hlt";
  ope[0xf4].func     = hlt;
/*}}}*/
}/*}}}*/
static void not_defined_halt(EMULATOR *emu) {/*{{{*/
  EXIT_MSG("opecode NOT DEFINED. halt\n");
}/*}}}*/

// calc/*{{{*/
static void add_Mimm_R( EMULATOR *emu){/*{{{*/
  MODRM m = read_modrm(emu);

  if        (m.mod < 3) {
    // add [M+imm], R ; 01 b([00~10] R M) none/imm8/imm32 ---- 逆注意
    const uint32_t mem = modrm_M_imm_to_addr(m, emu);
    const uint32_t src = readmem_uint32(mem, *emu);
    update_flag(src, *m.R, emu);
    writemem_uint32( mem, src + *m.R, emu);

  } else if (m.mod == 3) {
    // add M, R ; 01 b(11 R M)
    update_flag(*m.M, *m.R, emu);
    *m.M += *m.R;
  }
}/*}}}*/
static void or__Mimm_R( EMULATOR *emu){/*{{{*/
  MODRM m = read_modrm(emu);

  if (m.mod < 3) {
    // or  [M+imm], R ; 01 b([00~10] R M) none/imm8/imm32 ---- 逆注意
    const uint32_t mem = modrm_M_imm_to_addr(m, emu);
    const uint32_t src = readmem_uint32(mem, *emu);
    writemem_uint32( mem, (src | *m.R), emu);

  } else *m.M = (*m.M | *m.R); // or  M, R ; 01 b(11 R M)
}/*}}}*/
static void adc_Mimm_R( EMULATOR *emu){/*{{{*/
  EXIT_MSG("adc function is not supported\n");
}/*}}}*/
static void sbb_Mimm_R( EMULATOR *emu){/*{{{*/
  EXIT_MSG("sbb function is not supported\n");
}/*}}}*/
static void and_Mimm_R( EMULATOR *emu){/*{{{*/
  MODRM m = read_modrm(emu);

  if (m.mod < 3) {
    // and [M+imm], R ; 01 b([00~10] R M) none/imm8/imm32 ---- 逆注意
    const uint32_t mem = modrm_M_imm_to_addr(m, emu);
    const uint32_t src = readmem_uint32(mem, *emu);
    writemem_uint32( mem, (src & *m.R), emu);

  } else *m.M = (*m.M & *m.R); // and M, R ; 01 b(11 R M)
}/*}}}*/
static void sub_Mimm_R( EMULATOR *emu){/*{{{*/
  MODRM m = read_modrm(emu);

  uint32_t minus_mR = neg_uint32(*m.R);
  if        (m.mod < 3) {
    // sub [M+imm], R ; 01 b([00~10] R M) none/imm8/imm32 ---- 逆注意
    const uint32_t mem = modrm_M_imm_to_addr(m, emu);
    const uint32_t src = readmem_uint32(mem, *emu);
    update_flag(src, minus_mR, emu);
    writemem_uint32( mem, src + minus_mR, emu);

  } else if (m.mod == 3) {
    // sub M, R ; 01 b(11 R M)
    update_flag(*m.M, minus_mR, emu);
    *m.M += minus_mR;
  }
}/*}}}*/
static void xor_Mimm_R( EMULATOR *emu){/*{{{*/
  MODRM m = read_modrm(emu);

  if (m.mod < 3) {
    // xor [M+imm], R ; 01 b([00~10] R M) none/imm8/imm32 ---- 逆注意
    const uint32_t mem = modrm_M_imm_to_addr(m, emu);
    const uint32_t src = readmem_uint32(mem, *emu);
    writemem_uint32( mem, (src ^ *m.R), emu);

  } else *m.M = (*m.M ^ *m.R); // xor M, R ; 01 b(11 R M)
}/*}}}*/
static void cmp_Mimm_R( EMULATOR *emu){/*{{{*/
  MODRM m = read_modrm(emu);

  uint32_t minus_mR = neg_uint32(*m.R);
  if (m.mod < 3) {
    // cmp [M+imm], R ; 01 b([00~10] R M) none/imm8/imm32 ---- 逆注意
    const uint32_t mem = modrm_M_imm_to_addr(m, emu);
    const uint32_t src = readmem_uint32(mem, *emu);
    update_flag(src, minus_mR, emu);

  } else update_flag(*m.M, minus_mR, emu); // cmp M, R ; 01 b(11 R M)
}/*}}}*/

static void add_R_Mimm( EMULATOR *emu) {/*{{{*/
  // add R, [M+imm] ; 03 b([00~10] R M) imm
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  update_flag(*m.R, src, emu);
  *m.R += src;
}/*}}}*/
static void or__R_Mimm( EMULATOR *emu) {/*{{{*/
  // or  R, [M+imm] ; 03 b([00~10] R M) imm
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  *m.R = (*m.R | src);
}/*}}}*/
static void adc_R_Mimm( EMULATOR *emu) {/*{{{*/
  // adc R, [M+imm] ; 13 b([00~10] R M) imm
  EXIT_MSG("adc is not supported\n");
}/*}}}*/
static void sbb_R_Mimm( EMULATOR *emu) {/*{{{*/
  // sbb R, [M+imm] ; 1b b([00~10] R M) imm
  EXIT_MSG("sbb is not supported\n");
}/*}}}*/
static void and_R_Mimm( EMULATOR *emu) {/*{{{*/
  // and R, [M+imm] ; 03 b([00~10] R M) imm
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  *m.R = (*m.R & src);
}/*}}}*/
static void sub_R_Mimm( EMULATOR *emu) {/*{{{*/
  // sub R, [M+imm] ; 03 b([00~10] R M) imm
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  update_flag(*m.R, neg_uint32(src), emu);
  *m.R += neg_uint32(src);
}/*}}}*/
static void xor_R_Mimm( EMULATOR *emu) {/*{{{*/
  // xor R, [M+imm] ; 03 b([00~10] R M) imm
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  *m.R = (*m.R ^ src);
}/*}}}*/
static void cmp_R_Mimm( EMULATOR *emu) {/*{{{*/
  // cmp R, [M+imm] ; 03 b([00~10] R M) imm
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  update_flag(*m.R, neg_uint32(src), emu);
}/*}}}*/

static void add_eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  update_flag(*(emu -> eax), imm32, emu);
  *(emu -> eax) += imm32;
}/*}}}*/
static void or__eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> eax) = *(emu -> eax) | imm32;
}/*}}}*/
static void adc_eax_imm32(EMULATOR *emu) {/*{{{*/
  EXIT_MSG("adc is not supported\n");
}/*}}}*/
static void sbb_eax_imm32(EMULATOR *emu) {/*{{{*/
  EXIT_MSG("sbb is not supported\n");
}/*}}}*/
static void and_eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> eax) = *(emu -> eax) & imm32;
}/*}}}*/
static void sub_eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  uint32_t minus = neg_uint32(imm32);
  update_flag(*(emu -> eax), minus, emu);
  *(emu -> eax) += minus;
}/*}}}*/
static void xor_eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> eax) = *(emu -> eax) ^ imm32;
}/*}}}*/
static void cmp_eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  uint32_t minus = neg_uint32(imm32);
  update_flag(*(emu -> eax), minus, emu);
}/*}}}*/

static void calR_M_imm32(EMULATOR *emu) {/*{{{*/
  // R M, imm32 ; 81 b(11 R M) imm32
  // Rは[000:add] ~ [111:cmp]
  // (M != eax)

  MODRM m = read_modrm(emu);
  assert(m.mod   == 3); // modは11に固定
  assert(m.raw_m != 0); // Mはeax以外
  uint32_t imm32 = readmem_next_uint32(emu);

  if        (m.raw_r == 0 /*add*/) { update_flag(*m.M, imm32, emu)            ;*m.M += imm32;}
  else if   (m.raw_r == 1 /*or */) { *m.M = (*m.M | imm32)                    ;}
  else if   (m.raw_r == 2 /*adc*/) { EXIT_MSG("adc is not supported\n")       ;}
  else if   (m.raw_r == 3 /*sbb*/) { EXIT_MSG("sbb is not supported\n")       ;}
  else if   (m.raw_r == 4 /*and*/) { *m.M = (*m.M & imm32)                    ;}
  else if   (m.raw_r == 5 /*sub*/) { update_flag(*m.M, neg_uint32(imm32), emu);*m.M += neg_uint32(imm32);}
  else if   (m.raw_r == 6 /*xor*/) { *m.M = (*m.M ^ imm32)                    ;}
  else if   (m.raw_r == 7 /*cmp*/) { update_flag(*m.M, neg_uint32(imm32), emu);}
}/*}}}*/
static void calR_M_imm8(EMULATOR *emu) {/*{{{*/
  // R M, imm8 ; 81 b(11 R M) imm32
  // Rは[000:add] ~ [111:cmp]

  MODRM m = read_modrm(emu);
  assert(m.mod   == 3); // modは11に固定
  int8_t imm8 = readmem_next_uint8(emu);

  if        (m.raw_r == 0 /*add*/) { update_flag(*m.M, (int32_t)imm8, emu) ;*m.M +=  imm8;}
  else if   (m.raw_r == 1 /*or */) { *m.M = (*m.M | imm8)                                ;}
  else if   (m.raw_r == 2 /*adc*/) { EXIT_MSG("adc is not supported\n")                  ;}
  else if   (m.raw_r == 3 /*sbb*/) { EXIT_MSG("sbb is not supported\n")                  ;}
  else if   (m.raw_r == 4 /*and*/) { *m.M = (*m.M & imm8)                                ;}
  else if   (m.raw_r == 5 /*sub*/) { update_flag(*m.M, -(int32_t)imm8, emu);*m.M += -imm8;}
  else if   (m.raw_r == 6 /*xor*/) { *m.M = (*m.M ^ imm8)                                ;}
  else if   (m.raw_r == 7 /*cmp*/) { update_flag(*m.M, -(int32_t)imm8, emu)              ;}
}/*}}}*/
/*}}}*/
// inc/*{{{*/
static void inc_eax(EMULATOR *emu) {/*{{{*/
  // inc  eax ; 40
  *(emu -> eax) += 1;
}/*}}}*/
static void inc_ecx(EMULATOR *emu) {/*{{{*/
  // inc  ecx ; 41
  *(emu -> ecx) += 1;
}/*}}}*/
static void inc_edx(EMULATOR *emu) {/*{{{*/
  // inc  edx ; 42
  *(emu -> edx) += 1;
}/*}}}*/
static void inc_ebx(EMULATOR *emu) {/*{{{*/
  // inc  ebx ; 43
  *(emu -> ebx) += 1;
}/*}}}*/
static void inc_esp(EMULATOR *emu) {/*{{{*/
  // inc  esp ; 44
  *(emu -> esp) += 1;
}/*}}}*/
static void inc_ebp(EMULATOR *emu) {/*{{{*/
  // inc  ebp ; 45
  *(emu -> ebp) += 1;
}/*}}}*/
static void inc_esi(EMULATOR *emu) {/*{{{*/
  // inc  esi ; 46
  *(emu -> esi) += 1;
}/*}}}*/
static void inc_edi(EMULATOR *emu) {/*{{{*/
  // inc  edi ; 47
  *(emu -> edi) += 1;
}/*}}}*/
/*}}}*/
// dec/*{{{*/
static void dec_eax(EMULATOR *emu) {/*{{{*/
  // dec  eax ; 48
  *(emu -> eax) -= 1;
}/*}}}*/
static void dec_ecx(EMULATOR *emu) {/*{{{*/
  // dec  ecx ; 49
  *(emu -> ecx) -= 1;
}/*}}}*/
static void dec_edx(EMULATOR *emu) {/*{{{*/
  // dec  edx ; 4a
  *(emu -> edx) -= 1;
}/*}}}*/
static void dec_ebx(EMULATOR *emu) {/*{{{*/
  // dec  ebx ; 4b
  *(emu -> ebx) -= 1;
}/*}}}*/
static void dec_esp(EMULATOR *emu) {/*{{{*/
  // dec  esp ; 4c
  *(emu -> esp) -= 1;
}/*}}}*/
static void dec_ebp(EMULATOR *emu) {/*{{{*/
  // dec  ebp ; 4d
  *(emu -> ebp) -= 1;
}/*}}}*/
static void dec_esi(EMULATOR *emu) {/*{{{*/
  // dec  esi ; 4e
  *(emu -> esi) -= 1;
}/*}}}*/
static void dec_edi(EMULATOR *emu) {/*{{{*/
  // dec  edi ; 4f
  *(emu -> edi) -= 1;
}/*}}}*/
/*}}}*/
// push/*{{{*/
static void push_eax(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> eax), emu);
}/*}}}*/
static void push_ecx(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> ecx), emu);
}/*}}}*/
static void push_edx(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> edx), emu);
}/*}}}*/
static void push_ebx(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> ebx), emu);
}/*}}}*/
static void push_esp(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> esp), emu);
}/*}}}*/
static void push_ebp(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> ebp), emu);
}/*}}}*/
static void push_esi(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> esi), emu);
}/*}}}*/
static void push_edi(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> edi), emu);
}/*}}}*/
static void push_imm32(EMULATOR *emu) {/*{{{*/
  // push 0x12345678 ; 68 78 56 34 12
  *(emu -> esp) -= 4;
  const uint32_t imm32 = readmem_next_uint32(emu);
  writemem_uint32( *(emu -> esp), imm32, emu);
}/*}}}*/
static void push_imm8(EMULATOR *emu) {/*{{{*/
  // push 0x12 ; 6a 12
  *(emu -> esp) -= 4;
  const int8_t imm8 = readmem_next_uint8(emu);
  const int32_t imm32 = imm8;
  writemem_uint32( *(emu -> esp), imm32, emu);
}/*}}}*/
/*}}}*/
// pop/*{{{*/
static void pop_eax( EMULATOR *emu) {/*{{{*/
  *(emu -> eax)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_ecx( EMULATOR *emu) {/*{{{*/
  *(emu -> ecx)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_edx( EMULATOR *emu) {/*{{{*/
  *(emu -> edx)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_ebx( EMULATOR *emu) {/*{{{*/
  *(emu -> ebx)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_esp( EMULATOR *emu) {/*{{{*/
  *(emu -> esp)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_ebp( EMULATOR *emu) {/*{{{*/
  *(emu -> ebp)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_esi( EMULATOR *emu) {/*{{{*/
  *(emu -> esi)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void pop_edi( EMULATOR *emu) {/*{{{*/
  *(emu -> edi)  = readmem_uint32(  *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
/*}}}*/
// jmp/*{{{*/
static void jmp_imm32(EMULATOR *emu) {/*{{{*/
  // jmp  0x5 ; e8 00 00 00 00 (普通に命令更新した後のeipからの差分をimmで与える)
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> eip) += imm32;
}/*}}}*/
static void jmp_imm8(EMULATOR *emu) {/*{{{*/
  // jmp  0x5 ; e8 00 00 00 00 (普通に命令更新した後のeipからの差分をimmで与える)
  int8_t imm8 = readmem_next_uint8(emu);
  *(emu -> eip) += imm8;
}/*}}}*/

static void jo__imm8(EMULATOR *emu) {/*{{{*/
  //70 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_OVERFLOW, *emu) == 1) *(emu -> eip) += imm8;
}/*}}}*/
static void jno_imm8(EMULATOR *emu) {/*{{{*/
  //71 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_OVERFLOW, *emu) == 0) *(emu -> eip) += imm8;
}/*}}}*/
static void jb__imm8(EMULATOR *emu) {/*{{{*/
  //72 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_CARRY, *emu) == 1) *(emu -> eip) += imm8;
}/*}}}*/
static void jae_imm8(EMULATOR *emu) {/*{{{*/
  //73 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_CARRY, *emu) == 0) *(emu -> eip) += imm8;
}/*}}}*/
static void je__imm8(EMULATOR *emu) {/*{{{*/
  //74 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_ZERO, *emu) == 1) *(emu -> eip) += imm8;
}/*}}}*/
static void jne_imm8(EMULATOR *emu) {/*{{{*/
  //75 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_ZERO, *emu) == 0) *(emu -> eip) += imm8;
}/*}}}*/
static void jbe_imm8(EMULATOR *emu) {/*{{{*/
  //76 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_CARRY, *emu) == 1 || read_flag(FLAG_NAME_ZERO, *emu) == 1) *(emu -> eip) += imm8;
}/*}}}*/
static void ja__imm8(EMULATOR *emu) {/*{{{*/
  //77 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_CARRY, *emu) == 0 && read_flag(FLAG_NAME_ZERO, *emu) == 0) *(emu -> eip) += imm8;
}/*}}}*/
static void js__imm8(EMULATOR *emu) {/*{{{*/
  //78 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_SIGN, *emu) == 1) *(emu -> eip) += imm8;
}/*}}}*/
static void jns_imm8(EMULATOR *emu) {/*{{{*/
  //79 ff
  const int8_t imm8 = readmem_next_uint8(emu);
  if (read_flag(FLAG_NAME_SIGN, *emu) == 0) *(emu -> eip) += imm8;
}/*}}}*/
static void jp__imm8(EMULATOR *emu) {/*{{{*/
  //7a ff
  EXIT_MSG("jp Not Supported\n");
}/*}}}*/
static void jnp_imm8(EMULATOR *emu) {/*{{{*/
  //7b ff
  EXIT_MSG("jnp Not Supported\n");
}/*}}}*/
static void jl__imm8(EMULATOR *emu) {/*{{{*/
  //7c ff
  const int8_t imm8 = readmem_next_uint8(emu);
  int SF = read_flag(FLAG_NAME_SIGN    , *emu);
  int OF = read_flag(FLAG_NAME_OVERFLOW, *emu);
  if (SF != OF) *(emu -> eip) += imm8;
}/*}}}*/
static void jge_imm8(EMULATOR *emu) {/*{{{*/
  //7d ff
  const int8_t imm8 = readmem_next_uint8(emu);
  int SF = read_flag(FLAG_NAME_SIGN    , *emu);
  int OF = read_flag(FLAG_NAME_OVERFLOW, *emu);
  if (SF == OF) *(emu -> eip) += imm8;
}/*}}}*/
static void jle_imm8(EMULATOR *emu) {/*{{{*/
  //7e ff
  const int8_t imm8 = readmem_next_uint8(emu);
  int ZF = read_flag(FLAG_NAME_ZERO    , *emu);
  int SF = read_flag(FLAG_NAME_SIGN    , *emu);
  int OF = read_flag(FLAG_NAME_OVERFLOW, *emu);
  if ((ZF == 1) || (SF != OF)) *(emu -> eip) += imm8;
}/*}}}*/
static void jg__imm8(EMULATOR *emu) {/*{{{*/
  //7f ff
  const int8_t imm8 = readmem_next_uint8(emu);
  int ZF = read_flag(FLAG_NAME_ZERO    , *emu);
  int SF = read_flag(FLAG_NAME_SIGN    , *emu);
  int OF = read_flag(FLAG_NAME_OVERFLOW, *emu);
  if ((ZF != 1) && (SF == OF)) *(emu -> eip) += imm8;
}/*}}}*/

static void jcc_imm32(EMULATOR *emu) {/*{{{*/
  const uint8_t  imm8  = readmem_next_uint8(emu);
  const uint32_t imm32 = readmem_next_uint32(emu);

  int ZF = read_flag(FLAG_NAME_ZERO    , *emu);
  int SF = read_flag(FLAG_NAME_SIGN    , *emu);
  int OF = read_flag(FLAG_NAME_OVERFLOW, *emu);
  int CF = read_flag(FLAG_NAME_CARRY   , *emu);

  bool flag = false;
  if      (imm8 == 0x80 /*jo */) {flag = (OF == 1);}
  else if (imm8 == 0x81 /*jno*/) {flag = (OF == 0);}
  else if (imm8 == 0x82 /*jb */) {flag = (CF == 1);}
  else if (imm8 == 0x83 /*jae*/) {flag = (CF == 0);}
  else if (imm8 == 0x84 /*je */) {flag = (ZF == 1);}
  else if (imm8 == 0x85 /*jne*/) {flag = (ZF == 0);}
  else if (imm8 == 0x86 /*jbe*/) {flag = ((CF == 1) || (ZF == 1));}
  else if (imm8 == 0x87 /*ja */) {flag = ((CF == 0) && (ZF == 0));}
  else if (imm8 == 0x88 /*js */) {flag = (SF == 0);}
  else if (imm8 == 0x89 /*jns*/) {flag = (SF == 0);}
  else if (imm8 == 0x8a /*jp */) {EXIT_MSG("jp Not Supported\n");}
  else if (imm8 == 0x8b /*jnp*/) {EXIT_MSG("jnp Not Supported\n");}
  else if (imm8 == 0x8c /*jl */) {flag = (SF != OF);}
  else if (imm8 == 0x8d /*jge*/) {flag = (SF == OF);}
  else if (imm8 == 0x8e /*jle*/) {flag = ((SF != OF) || (ZF == 1));}
  else if (imm8 == 0x8f /*jg */) {flag = ((SF == OF) && (ZF != 1));}
  else EXIT_MSG("Not supported instruct\n");

  if (flag) *(emu -> eip) += imm32;
}/*}}}*/
/*}}}*/
// mov/*{{{*/
static void mov_eax_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> eax)  = imm32;
}/*}}}*/
static void mov_ecx_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> ecx)  = imm32;
}/*}}}*/
static void mov_edx_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> edx)  = imm32;
}/*}}}*/
static void mov_ebx_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> ebx)  = imm32;
}/*}}}*/
static void mov_esp_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> esp)  = imm32;
}/*}}}*/
static void mov_ebp_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> ebp)  = imm32;
}/*}}}*/
static void mov_esi_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> esi)  = imm32;
}/*}}}*/
static void mov_edi_imm32(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> edi)  = imm32;
}/*}}}*/

static void mov_R_Mimm(EMULATOR *emu) {/*{{{*/
  MODRM    m   = read_modrm(emu);
  uint32_t src = modrm_M_imm_to_src(m, emu);
  *m.R = src;
}/*}}}*/
static void mov_imm32_eax(EMULATOR *emu) {/*{{{*/
  uint32_t imm32 = readmem_next_uint32(emu);
  writemem_uint32(imm32, *(emu -> eax), emu);
}/*}}}*/
static void mov_Mimm_R(EMULATOR *emu) {/*{{{*/
  MODRM m = read_modrm(emu);
  if (m.mod < 3) {
    uint32_t mem = modrm_M_imm_to_addr(m, emu);
    writemem_uint32(mem, *m.R, emu);
  }
  else *m.M = *m.R;
}/*}}}*/
static void mov_Mimm_imm32(EMULATOR *emu) {/*{{{*/
  MODRM m = read_modrm(emu);
  assert(m.raw_r == 0);
  uint32_t mem   = modrm_M_imm_to_addr(m, emu);
  uint32_t imm32 = readmem_next_uint32(emu);
  writemem_uint32(mem, imm32, emu);
}/*}}}*/
/*}}}*/
// others/*{{{*/
static void call_imm32(EMULATOR *emu) {/*{{{*/
// push eip した後に eip=imm32 する命令
// call命令の書かれた(つまりe8の置かれた)アドレスからの相対値で次回のeipを定義
//
// 例1 次回の命令文に飛ぶ場合(NOP NOP NOPと等価)
// call 0x05 ; e8 00 00 00 00 ; eip=0x00 -> eip=(0x05+0)
// call 0x0a ; e8 00 00 00 00 ; eip=0x05 -> eip=(0x0a+0)
// call 0x0f ; e8 00 00 00 00 ; eip=0x0f -> eip=(0x14+0)
// 冒頭の 0x05 は次回の命令(call 0x0a)が格納されたメモリアドレス指すので、差分は00 00 00 00となる
//
// 例2
// call 0x04 ; e8 ff ff ff ff ; eip=0x00 -> eip=(0x05-1)
//
// 例3
// call 0x06 ; e8 01 00 00 00 ; eip=0x00 -> eip=(0x05+1)

  uint32_t imm32 = readmem_next_uint32(emu);
  *(emu -> esp) -= 4;
  writemem_uint32( *(emu -> esp), *(emu -> eip), emu);
  *(emu -> eip) += imm32;
}/*}}}*/
static void ret_near(EMULATOR *emu) {/*{{{*/
// pop eip
  *(emu -> eip) = readmem_uint32( *(emu -> esp), *emu);
  *(emu -> esp) += 4;
}/*}}}*/
static void leave(EMULATOR *emu) {/*{{{*/
  *(emu -> esp) = *(emu -> ebp);
  pop_ebp(emu);
}/*}}}*/
static void nop(EMULATOR *emu) {/*{{{*/
}/*}}}*/
static void hlt(EMULATOR *emu) {/*{{{*/
}/*}}}*/
/*}}}*/
