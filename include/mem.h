uint32_t readmem_uint32(uint32_t addr, const EMULATOR emu);
uint8_t  readmem_uint8( uint32_t addr, const EMULATOR emu);
uint32_t readmem_next_uint32(EMULATOR *emu);
uint8_t  readmem_next_uint8( EMULATOR *emu);
void writemem_uint32(uint32_t addr, const uint32_t src, EMULATOR *emu);
void writemem_uint8( uint32_t addr, const uint8_t  src, EMULATOR *emu);

void print_memory(const uint8_t memory[MEMORY_COUNT], const int bin_size, const int esp);
