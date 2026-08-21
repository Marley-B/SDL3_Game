#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include "tmx.h"
#include "resources.h"
#include "user_events.h"
#include "sdlstate.h" 
#include "game_state.h"

// Forward declarations
class Level;
struct Resources;
struct GameObject;
struct GameState;
class ObjectState;

class Button {
public:
    //Button dimensions
    float kButtonWidth = 300;
    float kButtonHeight = 200;

    //Initializes internal variables
    Button(Resources& res);
    void setPosition(float x, float y); //Sets top left position
    void setDimensions(float w, float h);
    bool handleEvent(SDL_Event* e, Resources& res, SDLState& state); //Handles mouse event
    void render(SDLState& state, GameState& gs); //Shows button sprite

private:
    //Top left position
    SDL_FPoint mPosition;
    //Currently used sprite
    SDL_Texture* mCurrentTexture;
};


class GameStatus{
public:
    //State transitions
    virtual bool enter(SDLState& state, GameState& gs, Resources& res) = 0;
    //Main loop functions
    virtual void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state) = 0;
    virtual void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) = 0;
    virtual void render(SDLState& state, GameState& gs, Resources& res, float deltaTime) = 0;

    //Make sure to call child destructors
    virtual ~GameStatus() = default;
};

class IntroMenu : public GameStatus {
public:
    static IntroMenu* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
private:
    static IntroMenu sIntroMenu;
    IntroMenu() = default;
    SDL_Texture* mBgTexture;
};

class LevelMenu : public GameStatus {
public:
    static LevelMenu* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
private:
    static LevelMenu sLevelMenu;
    LevelMenu() = default;
    SDL_Texture* mBgTexture;
    Button* button1 = nullptr;
    Button* button2 = nullptr;
    Button* button3 = nullptr;
    Button* button4 = nullptr;
};

class DeathState : public GameStatus {
public:
    static DeathState* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
private:
    static DeathState sDeathState;
    SDL_Texture* mBgTexture;
    Button* button1 = nullptr;
    Button* button2 = nullptr;
    DeathState() = default;
};

class WinState : public GameStatus {
public:
    static WinState* get();
    bool enter(SDLState& state, GameState& gs, Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res, SDLState& state) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
private:
    static WinState sWinState;
    SDL_Texture* mBgTexture;
    Button* button = nullptr;
    WinState() = default;
};

bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res, SDLState& state, GameState& gs);