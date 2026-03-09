#pragma once

#define FB_MEMORYARENA_ALLOC_ADDRESS CYPRESS_GW_SELECT(0x140389540, 0x140155630, 0x140416050)
#define FB_MEMORYARENA_FREE_ADDRESS CYPRESS_GW_SELECT(0x140389FC0, 0x140155740, 0x140417740)
#define FB_MEMORYARENA_FINDARENAFOROBJECT_ADDRESS CYPRESS_GW_SELECT(0x140389E70, 0x1401CA550, 0x14042FE10)

namespace fb
{
    class MemoryArena {
    public:
        static MemoryArena* findArenaForObject(void* ptr) {
            using tFindArena = MemoryArena * (*)(void*, bool);
            auto findArena = reinterpret_cast<tFindArena>(FB_MEMORYARENA_FINDARENAFOROBJECT_ADDRESS);

            return findArena(ptr, true);
        }

        void* alloc(size_t size, int align = 8) {
            using tArenaAlloc = void* (*)(MemoryArena*, size_t, int);
            auto arenaAlloc = reinterpret_cast<tArenaAlloc>(FB_MEMORYARENA_ALLOC_ADDRESS);

            return arenaAlloc(this, static_cast<int>(size), align);
        }

        static void* alloc(void* parent, size_t size, int align = 8) {
            auto memArena = findArenaForObject(parent);

            return memArena->alloc(size, align);
        }

        static void free(void* ptr) {
            using tArenaFree = void(*)(MemoryArena*, void*);
            auto arenaFree = reinterpret_cast<tArenaFree>(FB_MEMORYARENA_FREE_ADDRESS);

            arenaFree(findArenaForObject(ptr), ptr);
        }
    };
}