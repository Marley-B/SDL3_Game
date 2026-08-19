// SDL3_Game.cpp : Defines the entry point for the application.
//

// General debugin tips:
//	if (!audio) {
//		SDL_Log("Failed to load audio: %%s - %s", filepath.c_str(), SDL_GetError());
//		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error loading audio", nullptr);
//	}

#include "SDL3_Game.h"

using namespace std;

const int TILE_SIZE = 32;

GameState::GameState(const SDLState& state) {
	playerLayer = -1;
	playerIndex = -1;
	mapViewport = SDL_FRect{
		.x = 0, .y = 0,
		.w = static_cast<float>(state.logW),
		.h = static_cast<float>(state.logH)
	};
	bg2Scroll = bg3Scroll = bg4Scroll = 0;
	debugMode = false;
	currentStatePlayer = PlayerIdle::get();
	currentStateGame = IntroMenu::get();
}


int main(int argc, char *agrc[])
{
	SDLState state;
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
	createTiles(state, gs, res);
	uint64_t prevTime = SDL_GetTicks();

	// MIX_SetTrackGain(res.musicMain, 0.5f);
	// MIX_PlayTrack(res.musicMain, -1);

	Button button = Button(res);
	initializeLevels(res);

	// start the game loop
	bool running = true;
	while (running) {
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds
		if (deltaTime > 0.05f) deltaTime = 0.05f; // avoid fisics errors when the game lags
		SDL_Event event{ 0 };

		while (SDL_PollEvent(&event)) {
			// all events that trigger a change in user state
			if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP || 
				event.type == UserEvents::STAMINA_DEPLETED || event.type == UserEvents::STAMINA_RESTORED) {
				for (auto& layer : gs.layers) {
					for (GameObject& obj : layer) {
						if (obj.dynamic && obj.type == ObjectType::player) {
							gs.currentStatePlayer->handleEvent(event, res, obj, gs);
						}
					}
				}
			}
			if (event.type == SDL_EVENT_MOUSE_MOTION || event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
				button.handleEvent(&event, res);
			}

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
		}

		// update all objects
		for (auto& layer : gs.layers) {
			for (GameObject& obj : layer) {
				if (obj.dynamic) { // No updating background elements
					update(state, gs, res, obj, deltaTime);
				}
			}
		}

		// calculate viewport position 
		gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2  + 30 ;
		gs.mapViewport.y = (gs.player().position.y + TILE_SIZE / 2) - gs.mapViewport.h / 2;


		// draw background images
		SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
		drawParalaxBackground(&gs, state.renderer, res.texBg4, gs.player().velocity.x, gs.bg4Scroll, 0.075f, deltaTime);
		drawParalaxBackground(&gs, state.renderer, res.texBg3, gs.player().velocity.x, gs.bg3Scroll, 0.15f, deltaTime);
		drawParalaxBackground(&gs, state.renderer, res.texBg2, gs.player().velocity.x, gs.bg2Scroll, 0.3f, deltaTime);

		// draw all objects
		for (auto& layer : gs.layers) {
			for (GameObject& obj : layer) {
				if (!obj.invisible) {
					drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
				}
			}    
		}

		button.render(&state);

		if (gs.debugMode == true) {
			// Enhanced debug display with position and viewport info
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5,
				std::format("S: {}, C: {}, St: {}, Pos:({:.1f},{:.1f}), Dir:({},{})",
					typeid(*gs.currentStatePlayer).name(),
					gs.player().data.player.collectedCoins,
					gs.player().data.player.staminaPoints,
					gs.player().position.x,
					gs.player().position.y,
					gs.player().directionH,
					gs.player().directionV
				).c_str());
		}

		// swap buffers and present
		SDL_RenderPresent(state.renderer);
		prevTime = nowTime;
	}
	res.unload();
	cleanup(state);
	return 0;
}

void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
	// update animation
	if (obj.currentAnimation != -1) {
		obj.animations[obj.currentAnimation].step(deltaTime);
	}

	// update the player according to their current state
	if (obj.type == ObjectType::player) {
		gs.currentStatePlayer->update(obj, deltaTime);

		if (obj.data.player.invincible) {
			if (obj.data.player.damagedTimer.step(deltaTime)) {
				obj.data.player.invincible = false;
			}
		}
		else { // decrese stamina with time
			//obj.data.player.staminaPoints += -0.1 * deltaTime;
		}

		if (obj.data.player.staminaPoints <= 0) {
			SDL_Event event{ UserEvents::STAMINA_DEPLETED };
			SDL_PushEvent(&event);
		}
	}

	// handle collision detection 
	for (auto& layer : gs.layers) {
		for (GameObject& objB : layer) {
			if (&obj != &objB) {
				checkCollision(state, gs, res, obj, objB, deltaTime);
			}
		}
	}
}

void initializeLevels(Resources& res) {
	if (Level::sLevels.empty()) {
		Level::sLevels.push_back(new Level(1, res));
		Level::sLevels.push_back(new Level(2, res));
		Level::sLevels.push_back(new Level(3, res));
		Level::sLevels.push_back(new Level(4, res));
	}
}

void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime) {
	// If it has no colliders
	if (a.collider.w == 0 || a.collider.h == 0 || b.collider.w == 0 || b.collider.h == 0) {
		return;
	}

	SDL_FRect recA{
		.x = a.position.x + a.collider.x,
		.y = a.position.y + a.collider.y,
		.w = a.collider.w,
		.h = a.collider.h
	};
	SDL_FRect recB{
		.x = b.position.x + b.collider.x,
		.y = b.position.y + b.collider.y,
		.w = b.collider.w,
		.h = b.collider.h
	};
	SDL_FRect recC{ 0 };

	if (SDL_GetRectIntersectionFloat(&recA, &recB, &recC)) {
		// found intersecction aka colision area
		collisionResponse(state, gs, res, recA, recB, recC, a, b, deltaTime);
	}
}

void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& recA, const SDL_FRect& recB,
	const SDL_FRect& recC, GameObject& objA, GameObject& objB, float deltaTime) {

	// pushes the object back so they stop cliping with what they collided
	const auto genericResponse = [&]() {
		if (recC.w < recC.h) {
			// horizontal colision
			if (objA.velocity.x > 0) { // going right
				objA.position.x -= recC.w;
			}
			else if (objA.velocity.x < 0) { // going left
				objA.position.x += recC.w;
			}
			objA.velocity.x = 0;
		}
		else {
			// vertical collision
			if (objA.velocity.y > 0) { // going down
				objA.position.y -= recC.h;
			}
			else if (objA.velocity.y < 0) { // going up
				objA.position.y += recC.h;
			}
			objA.velocity.y = 0;
		}
	};

	//object we are checking 
	if (objA.type == ObjectType::player) {
		// object we are coliding with
		switch (objB.type) {
			case ObjectType::level: {
				if (!objA.data.player.invincible) {
					objA.data.player.staminaPoints -= 10; // depleet stamina on hit
					objA.data.player.invincible = true;
				}
				glm::vec2 prevVel = objA.velocity;
				genericResponse();
				objA.velocity = -prevVel; // bounce of wall
				objA.shouldFlash = true;
				break;
			}
			case ObjectType::coin: {
				objA.data.player.collectedCoins += objB.data.pickUp.value;
				// despawn the coin
				objB.invisible = true;
				objB.collider = { 0, 0, 0, 0 };
				break;
			}
			case ObjectType::juice: {
				if (objA.data.player.staminaPoints <= 0) {
					objA.data.player.staminaPoints = objB.data.pickUp.value;
				}
				else {
					objA.data.player.staminaPoints = std::min(objA.data.player.staminaPoints + objB.data.pickUp.value, (float)objA.data.player.maxStamina);
				}
				objB.invisible = true;
				objB.collider = { 0, 0, 0, 0 };
				SDL_Event event{ UserEvents::STAMINA_RESTORED };
				SDL_PushEvent(&event);
				break;
			}
		}
	}
}

void createTiles(const SDLState& state, GameState& gs, Resources& res) {
	struct LayerVisitor {
		const SDLState& state;
		GameState& gs;
		const Resources& res;

		LayerVisitor(const SDLState& state, GameState& gs, const Resources& res) : state(state), gs(gs), res(res) {}

		auto createObject(int r, int c, SDL_Texture* tex, ObjectType type) {
			GameObject o;
			o.type = type;
			o.position = glm::vec2(
				c * res.map->tileWidth,
				r * res.map->tileHeight);
			o.texture = tex;
			o.collider = { .x = 0, .y = 0, .w = TILE_SIZE, .h = TILE_SIZE };
			return o;
		}

		// Visitor pattern, handels difrent types of layers
		void operator()(tmx::Layer& layer) { // Tile layers
			std::vector<GameObject> newLayer;
			int i = 0;
			for (int tGid : layer.data) { // tGid = Tile Global ID
				if (tGid) { // Skip empty slots (0 = empty)

					// Find which tileset owns this tile
					const auto itr = std::find_if(res.tilesetTextures.begin(), res.tilesetTextures.end(),
						[tGid](const TileSetTextures& tst) {
							return tGid >= tst.firstGid && tGid < tst.firstGid + tst.textures.size();
						});

					if (itr == res.tilesetTextures.end()) {
						SDL_Log("Warning: Tileset not found for tile GID %d", tGid);
						i++;
						continue;  // Skip this tile
					}

					const TileSetTextures& tst = *itr;
					int localId = tGid - tst.firstGid;

					// Check bounds before accessing
					if (localId < 0 || localId >= tst.textures.size()) {
						SDL_Log("Warning: Invalid local tile ID %d for GID %d", localId, tGid);
						i++;
						continue;
					}

					SDL_Texture* tex = tst.textures[localId];

					if (!tex) {
						SDL_Log("Warning: Texture is null for tile GID %d", tGid);
						i++;
						continue;  // Skip this tile
					}

					int r = i / res.map->mapWidth; // Row position
					int c = i % res.map->mapWidth; // Colum position

					auto tile = createObject(r, c, tex, ObjectType::level);
					tile.spriteFrame = 1;

					// Only add collider if this is the "Level" layer
					if (layer.name != "Level") {
						tile.collider.w = 0;
						tile.collider.h = 0;
						tile.collider.y = 0;
						tile.collider.x = 0;
					}
					newLayer.push_back(tile);
				}
				i++;
			}
			if (!newLayer.empty()){
				gs.layers.push_back(newLayer);
				SDL_Log("Added layer '%s' with %zu tiles", layer.name.c_str(), newLayer.size());
			}
		}
		void operator()(tmx::ObjectGroup& objectGroup) { // Object layers
			std::vector<GameObject> newLayer;
			for (tmx::LayerObject& obj : objectGroup.objects){
				glm::vec2 objPos(
					obj.x - res.map->tileWidth / 2,
					obj.y - res.map->tileHeight / 2);

				// Create the player character acording to their position on the map
				if (obj.type == "Player"){
					GameObject player = createObject(1, 1, res.texIdle, ObjectType::player);
					player.position = objPos;
					player.data.player = PlayerData();
					player.data.player.collectedCoins = 0;
					player.animations = res.playerAnims;
					player.currentAnimation = res.ANIM_PLAYER_IDLE;
					player.acceleration = glm::vec2(130, 130);
					player.maxSpeedX = 100;
					player.maxSpeedY = 100;
					player.directionH = 0;
					player.directionV = 0;
					player.dynamic = true;
					player.collider = {
						.x = 11, .y = 6,
						.w = 10, .h = 26
					};
					newLayer.push_back(player);
					gs.playerIndex = 0;
					gs.playerLayer = gs.layers.size();
				}
				else if (obj.type == "Coin") {
					GameObject coin = createObject(1, 1, res.texIdle, ObjectType::coin);
					coin.data.pickUp = PickUpData();
					coin.data.pickUp.value = 1;
					coin.position = objPos;
					coin.animations = res.playerAnims; // REMOVE for item anim
					coin.currentAnimation = res.ANIM_PLAYER_IDLE;
					coin.dynamic = true;
					coin.collider = {
						.x = 11, .y = 6,
						.w = 10, .h = 26
					};
					newLayer.push_back(coin);
				}
				else if (obj.type == "Juice") {
					GameObject juice = createObject(1, 1, res.texRun, ObjectType::juice);
					juice.data.pickUp = PickUpData();
					juice.data.pickUp.value = 50;
					juice.position = objPos;
					juice.animations = res.playerAnims; // REMOVE for item anim
					juice.currentAnimation = res.ANIM_PLAYER_RUN;
					juice.dynamic = true;
					juice.collider = {
						.x = 11, .y = 6,
						.w = 10, .h = 26
					};
					newLayer.push_back(juice);
				}
			}
			gs.layers.push_back(std::move(newLayer));
		}
	};

	for (auto& layer : res.map->layers)
	{
		std::visit(LayerVisitor(state, gs, res), layer);
	}
	
}

void drawParalaxBackground(GameState* gs ,SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime) {
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w) {
		scrollPos = 0;
	}
	float yPos = - gs->mapViewport.y + 330;

	SDL_FRect dst{
		.x = scrollPos, 
		.y = yPos,
		.w = texture->w * 2.0f, //we get two copies in succesion
		.h = static_cast<float>(texture->h)
	};

	SDL_RenderTextureTiled(renderer, texture, nullptr, 1, &dst);
}

void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float width, float hight, float deltaTime) {
	float srcX = 0;
	float srcY = 0;

	// Loop through the animation
	if (obj.currentAnimation != -1) {
		srcX = obj.animations[obj.currentAnimation].currentFrame() * width;
	}
	else {
		srcX = (obj.spriteFrame - 1) * width;
	}

	SDL_FRect src{ // Portion of the texture to copy
		.x = srcX,
		.y = srcY,
		.w = width,
		.h = hight
	};

	SDL_FRect dst{ // Where on screen to render the sprite
		.x = obj.position.x - gs.mapViewport.x,
		.y = obj.position.y - gs.mapViewport.y,
		.w = width,
		.h = hight
	};

	SDL_FlipMode flipMode = obj.visualDirectionH == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	if (!obj.shouldFlash) {
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
	}
	else {
		// flash object with a redish tint
		SDL_SetTextureColorModFloat(obj.texture, 2.5f, 1.0f, 1.0f);
		SDL_RenderTextureRotated(state.renderer, obj.texture, &src, &dst, 0, nullptr, flipMode);
		SDL_SetTextureColorModFloat(obj.texture, 1.0f, 1.0f, 1.0f);

		if (obj.flashTimer.step(deltaTime)) {
			obj.shouldFlash = false;
		}
	}

	if (gs.debugMode){
		// Draw collider box (red semi-transparent rectangle)
		SDL_FRect rectA{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y - gs.mapViewport.y,
			.w = obj.collider.w,
			.h = obj.collider.h
		};
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_BLEND);

		SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 150);
		SDL_RenderFillRect(state.renderer, &rectA);

		// Draw ground sensor (blue line at bottom of collider)
		SDL_FRect sensor{
			.x = obj.position.x + obj.collider.x - gs.mapViewport.x,
			.y = obj.position.y + obj.collider.y + obj.collider.h - gs.mapViewport.y,
			.w = obj.collider.w, .h = 1
		};
		SDL_SetRenderDrawColor(state.renderer, 0, 0, 255, 150);
		SDL_RenderFillRect(state.renderer, &sensor);
		SDL_SetRenderDrawBlendMode(state.renderer, SDL_BLENDMODE_NONE);
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