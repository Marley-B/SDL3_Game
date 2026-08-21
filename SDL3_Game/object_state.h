#pragma once
#include <SDL3/SDL.h>
#include "resources.h"
#include "game_object.h"
#include "game_state.h"
#include "user_events.h"
#include <cmath> 

// Forward declarations
struct Resources;
struct GameObject;
struct GameState;
class ObjectState;

class ObjectState {
public:
	virtual bool enter(Resources& res, GameObject& obj) = 0; // load tex and animations
    // If x happens -> change state
	virtual void handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) = 0; // handle key input
	virtual void update(GameObject& obj, float deltaTime) = 0; // handle actions and non input changes
    virtual ~ObjectState() = default;
};

class PlayerIdle : public ObjectState {
public:
	static PlayerIdle* get();
	bool enter(Resources& res, GameObject& obj) override;
	void handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) override;
	void update(GameObject& obj, float deltaTime) override;
private:
    static PlayerIdle sPlayerIdle;
    PlayerIdle() = default;
};

class PlayerFly : public ObjectState {
public:
    static PlayerFly* get();
    bool enter(Resources& res, GameObject& obj) override;
    void handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) override;
    void update(GameObject& obj, float deltaTime) override;
private:
    static PlayerFly sPlayerFly;
    PlayerFly() = default;
};

class PlayerNoStamina : public ObjectState {
public:
    static PlayerNoStamina* get();
    bool enter(Resources& res, GameObject& obj) override;
    void handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) override;
    void update(GameObject& obj, float deltaTime) override;
private:
    static PlayerNoStamina sPlayerNoStamina;
    PlayerNoStamina() = default;
};

bool changeState(ObjectState* newState, ObjectState*& currentState, Resources& res, GameObject& obj);

