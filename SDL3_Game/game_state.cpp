#include "game_state.h"
#include "object_state.h" 
#include "game_status.h"   
#include "sdlstate.h"  

GameState::GameState(const SDLState& state) {
	playerLayer = -1;
	playerIndex = -1;
	mapViewport = SDL_FRect{
		.x = 0, .y = 0,
		.w = static_cast<float>(state.logW),
		.h = static_cast<float>(state.logH)
	};
	bg2Scroll = bg3Scroll = bg4Scroll = bg5Scroll = 0;
	debugMode = false;
	coinsCollected = 0;
	currentStatePlayer = PlayerIdle::get();;
	currentStateGame = IntroMenu::get();
}