#include "level_state.h"
#include <variant>

std::vector<Level*> Level::sLevels;

const int TILE_SIZE = 32;

Level::Level(int levelId, std::unique_ptr<tmx::Map>& map) :
    mLevelId{ levelId },
	mMap{ map.get() }
{
}

Level* Level::get(int id) {
    // Check if level already exists
    for (auto* level : sLevels) {
        if (level && level->mLevelId == id) {
            return level;
        }
    }
    return nullptr;
}

tmx::Map* Level::getMap() {
	return mMap;
}


bool Level::enter(SDLState& state, GameState& gs, Resources& res) {
	createTiles(state, gs, res);
	stUi = new StaminaUi(res);
	stUi->setPosition(20, 20);
	coUi = new CoinUi();
	coUi->setPosition(130, 14);
	return true;
}

void Level::handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) {
    if (e.type == UserEvents::PLAYER_DEATH) {
		exit(gs, obj);
        changeState(DeathState::get(), gs.currentStateGame, res, state, gs);
    }
    else if (e.type == UserEvents::PLAYER_WIN) {
		exit(gs, obj);
		changeState(WinState::get(), gs.currentStateGame, res, state, gs);
    }

	if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP ||
		e.type == UserEvents::STAMINA_DEPLETED || e.type == UserEvents::STAMINA_RESTORED) {
		for (auto& layer : gs.layers) {
			for (GameObject& obj : layer) {
				if (obj.dynamic && obj.type == ObjectType::player) {
					gs.currentStatePlayer->handleEvent(e, res, obj, gs);
				}
			}
		}
	}
}

void Level::render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime){
	// draw background images
	SDL_RenderTexture(state.renderer, res.texBg1, nullptr, nullptr);
	drawParalaxBackground(&gs, state.renderer, res.texBg5, gs.player().velocity.x, gs.bg5Scroll, 0.025f, deltaTime);
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

	stUi->render(state, gs, obj, res);
	coUi->render(state, gs);
}

void Level::update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
	// calculate viewport position 
	gs.mapViewport.x = (gs.player().position.x + TILE_SIZE / 2) - gs.mapViewport.w / 2 + 30;
	gs.mapViewport.y = (gs.player().position.y + TILE_SIZE / 2) - gs.mapViewport.h / 2;

	// update all objects
	for (auto& layer : gs.layers) {
		for (GameObject& obj : layer) {
			if (obj.dynamic) { // No updating background elements
				objUpdate(state, gs, res, obj, deltaTime);
			}
		}
	}

	stUi->update(res, gs);

	if (gs.debugMode == true) {
		// Enhanced debug display with position and viewport info
		SDL_SetRenderDrawColor(state.renderer, 255, 255, 255, 255);
		SDL_Log("S: %s, C: %d, St: %.1f, Pos:(%.1f,%.1f), Dir:(%d,%d)",
			typeid(*gs.currentStatePlayer).name(),
			gs.player().data.player.collectedCoins,
			gs.player().data.player.staminaPoints,
			gs.player().position.x,
			gs.player().position.y,
			gs.player().directionH,
			gs.player().directionV
		);
	}
}

void Level::exit(GameState& gs, GameObject& obj) {
	// Clear all layers
	gs.layers.clear();
	// Reset player indices
	gs.playerIndex = -1;
	gs.playerLayer = -1;
	gs.currentStatePlayer = PlayerIdle::get();
	obj.data.player.collectedCoins = 0;
	obj.data.player.staminaPoints = 100;
	if (stUi) {
		delete stUi;
		stUi = nullptr;
	}
	SDL_Log("Cleared all level tiles and objects");
}

void objUpdate(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) {
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
			obj.data.player.staminaPoints += -0.7 * deltaTime;
		}

		if (obj.data.player.staminaPoints <= 0) {
			SDL_Event event{ UserEvents::STAMINA_DEPLETED };
			SDL_PushEvent(&event);
		}
		if (obj.data.player.staminaPoints <= -25 || obj.position.y > 570) {
			SDL_Event event{ UserEvents::PLAYER_DEATH };
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
		case ObjectType::win: {
			objA.data.player.collectedCoins += objB.data.pickUp.value;
			SDL_Event event{ UserEvents::PLAYER_WIN};
			SDL_PushEvent(&event);
		}
		}
	}
}

void createTiles(const SDLState& state, GameState& gs, Resources& res) {
	if (gs.currentLevel == nullptr) {
		SDL_Log("createTiles: currentLevel is null");
		return;
	}
	Level* lvl = dynamic_cast<Level*>(gs.currentLevel);
	if (!lvl) {
		SDL_Log("createTiles: currentLevel is not a Level");
		return;
	}
	tmx::Map* map = lvl->getMap();
	if (!map) {
		SDL_Log("createTiles: level map is null");
		return;
	}

	struct LayerVisitor {
		const SDLState& state;
		GameState& gs;
		const Resources& res;
		tmx::Map* map;

		LayerVisitor(const SDLState& state, GameState& gs, const Resources& res, tmx::Map* map) : state(state), gs(gs), res(res), map(map) {}
		auto createObject(int r, int c, SDL_Texture* tex, ObjectType type) {
			GameObject o;
			o.type = type;
			o.position = glm::vec2(
				c * map->tileWidth,
				r * map->tileHeight);
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

					int r = i / map->mapWidth; // Row position
					int c = i % map->mapWidth; // Colum position

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
			if (!newLayer.empty()) {
				gs.layers.push_back(newLayer);
				SDL_Log("Added layer '%s' with %zu tiles", layer.name.c_str(), newLayer.size());
			}
		}
		void operator()(tmx::ObjectGroup& objectGroup) { // Object layers
			std::vector<GameObject> newLayer;
			for (tmx::LayerObject& obj : objectGroup.objects) {
				glm::vec2 objPos(
					obj.x - map->tileWidth / 2,
					obj.y - map->tileHeight / 2);

				// Create the player character acording to their position on the map
				if (obj.type == "Player") {
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
						.x = 1, .y = 2,
						.w = 30, .h = 26
					};
					newLayer.push_back(player);
					gs.playerIndex = 0;
					gs.playerLayer = gs.layers.size();
				}
				else if (obj.type == "Coin") {
					GameObject coin = createObject(1, 1, res.texBigButterfly, ObjectType::coin);
					coin.data.pickUp = PickUpData();
					coin.data.pickUp.value = 1;
					coin.position = objPos;
					coin.animations = res.objectAnims;
					coin.currentAnimation = res.ANIM_BIG_BUTTERFLY;
					coin.dynamic = true;
					coin.collider = {
						.x = 11, .y = 6,
						.w = 10, .h = 26
					};
					newLayer.push_back(coin);
				}
				else if (obj.type == "SmallB") {
					GameObject smallB = createObject(1, 1, res.texSmallButterfly, ObjectType::coin);
					smallB.data.pickUp = PickUpData();
					smallB.data.pickUp.value = 0;
					smallB.position = objPos;
					smallB.animations = res.objectAnims;
					smallB.currentAnimation = res.ANIM_SMALL_BUTTERFLY;
					smallB.dynamic = true;
					smallB.collider = {
						.x = 0, .y = 0,
						.w = 0, .h = 0
					};
					newLayer.push_back(smallB);
				}
				else if (obj.type == "Juice") {
					GameObject juice = createObject(1, 1, res.texMana, ObjectType::juice);
					juice.data.pickUp = PickUpData();
					juice.data.pickUp.value = 50;
					juice.position = objPos;
					juice.animations = res.objectAnims; 
					juice.currentAnimation = res.ANIM_MANA;
					juice.dynamic = true;
					juice.collider = {
						.x = 11, .y = 6,
						.w = 10, .h = 26
					};
					newLayer.push_back(juice);
				}
				else if (obj.type == "Win") {
					GameObject win = createObject(1, 1, res.texBigButterfly, ObjectType::win);
					win.data.pickUp = PickUpData();
					win.data.pickUp.value = 1;
					win.position = objPos;
					win.animations = res.objectAnims;
					win.currentAnimation = res.ANIM_BIG_BUTTERFLY;
					win.dynamic = true;
					win.collider = {
						.x = 11, .y = 6,
						.w = 10, .h = 26
					};
					newLayer.push_back(win);
				}
			}
			gs.layers.push_back(std::move(newLayer));
		}
	};

	for (auto& layer : map->layers)
	{
		std::visit(LayerVisitor(state, gs, res, map), layer);
	}

}

void drawParalaxBackground(GameState* gs, SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity, float& scrollPos, float scrollFactor, float deltaTime) {
	scrollPos -= xVelocity * scrollFactor * deltaTime;
	if (scrollPos <= -texture->w) {
		scrollPos = 0;
	}
	float yPos = -gs->mapViewport.y + 330;

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

	if (gs.debugMode) {
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