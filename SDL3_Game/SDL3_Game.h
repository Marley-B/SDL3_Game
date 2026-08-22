#pragma once

#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <vector>
#include <string>
#include <array>
#include <format>
#include <filesystem>
#include "game_object.h"
#include "tmx.h"
#include "sdlstate.h"
#include "game_state.h"
#include "resources.h"
#include "object_state.h"
#include "user_events.h"
#include "game_status.h"
#include "level_state.h"
#include "ui.h"

bool initialize(SDLState& state);
void cleanup(SDLState& state);
void initializeLevels(Resources& res);