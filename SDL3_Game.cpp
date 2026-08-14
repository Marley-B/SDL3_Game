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
	//std::vector<GameObject> bullets;
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
	const int ANIM_PLAYER_SHOOT = 3;
	const int ANIM_PLAYER_SLIDE_SHOOT = 4;
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

		map = tmx::loadMap("data/maps/largemap.tmx");

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
		gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2;
		gs.mapViewport.y = (gs.player().position.y + TILE_SIZE / 2) - gs.mapViewport.h / 2;
		//gs.mapViewport.y = res.map->mapHeight * res.map->tileHeight - gs.mapViewport.h;  REMOVE

		// perform drawing commands  REMOVE ?
		//SDL_SetRenderDrawColor(state.renderer, 20, 10, 30, 255);
		//SDL_RenderClear(state.renderer);

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
			/* // REMOVE
			if (!newLayer.empty()){
				gs.layers.push_back(newLayer);
				SDL_Log("Added layer '%s' with %zu tiles", layer.name.c_str(), newLayer.size());
			}
			*/
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