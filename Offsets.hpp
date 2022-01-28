#include <Windows.h>
#include <cstdint>
#include <string> 

#define aslr(x) (x - 0x400000 + reinterpret_cast<DWORD>(GetModuleHandle(0)))
#define r_sizeclass(sz) (std::size_t((sz)-1) < r_kMaxSmallSize ? r_kSizeClassConfig.r_classForSize[sz] : -1)

#define R_LUA_ERRRUN "lua_exception: LUA_ERRRUN (no string/number provided as description)"
#define R_LUA_ERRSYNTAX "lua_exception: LUA_ERRSYNTAX (no string/number provided as description)"
#define R_LUA_ERRMEM "lua_exception: not enough memory"
#define R_LUA_ERRERR "lua_exception: error in error handling"

/*
** lua casting, we're gonna yoink this
*/
#ifndef r_cast_to
#define r_cast_to(t, cast) ((t)(cast))
#endif

#define r_cast_byte(i) r_cast_to(std::uint8_t, (i))
#define r_cast_num(i) r_cast_to(double, (i))
#define r_cast_int(i) r_cast_to(int, (i))

/*
** lua memory shortcuts
*/
#define r_luaM_new(L, t, size, memcat) r_cast_to(t*, r_luaM_new_(L, size, memcat))
#define r_luaM_free(L, p, size, memcat) r_luaM_free_(L, (p), size, memcat)

/*
** luau's sizeclass configuration, we're gonna yoink this
*/
const std::size_t r_kSizeClasses = 32;
const std::size_t r_kMaxSmallSize = 512;
const std::size_t r_kPageSize = 16 * 1024 - 24;
const std::size_t r_kBlockHeader = sizeof(double) > sizeof(void*) ? sizeof(double) : sizeof(void*);

struct r_SizeClassConfig
{
    int r_sizeOfClass[r_kSizeClasses];
    std::int8_t r_classForSize[r_kMaxSmallSize + 1];
    int r_classCount = 0;

    r_SizeClassConfig()
    {
        std::memset(r_sizeOfClass, 0, sizeof(r_sizeOfClass));
        std::memset(r_classForSize, -1, sizeof(r_classForSize));

        for (auto size = 8; size < 64; size += 8)
            r_sizeOfClass[r_classCount++] = size;

        for (auto size = 64; size < 256; size += 16)
            r_sizeOfClass[r_classCount++] = size;

        for (auto size = 256; size <= 512; size += 32)
            r_sizeOfClass[r_classCount++] = size;

        for (auto klass = 0; klass < r_classCount; ++klass)
            r_classForSize[r_sizeOfClass[klass]] = std::int8_t(klass);

        for (auto size = r_kMaxSmallSize - 1; size >= 0; --size)
            if (r_classForSize[size] < 0)
                r_classForSize[size] = r_classForSize[size + 1];
    }
};

const r_SizeClassConfig r_kSizeClassConfig;

/*
** generic addresses/offsets and obfuscations.
*/
const auto luastate_g = 32;

const auto globalstate_frealloc = 12;
const auto globalstate_totalbytes = 52;
const auto globalstate_ud = 16;

std::uintptr_t r_G(std::uintptr_t rL)
{
    return rL + luastate_g - *reinterpret_cast<std::uintptr_t*>(rL + luastate_g);
}

typedef void* (__fastcall* T_luaM_newblock)(std::uintptr_t a1, std::uint32_t a2);
T_luaM_newblock r_luaM_newblock = reinterpret_cast<T_luaM_newblock>(aslr(0x1A712C0));

typedef void(__fastcall* T_luaM_freeblock)(std::uintptr_t a1, std::uint32_t a2, void* block);
T_luaM_freeblock r_luaM_freeblock = reinterpret_cast<T_luaM_freeblock>(aslr(0x1A714F0));
