#include <Windows.h>
#include <cstdint>
#include <string> 

#include "Offsets.hpp"

/*
** generic allocation routines.
*/
template<typename T>
__inline void* r_luaM_new_(const T rL, const std::size_t nsize, const std::uint8_t memcat)
{
    const auto g = r_GGS(rL);
    const auto ud = *reinterpret_cast<std::uintptr_t*>(g + globalstate_ud);

    auto nclass = r_sizeclass(nsize);

    void* block = nclass >= 0 ? r_luaM_newblock(rL, nclass) : (std::uintptr_t*)(*(std::uint32_t(__cdecl**)(std::uint32_t, const std::uintptr_t, void*, const std::uintptr_t, std::uint32_t))(g + globalstate_frealloc))(rL, ud, NULL, 0, nsize);

    if (block == NULL && nsize > 0)
        throw std::exception(R_LUA_ERRMEM);

    *reinterpret_cast<std::size_t*>(g + globalstate_totalbytes) += nsize;
    *reinterpret_cast<std::uintptr_t*>(g + 4u * memcat + 200) += nsize;

    return block;
}

template<typename T>
__inline void r_luaM_free_(const T rL, void* block, const std::size_t osize, const std::uint8_t memcat)
{
    const auto g = r_GGS(rL);
    const auto ud = *reinterpret_cast<std::uintptr_t*>(g + globalstate_ud);

    auto oclass = r_sizeclass(osize);

    if (oclass >= 0)
        r_luaM_freeblock(rL, oclass, block);
    else
        (std::uintptr_t*)(*(std::uint32_t(__cdecl**)(std::uint32_t, const std::uintptr_t, void*, const std::uintptr_t, std::uint32_t))(g + globalstate_frealloc))(rL, ud, block, osize, 0);

    *reinterpret_cast<std::size_t*>(g + globalstate_totalbytes) -= osize;
    *reinterpret_cast<std::uintptr_t*>(g + 4u * memcat + 200) -= osize;
}

template<typename T>
__inline void* r_luaM_realloc_(const T rL, void* block, const std::size_t osize, const std::size_t nsize, const std::uint8_t memcat)
{
    const auto g = r_GGS(rL);
    const auto ud = *reinterpret_cast<std::uintptr_t*>(g + globalstate_ud);

    auto nclass = r_sizeclass(nsize);
    auto oclass = r_sizeclass(osize);
    void* result;

    if (nclass >= 0 || oclass >= 0)
    {
        result = nclass >= 0 ? r_luaM_newblock(rL, nclass) : (std::uintptr_t*)(*(std::uint32_t(__cdecl**)(std::uint32_t, const std::uintptr_t, void*, const std::uintptr_t, std::uint32_t))(g + globalstate_frealloc))(rL, ud, NULL, 0, nsize);
        if (result == NULL && nsize > 0)
            throw std::exception(R_LUA_ERRMEM);

        if (osize > 0 && nsize > 0)
            std::memcpy(result, block, osize < nsize ? osize : nsize);

        if (oclass >= 0)
            r_luaM_freeblock(rL, oclass, block);
        else
            (std::uintptr_t*)(*(std::uint32_t(__cdecl**)(std::uint32_t, const std::uintptr_t, void*, const std::uintptr_t, std::uint32_t))(g + globalstate_frealloc))(rL, ud, block, osize, 0);
    }
    else
    {
        result = (std::uintptr_t*)(*(std::uint32_t(__cdecl**)(std::uint32_t, const std::uintptr_t, void*, const std::uintptr_t, std::uint32_t))(g + globalstate_frealloc))(rL, ud, block, osize, nsize);

        if (result == NULL && nsize > 0)
            throw std::exception(R_LUA_ERRMEM);
    }

    *reinterpret_cast<std::size_t*>(g + globalstate_totalbytes) = (*reinterpret_cast<const std::size_t*>(g + globalstate_totalbytes) - osize) + nsize;
    *reinterpret_cast<std::uintptr_t*>(g + 4u * memcat + 200) += nsize - osize;
    return result;
}
