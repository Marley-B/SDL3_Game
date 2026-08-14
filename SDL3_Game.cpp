// SDL3_Game.cpp : Defines the entry point for the application.
//

// General debugin tips:
//	if (!audio) {
//		SDL_Log("Failed to load audio: %%s - %s", filepath.c_str(), SDL_GetError());
//		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error loading audio", nullptr);
//	}


#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <vector>
#include <string>
#include <array>
#include <format>
#include <filesystem>
#include "SDL3_Game.h"
#include "gameobject.h"
#include "tmx.h"

using namespace std;

const int TILE_SIZE = 32;

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

// general game components coordination
struct GameState {
	std::vector<std::vector<GameObject>> layers;
	int playerLayer, playerIndex;
	SDL_FRect mapViewport;
	float bg2Scroll, bg3Scroll, bg4Scroll;
	bool debugMode;

	GameState(const SDLState& state) {
		playerLayer = -1;
		playerIndex = -1;
		mapViewport = SDL_FRect{
			.x = 0, .y = 0,
			.w = static_cast<float>(state.logW),
			.h = static_cast<float>(state.logH)
		};
		bg2Scroll = bg3Scroll = bg4Scroll = 0;
		debugMode = false;
	}
	GameObject& player() { return layers[playerLayer][playerIndex]; }
};

struct TileSetTextures
{
	int firstGid;
	std::vector<SDL_Texture*> textures;
};

// Loads assets from memory
struct Resources {
	const int ANIM_PLAYER_IDLE = 0;
	const int ANIM_PLAYER_RUN = 1;
	const int ANIM_PLAYER_SLIDE = 2;
	std::vector<Animation> playerAnims;

	std::vector<SDL_Texture*> textures;
	SDL_Texture* texIdle, * texRun, * texSlide, * texBg1, * texBg2, * texBg3, * texBg4;

	std::vector<MIX_Audio*> audioEffects;
	MIX_Audio* audioShoot, * audioShootHit, * audioEnemyHit;
	std::vector<MIX_Track*> tracks;
	MIX_Track* musicMain;

	std::vector<TileSetTextures> tilesetTextures;
	std::unique_ptr<tmx::Map> map; // currentlly only loads one map

	SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filepath) {
		SDL_Texture* tex = IMG_LoadTexture(renderer, filepath.c_str());
		if (!tex) { // fail state
			SDL_Log("Failed to load texture: %s - %s", filepath.c_str(), SDL_GetError());
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error loading texture", nullptr);
			return nullptr;
		}
		SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
		textures.push_back(tex);
		return tex;
	}

	MIX_Audio* loadAudio(MIX_Mixer* mixer, const std::string& filepath) {
		MIX_Audio* audio = MIX_LoadAudio(mixer, filepath.c_str(), true);
		if (!audio) {
			SDL_Log("Failed to load audio: %s - %s", filepath.c_str(), SDL_GetError());
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error loading audio", nullptr);
			return nullptr;
		}
		else {
			audioEffects.push_back(audio);
		}
		return audio;
	}

	MIX_Track* loadTrack(MIX_Mixer* mixer, const std::string& filepath) {
		MIX_Audio* audio = MIX_LoadAudio(mixer, filepath.c_str(), true);
		if (!audio) {
			SDL_Log("Failed to load music audio: %s - %s", filepath.c_str(), SDL_GetError());
			return nullptr;
		}
		audioEffects.push_back(audio);
		MIX_Track* track = MIX_CreateTrack(mixer);
		MIX_SetTrackAudio(track, audio);
		MIX_SetTrackLoops(track, -1);  // Set to loop infinitely
		tracks.push_back(track);
		return track;
	}

	void load(SDLState& state) {
		playerAnims.resize(3);
		playerAnims[ANIM_PLAYER_IDLE] = Animation(8, 1.6f);
		playerAnims[ANIM_PLAYER_RUN] = Animation(4, 0.5f);
		playerAnims[ANIM_PLAYER_SLIDE] = Animation(1, 1.0f);

		texIdle = loadTexture(state.renderer, "data/idle.png");
		texRun = loadTexture(state.renderer, "data/run.png");
		texSlide = loadTexture(state.renderer, "data/slide.png");
		texBg1 = loadTexture(state.renderer, "data/bg/bg_layer1.png");
		texBg2 = loadTexture(state.renderer, "data/bg/bg_layer2.png");
		texBg3 = loadTexture(state.renderer, "data/bg/bg_layer3.png");
		texBg4 = loadTexture(state.renderer, "data/bg/bg_layer4.png");

		audioShoot = loadAudio(state.mixer, "data/audio/shoot.wav");
		audioShootHit = loadAudio(state.mixer, "data/audio/wall_hit.wav");
		audioEnemyHit = loadAudio(state.mixer, "data/audio/shoot_hit.wav");
		musicMain = loadTrack(state.mixer, "data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");

		map = tmx::loadMap("data/maps/smallmap.tmx");
		if (!map)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load map file", state.window);
			return;
		}
		// Verify map properties
		SDL_Log("Map loaded: %d x %d, tile size: %d x %d",
			map->mapWidth, map->mapHeight, map->tileWidth, map->tileHeight);

		for (tmx::TileSet& tileSet : map->tileSets){
			TileSetTextures tst;
			tst.firstGid = tileSet.firstgid;

			if (tileSet.tiles.size() == 1 && tileSet.count > 1){
				// Only ONE image file, but it contains MULTIPLE tiles
				// Single image tileset - load the main image once
				const std::string imagePath = "data/tiles/" +
					std::filesystem::path(tileSet.tiles[0].image.source).filename().string();
				SDL_Texture* mainTexture = loadTexture(state.renderer, imagePath);

				// Push the same texture for each tile in the set
				for (int i = 0; i < tileSet.count; i++){
					tst.textures.push_back(mainTexture);
				}
			}
			else{
				// Individual tile images
				tst.textures.reserve(tileSet.tiles.size());
				for (tmx::Tile& tile : tileSet.tiles){
					const std::string imagePath = "data/tiles/" +
						std::filesystem::path(tile.image.source).filename().string();
					tst.textures.push_back(loadTexture(state.renderer, imagePath));
				}
			}
			tilesetTextures.push_back(std::move(tst));
		}
	}

	void unload() {
		for (SDL_Texture* tex : textures) {
			SDL_DestroyTexture(tex);
		}
		for (MIX_Audio* audio : audioEffects) {
			MIX_DestroyAudio(audio);
		}
		for (MIX_Track* track : tracks) {
			MIX_DestroyTrack(track);
		}
	}
};

bool initialize(SDLState& state);
void cleanup(SDLState& state);
void createTiles(const SDLState& state, GameState& gs, Resources& res);
void drawParalaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float width, float hight, float deltaTime);
void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);
void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& recA, const SDL_FRect& recB,
	const SDL_FRect& recC, GameObject& objA, GameObject& objB, float deltaTime);

int main(int argc, char *agrc[])
{
	//cout << "Hello CMake." << endl;
	SDLState state;
	state.width = 1600;
	state.height = 900;
	state.logW = 640;
	state.logH = 300;

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

	// start the game loop
	bool running = true;
	while (running) {
		uint64_t nowTime = SDL_GetTicks();
		float deltaTime = (nowTime - prevTime) / 1000.0f; // convert to seconds
		if (deltaTime > 0.05f) deltaTime = 0.05f; // avoid fisics errors when the game lags
		SDL_Event event{ 0 };

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
		gs.mapViewport.x = ((gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2) + 50;
		gs.mapViewport.y = ((gs.player().position.y + TILE_SIZE / 2) - gs.mapViewport.h / 2 );

		// draw background images
		SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
		drawParalaxBackground(state.renderer, res.texBg4, gs.player().velocity.x, gs.bg4Scroll, 0.075f, deltaTime);
		drawParalaxBackground(state.renderer, res.texBg3, gs.player().velocity.x, gs.bg3Scroll, 0.15f, deltaTime);
		drawParalaxBackground(state.renderer, res.texBg2, gs.player().velocity.x, gs.bg2Scroll, 0.3f, deltaTime);

		// draw all objects
		for (auto& layer : gs.layers) {
			for (GameObject& obj : layer) {
				drawObject(state, gs, obj, TILE_SIZE, TILE_SIZE, deltaTime);
			}    
		}

		if (gs.debugMode == true) {
			// Enhanced debug display with position and viewport info
			SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(state.renderer, 5, 5,
				std::format("S: {}, S: {}, Pos:({:.1f},{:.1f}), VP:({:.1f},{:.1f})",
					static_cast<int>(gs.player().data.player.state),
					gs.player().data.player.staminaPoints,
					gs.player().position.x,
					gs.player().position.y,
					gs.mapViewport.x,
					gs.mapViewport.y
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

void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
	// update animation
	if (obj.currentAnimation != -1) {
		obj.animations[obj.currentAnimation].step(deltaTime);
	}

	// if (obj.dynamic && !obj.grounded)
	//if (obj.dynamic) { // apply gravity
		//obj.velocity += glm::vec2(0, 500) * deltaTime;
	//} REMOVE

	// float moving = 0;
	// float currentDirectionH and currentDirectionV
	float currentDirection = 0;
	if (obj.type == ObjectType::player) {
		if (state.keys[SDL_SCANCODE_A]) {
			currentDirection += -1;
		}
		if (state.keys[SDL_SCANCODE_D]) {
			currentDirection += 1;
		}
		// add up and down directions FUTURE 

		switch (obj.data.player.state) {
		case PlayerState::idle: {
			// switch to running state
			if (currentDirection) {
				obj.data.player.state = PlayerState::running;
			}
			else {
				// expand for y axis FUTURE
				// decelerate
				if (obj.velocity.x) {
					const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
					float amount = factor * obj.acceleration.x * deltaTime;
					if (std::abs(obj.velocity.x) < std::abs(amount)) {
						obj.velocity.x = 0;
					}
					else {
						obj.velocity.x += amount;
					}
				}
			}
			break;
		}
		case PlayerState::running: {
			if (!currentDirection) {
				obj.data.player.state = PlayerState::idle;
				obj.texture = res.texIdle;
				obj.currentAnimation = res.ANIM_PLAYER_IDLE;
			}
			// moving in opposite direction of velocity, sliding!
			if (obj.velocity.x * obj.direction < 0) {
				// add sliding animation
			}
			else {
				
			}
			break;
		}
		}

	}
	
	

	if (currentDirection)
	{
		obj.direction = currentDirection;
	}
	// add acceleration to velocity
	obj.velocity += currentDirection * obj.acceleration * deltaTime;
	if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
		obj.velocity.x = currentDirection * obj.maxSpeedX;
	}
	// add velocity to position
	obj.position += obj.velocity * deltaTime;

	// handle collision detection
	for (auto& layer : gs.layers) {
		for (GameObject& objB : layer) {
			if (&obj != &objB) {
				checkCollision(state, gs, res, obj, objB, deltaTime);

				// maybe usable for detecting ground = death ? FUTURE
				//if (objB.type == ObjectType::level && objB.collider.w != 0
				//	&& objB.collider.h != 0) {
				//	// grounded sensor
				//	SDL_FRect sensor{
				//		.x = obj.position.x + obj.collider.x,
				//		.y = obj.position.y + obj.collider.y + obj.collider.h,
				//		.w = obj.collider.w, .h = 1
				//	};
				//	SDL_FRect recB{
				//		.x = objB.position.x + objB.collider.x,
				//		.y = objB.position.y + objB.collider.y,
				//		.w = objB.collider.w, .h = objB.collider.h
				//	};
				//	SDL_FRect recC{ 0 };
				//	if (SDL_GetRectIntersectionFloat(&sensor, &recB, &recC)) {
				//		foundGround = true;
				//	}
				//}
			}
		}
	}
}

void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime) {
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
				glm::vec2 prevVel = objA.velocity;
				objA.data.player.staminaPoints -= 10; // depleat stamina on hit
				genericResponse();
				objA.velocity = -prevVel; // push away from wall
				break;
			}
			//case ObjectType::coin{}  FUTURE
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
					player.animations = res.playerAnims;
					player.currentAnimation = res.ANIM_PLAYER_IDLE;
					player.acceleration = glm::vec2(300, 0);
					player.maxSpeedX = 100;
					player.direction = 1;
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
					GameObject coin = createObject(1, 1, res.texSlide, ObjectType::coin);
					coin.position = objPos;
					coin.data.pickUp = PickUpData();
					coin.dynamic = true;
					// need to fill rest FUTURE
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

void drawParalaxBackground(SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime) {
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w) {
		scrollPos = 0;
	}

	SDL_FRect dst{
		.x = scrollPos, .y = 0,
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

	SDL_FlipMode flipMode = obj.direction == -1 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
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