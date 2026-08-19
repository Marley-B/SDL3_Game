#pragma once
#include <vector>
#include <memory>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include "tmx.h"
#include "resources.h"
#include "user_events.h"
#include "game_status.h"
#include "game_state.h"

// Forward declarations
class Resources;
class GameObject;
struct GameState;
class ObjectState;

class Level : public GameStatus {
public:
    static std::vector<Level*> sLevels;
    Level(int levelId, std::unique_ptr<tmx::Map>& map);
    static Level* get(int id);
    bool enter(Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState* state) override;
private:
    int mLevelId;
    tmx::Map* map = nullptr;
};