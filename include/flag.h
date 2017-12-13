typedef enum {
  FLAG_NAME_CARRY,
  FLAG_NAME_ZERO,
  FLAG_NAME_SIGN,
  FLAG_NAME_OVERFLOW,
} FLAG_NAME;

void update_flag(const uint32_t rnd1, const uint32_t rnd2, EMULATOR *emu);
int read_flag(const int flag_num, const EMULATOR emu);
void print_flag(const EMULATOR emu);
