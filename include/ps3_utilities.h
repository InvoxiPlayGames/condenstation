#include <stdint.h>

void HookFunction(unsigned int OriginalAddress, void *StubFunction, void *NewFunction);
void UnhookFunction(unsigned int *OriginalAddress, unsigned int *StubFunction);

// utilities for working with PS3
typedef struct _written_memory_patch
{
    uint32_t address;
    uint32_t value;
} written_memory_patch;
typedef struct _toc_stub {
    uint32_t func;
    uint32_t toc;
} toc_stub;
char PS3_MemoryWriteCheck();
void PS3_Write32(uint32_t address, uint32_t value);
void PS3_WriteMemory(uint32_t address, void *data, size_t size);
void PS3_SetPluginTOCBase(uint32_t toc);
uint32_t PS3_CreateTrampoline(uint32_t target_func);
void PS3_PokeBranch(uint32_t address, uint32_t replacement, char linked);
#define POKE_PLUGIN_B(addr, dest) PS3_PokeBranch(addr, dest, 0)
#define POKE_PLUGIN_BL(addr, dest) PS3_PokeBranch(addr, dest, 1)
#undef POKE_32
#define POKE_32 PS3_Write32

#define FUNC_PTR(x) (uint32_t)(((uint32_t *)x)[0])
#define TOC_PTR(x) (uint32_t)(((uint32_t *)x)[1])

#define POKE_STUB(addr, dest) POKE_B(FUNC_PTR(addr), dest)
