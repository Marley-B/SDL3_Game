#pragma once
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h> 

// SDL components coordination
struct SDLState {
	SDL_Window* window;
	SDL_Renderer* renderer;
	MIX_Mixer* mixer = nullptr;
	int width, height, logW, logH;
	const bool* keys;
	bool fullscreen;

	SDLState() : keys(SDL_GetKeyboardState(nullptr)) {
		fullscreen = false;
	}
};