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


bool initialize(SDLState& state);
void cleanup(SDLState& state);
void createTiles(const SDLState& state, GameState& gs, Resources& res);
void drawParalaxBackground(GameState* gs, SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float width, float hight, float deltaTime);
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);
void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& recA, const SDL_FRect& recB,
	const SDL_FRect& recC, GameObject& objA, GameObject& objB, float deltaTime);