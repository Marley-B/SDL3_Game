#pragma once
#include <vector>
#include <memory>
#include <format>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include "tmx.h"
#include "resources.h"
#include "user_events.h"
#include "game_status.h"
#include "game_state.h"
#include "object_state.h" 


// Forward declarations
struct Resources;
struct GameObject;
struct GameState;
class ObjectState;
class GameStatus;

class Level : public GameStatus {
public:
    static std::vector<Level*> sLevels;
    Level(int levelId, std::unique_ptr<tmx::Map>& map);
    static Level* get(int id);
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    tmx::Map* getMap();
private:
    int mLevelId;
    tmx::Map* mMap = nullptr;
};

void objUpdate(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime);
void checkCollision(const SDLState& state, GameState& gs, Resources& res, GameObject& a, GameObject& b, float deltaTime);
void collisionResponse(const SDLState& state, GameState& gs, Resources& res, const SDL_FRect& recA, const SDL_FRect& recB,
    const SDL_FRect& recC, GameObject& objA, GameObject& objB, float deltaTime);
void createTiles(const SDLState& state, GameState& gs, Resources& res);
void drawParalaxBackground(GameState* gs, SDL_Renderer* renderer, SDL_Texture* texture, float xVelocity,
    float& scrollPos, float scrollFactor, float deltaTime);
void drawObject(const SDLState& state, GameState& gs, GameObject& obj, float width, float hight, float deltaTime);
