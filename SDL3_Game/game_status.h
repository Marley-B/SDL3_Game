#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include "tmx.h"
#include "resources.h"
#include "user_events.h"
#include "level_state.h"
#include "game_state.h"

// Forward declarations
class Resources;
class GameObject;
struct GameState;
class ObjectState;
class Level;

class Button {
public:
    //Button dimensions
    float kButtonWidth = 300;
    float kButtonHeight = 200;

    //Initializes internal variables
    Button(Resources& res);
    void setPosition(float x, float y); //Sets top left position
    void setDimensions(float w, float h);
    bool handleEvent(SDL_Event* e, Resources& res); //Handles mouse event
    void render(SDLState* state); //Shows button sprite

private:
    //Top left position
    SDL_FPoint mPosition;
    //Currently used sprite
    SDL_Texture* mCurrentTexture;
};

class GameStatus{
public:
    //State transitions
    virtual bool enter(Resources& res) = 0;
    //virtual bool exit() = 0;

    //Main loop functions
    virtual void handleEvent(SDL_Event& e, GameState& gs, Resources& res) = 0;
    virtual void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) = 0;
    virtual void render(SDLState* state) = 0;

    //Make sure to call child destructors
    virtual ~GameStatus() = default;
};

class IntroMenu : public GameStatus {
public:
    static IntroMenu* get();
    bool enter(Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState* state) override;
private:
    static IntroMenu sIntroMenu;
    IntroMenu();
    SDL_Texture* mBgTexture;
};

class LevelMenu : public GameStatus {
public:
    static LevelMenu* get();
    bool enter(Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState* state) override;
private:
    static LevelMenu sLevelMenu;
    LevelMenu();
    SDL_Texture* mBgTexture;
    Button button1;
    Button button2;
    Button button3;
    Button button4;
};

class DeathState : public GameStatus {
public:
    static DeathState* get();
    bool enter(Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState* state) override;
private:
    static DeathState sDeathState;
    SDL_Texture* mBgTexture;
    Button button1;
    Button button2;
    DeathState();
};

class WinState : public GameStatus {
public:
    static WinState* get();
    bool enter(Resources& res) override;
    void handleEvent(SDL_Event& e, GameState& gs, Resources& res) override;
    void update(const SDLState& state, GameState& gs, Resources& res, float deltaTime) override;
    void render(SDLState* state) override;
private:
    static WinState sWinState;
    SDL_Texture* mBgTexture;
    Button button;
    WinState();
};

bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res);