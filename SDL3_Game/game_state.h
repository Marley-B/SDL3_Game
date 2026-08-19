#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include "game_object.h"

struct SDLState;
class ObjectState;
class GameStatus;
class Level;

// general game components coordination
struct GameState {
	std::vector<std::vector<GameObject>> layers;
	//std::vector<GameObject> coins;
	int playerLayer, playerIndex;
	SDL_FRect mapViewport;
	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;
	//ObjectState* currentStateGame;
	ObjectState* currentStatePlayer;
	GameStatus* currentStateGame;
	GameStatus* currentLevel;
	std::vector<Level*> Level::sLevels;

	GameState(const SDLState& state);
	GameObject& player() { return layers[playerLayer][playerIndex]; }
};