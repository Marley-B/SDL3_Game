#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include "tmx.h"
#include "resources.h"
#include "user_events.h"
#include "sdlstate.h" 
#include "game_state.h"
#include "ui.h"

// Forward declarations
class Level;
struct Resources;
struct GameObject;
struct GameState;
class ObjectState;

class GameStatus{
public:
    //State transitions
    virtual bool enter(SDLState& state, GameState& gs, Resources& res) = 0;
    //Main loop functions
    virtual void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj) = 0;
    virtual void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) = 0;
    virtual void render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) = 0;

    //Make sure to call child destructors
    virtual ~GameStatus() = default;
};

class IntroMenu : public GameStatus {
public:
    static IntroMenu* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj)  override;
    void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
private:
    static IntroMenu sIntroMenu;
    IntroMenu() = default;
    SDL_Texture* mBgTexture;
};

class LevelMenu : public GameStatus {
public:
    static LevelMenu* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj)  override;
    void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
private:
    static LevelMenu sLevelMenu;
    LevelMenu() = default;
    SDL_Texture* mBgTexture;
    std::vector<std::unique_ptr<Button>> mButtons;
};

class DeathState : public GameStatus {
public:
    static DeathState* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj)  override;
    void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
private:
    static DeathState sDeathState;
    SDL_Texture* mBgTexture;
    std::vector<std::unique_ptr<Button>> mButtons;
    DeathState() = default;
};

class WinState : public GameStatus {
public:
    static WinState* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state, GameObject& obj)  override;
    void update(const SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, GameObject& obj, float deltaTime) override;
private:
    static WinState sWinState;
    SDL_Texture* mBgTexture;
    std::vector<std::unique_ptr<Button>> mButtons;
    WinState() = default;
};

bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res, SDLState& state, GameState& gs);