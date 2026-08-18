#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <glm/glm.hpp>
#include "tmx.h"
#include "resources.h"

// Forward declarations
class Resources;
class GameObject;
struct GameState;
class ObjectState;

class GameStatus{
public:
    //State transitions
    virtual bool enter() = 0;
    //virtual bool exit() = 0;

    //Main loop functions
    virtual void handleEvent(SDL_Event& e) = 0;
    virtual void update() = 0;
    virtual void render() = 0;

    //Make sure to call child destructors
    virtual ~GameStatus() = default;
};

//class IntroMenu : public GameStatus {
//public:
//    static IntroMenu* get();
//    bool enter() override; 
//    void handleEvent(SDL_Event& e) override;
//    void update() override;
//    void render() override;
//private:
//    static IntroMenu sIntroMenu;
//    IntroMenu();
//};
//
//class LevelMenu : public GameStatus {
//public:
//    static LevelMenu* get();
//    bool enter() override;
//    void handleEvent(SDL_Event& e) override;
//    void update() override;
//    void render() override;
//private:
//    static LevelMenu sLevelMenu;
//    LevelMenu() = default;
//};
//
//class Level : public GameStatus {
//public:
//    static Level* get();
//    bool enter() override;
//    void handleEvent(SDL_Event& e) override;
//    void update() override;
//    void render() override;
//private:
//    static Level sLevel;
//    LevelMenu() = default;
//    tmx::Map map;
//};
//
//class Death : public GameStatus {
//public:
//    static Death* get();
//    bool enter() override;
//    void handleEvent(SDL_Event& e) override;
//    void update() override;
//    void render() override;
//private:
//    static Death sDeath;
//    Death() = default;
//};

class Button {
public:
    //Button dimensions
    float kButtonWidth = 300;
    float kButtonHeight = 200;

    //Initializes internal variables
    Button(Resources& res);
    //Sets top left position
    void setPosition(float x, float y);
    void setDimensions(float w, float h);
    //Handles mouse event
    void handleEvent(SDL_Event* e, Resources& res);
    //Shows button sprite
    void render(SDLState* state);

private:
    enum class eButtonSprite
    {
        MouseOut = 0,
        MouseOverMotion = 1,
        MouseDown = 2,
        MouseUp = 3
    };

    //Top left position
    SDL_FPoint mPosition;
    //Currently used sprite
    SDL_Texture* mCurrentTexture;
};

bool changeState(GameStatus* newState, GameStatus*& currentState, Resources& res);