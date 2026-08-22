// SDL3_Game.cpp : Defines the entry point for the application.
//

// General debugin tips:
//	if (!audio) {
//		SDL_Log("Failed to load audio: %%s - %s", filepath.c_str(), SDL_GetError());
//		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error loading audio", nullptr);
//	}

#include "SDL3_Game.h"

using namespace std;

int main(int argc, char *agrc[])
{
	SDLState state;
	GameObject obj;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 360;
	
	if (!initialize(state)) {
		return 1;
	}

	// load game assets
	Resources res;
	res.load(state);

	// setup game data
	GameState gs(state);
	uint64_t prevTime = SDL_GetTicks();

	// MIX_SetTrackGain(res.musicMain, 0.5f);
	// MIX_PlayTrack(res.musicMain, -1);

	//Button button = Button(res);
	initializeLevels(res);
	gs.currentStateGame->enter(state, gs, res);

	state.text = new TextRenderer(res);

	int prevWidth = state.width;
	int prevHeight = state.height;

	// start the game loop
	bool running = true;
	while (running) {
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds
		if (deltaTime > 0.05f) deltaTime = 0.05f; // avoid fisics errors when the game lags
		SDL_Event event{ 0 };
		SDL_GetWindowSize(state.window, &state.width, &state.height);
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_EVENT_QUIT: {
					running = false;
					break;
				}
				case SDL_EVENT_WINDOW_RESIZED: {
					state.width = event.window.data1;
					state.height = event.window.data2;
					break;
				}
				case SDL_EVENT_KEY_DOWN: {
					if (event.key.scancode == SDL_SCANCODE_0) {
						SDL_Event event{ UserEvents::PLAYER_DEATH };
						SDL_PushEvent(&event);
					}
					else if ((event.key.scancode == SDL_SCANCODE_9)) {
						SDL_Event event{ UserEvents::PLAYER_WIN };
						SDL_PushEvent(&event);
					}
					break;
				}
				case SDL_EVENT_KEY_UP: {
					if (event.key.scancode == SDL_SCANCODE_F12) {
						gs.debugMode = !gs.debugMode;
					}
					else if (event.key.scancode == SDL_SCANCODE_F11) {
						state.fullscreen = !state.fullscreen;
						SDL_SetWindowFullscreen(state.window, state.fullscreen);
					}
					break;
				}
			}

			gs.currentStateGame->handleEvent(event, gs, res, state, obj);  
		}

		gs.currentStateGame->update(state, gs, res, obj, deltaTime);
		gs.currentStateGame->render(state, gs, res, obj, deltaTime);

		//sSDL_Log("Window w: %d, Window h: %d", state.width, state.height);

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
	}
	res.unload();
	cleanup(state);
	return 0;
}

void initializeLevels(Resources& res) {
	if (Level::sLevels.empty()) {
		Level::sLevels.push_back(new Level(1, res.map1));
		Level::sLevels.push_back(new Level(2, res.map2));
		Level::sLevels.push_back(new Level(3, res.map1));
		Level::sLevels.push_back(new Level(4, res.map1));
	}
}

bool initialize(SDLState& state) {

	bool initSucces = true;
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL3", nullptr);
		initSucces = false;
	}

	// create window
	state.window = SDL_CreateWindow("Undertale", state.width, state.height, SDL_WINDOW_RESIZABLE);
	if (!state.window) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", nullptr);
		cleanup(state);
		initSucces = false;
	}

	// create the renderer
	state.renderer = SDL_CreateRenderer(state.window, nullptr);
	if (!state.renderer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", nullptr);
		cleanup(state);
		initSucces = false;
	}
	SDL_SetRenderVSync(state.renderer, 1);

	// configure presentation
	SDL_SetRenderLogicalPresentation(state.renderer, state.logW, state.logH, SDL_LOGICAL_PRESENTATION_LETTERBOX);

	if (!MIX_Init()) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL_mixer", nullptr);
		cleanup(state);
		initSucces = false;
	}

	// initialize the SDL_mixer audio pointer
	state.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	if (!state.mixer) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating audio device", state.window);
		cleanup(state);
		initSucces = false;
	}

	if (!TTF_Init()) {
		SDL_Log("TTF_Init failed: %s", SDL_GetError());
		cleanup(state);
		initSucces = false;
	}

	return initSucces;
}

void cleanup(SDLState& state) {
	if (state.mixer) {
		MIX_DestroyMixer(state.mixer);
	}
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	SDL_Quit();
}