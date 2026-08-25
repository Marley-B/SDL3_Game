#include "resources.h"

SDL_Texture* Resources::loadTexture(SDL_Renderer* renderer, const std::string& filepath) {
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

MIX_Audio* Resources::loadAudio(MIX_Mixer* mixer, const std::string& filepath) {
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

MIX_Track* Resources::loadTrack(MIX_Mixer* mixer, const std::string& filepath) {
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

void Resources::load(SDLState& state) {
	playerAnims.resize(2);
	objectAnims.resize(3);
	playerAnims[ANIM_PLAYER_IDLE] = Animation(3, 1.6f);
	playerAnims[ANIM_PLAYER_FLY] = Animation(3, 0.5f);
	objectAnims[ANIM_SMALL_BUTTERFLY] = Animation(3, 1.0f);
	objectAnims[ANIM_BIG_BUTTERFLY] = Animation(3, 1.0f);
	objectAnims[ANIM_MANA] = Animation(3, 1.0f);

	texIdle = loadTexture(state.renderer, "data/idle.png");
	texFly = loadTexture(state.renderer, "data/fly.png");
	texMana = loadTexture(state.renderer, "data/mana.png");
	texBigButterfly = loadTexture(state.renderer, "data/bigButterfly.png");
	texSmallButterfly = loadTexture(state.renderer, "data/smallButterfly.png");
	texBg1 = loadTexture(state.renderer, "data/bg/bg_layer1.png");
	texBg2 = loadTexture(state.renderer, "data/bg/bg_layer2.png");
	texBg3 = loadTexture(state.renderer, "data/bg/bg_layer3.png");
	texBg4 = loadTexture(state.renderer, "data/bg/bg_layer4.png");
	texBg5 = loadTexture(state.renderer, "data/bg/bg_layer5.png");
	texGameOverScreen = loadTexture(state.renderer, "data/bg/gameOverScreen.png");
	texIntroScreen = loadTexture(state.renderer, "data/bg/introScreen.png");
	texLevelMenuScreen = loadTexture(state.renderer, "data/bg/levelMenuScreen.png");
	texWinScreen = loadTexture(state.renderer, "data/bg/winScreen.png");
	texButtHov = loadTexture(state.renderer, "data/ui/button_hovering.png");
	texButtDown = loadTexture(state.renderer, "data/ui/button_idle.png");
	texButt = loadTexture(state.renderer, "data/ui/button_idle.png");
	texStBox = loadTexture(state.renderer, "data/ui/stamina_box.png");
	texStRed = loadTexture(state.renderer, "data/ui/stamina_red.png");
	texStYellow = loadTexture(state.renderer, "data/ui/stamina_yellow.png");
	texStGreen = loadTexture(state.renderer, "data/ui/stamina_green.png");
	texStBg = loadTexture(state.renderer, "data/ui/stamina_bg.png");

	audioShoot = loadAudio(state.mixer, "data/audio/shoot.wav");
	audioShootHit = loadAudio(state.mixer, "data/audio/wall_hit.wav");
	audioEnemyHit = loadAudio(state.mixer, "data/audio/shoot_hit.wav");
	musicMain = loadTrack(state.mixer, "data/audio/Juhani Junkala [Retro Game Music Pack] Level 1.mp3");

	font = TTF_OpenFont("data/text/Pixeled.ttf", 14);

	map1 = tmx::loadMap("data/maps/Level1.tmx");
	if (!map1)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load map file", state.window);
		return;
	}
	// Verify map properties
	SDL_Log("Map loaded: %d x %d, tile size: %d x %d",
		map1->mapWidth, map1->mapHeight, map1->tileWidth, map1->tileHeight);

	for (tmx::TileSet& tileSet : map1->tileSets) {
		TileSetTextures tst;
		tst.firstGid = tileSet.firstgid;

		if (tileSet.tiles.size() == 1 && tileSet.count > 1) {
			// Only ONE image file, but it contains MULTIPLE tiles
			// Single image tileset - load the main image once
			const std::string imagePath = "data/tiles/" +
				std::filesystem::path(tileSet.tiles[0].image.source).filename().string();
			SDL_Texture* mainTexture = loadTexture(state.renderer, imagePath);

			// Push the same texture for each tile in the set
			for (int i = 0; i < tileSet.count; i++) {
				tst.textures.push_back(mainTexture);
			}
		}
		else {
			// Individual tile images
			tst.textures.reserve(tileSet.tiles.size());
			for (tmx::Tile& tile : tileSet.tiles) {
				const std::string imagePath = "data/tiles/" +
					std::filesystem::path(tile.image.source).filename().string();
				tst.textures.push_back(loadTexture(state.renderer, imagePath));
			}
		}
		tilesetTextures.push_back(std::move(tst));
	}

	map2 = tmx::loadMap("data/maps/Level1.tmx");
	if (!map2)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Failed to load map file", state.window);
		return;
	}
	// Verify map properties
	SDL_Log("Map loaded: %d x %d, tile size: %d x %d",
		map2->mapWidth, map2->mapHeight, map2->tileWidth, map2->tileHeight);

	for (tmx::TileSet& tileSet : map2->tileSets) {
		TileSetTextures tst;
		tst.firstGid = tileSet.firstgid;

		if (tileSet.tiles.size() == 1 && tileSet.count > 1) {
			// Only ONE image file, but it contains MULTIPLE tiles
			// Single image tileset - load the main image once
			const std::string imagePath = "data/tiles/" +
				std::filesystem::path(tileSet.tiles[0].image.source).filename().string();
			SDL_Texture* mainTexture = loadTexture(state.renderer, imagePath);

			// Push the same texture for each tile in the set
			for (int i = 0; i < tileSet.count; i++) {
				tst.textures.push_back(mainTexture);
			}
		}
		else {
			// Individual tile images
			tst.textures.reserve(tileSet.tiles.size());
			for (tmx::Tile& tile : tileSet.tiles) {
				const std::string imagePath = "data/tiles/" +
					std::filesystem::path(tile.image.source).filename().string();
				tst.textures.push_back(loadTexture(state.renderer, imagePath));
			}
		}
		tilesetTextures.push_back(std::move(tst));
	}
}

void Resources::unload() {
	for (SDL_Texture* tex : textures) {
		SDL_DestroyTexture(tex);
	}
	for (MIX_Audio* audio : audioEffects) {
		MIX_DestroyAudio(audio);
	}
	for (MIX_Track* track : tracks) {
		MIX_DestroyTrack(track);
	}
	if (font) {
		TTF_CloseFont(font);
		font = nullptr;
	}
}