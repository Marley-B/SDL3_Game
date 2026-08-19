#pragma once
#include <SDL3/SDL.h>

namespace UserEvents {
    const inline Uint32 STAMINA_DEPLETED{
      SDL_RegisterEvents(1) };

    const inline Uint32 STAMINA_RESTORED{
      SDL_RegisterEvents(1) };

    const inline Uint32 PLAYER_DEATH{
      SDL_RegisterEvents(1) };

    const inline Uint32 PLAYER_WIN{
      SDL_RegisterEvents(1) };
}