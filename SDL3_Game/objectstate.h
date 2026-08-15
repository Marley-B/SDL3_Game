#pragma once
#include <SDL3/SDL.h>
#include <cmath> 

// Forward declarations
class Resources;
class GameObject;
struct GameState;
class ObjectState;

class ObjectState {
public:
	virtual bool enter(Resources& res, GameObject& obj) = 0; // load tex and animations
	virtual void handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) = 0; // handle key inputs, if x happens -> change state
	virtual void update(GameObject& obj, float deltaTime) = 0; // handle actions
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

bool changeState(ObjectState* newState, ObjectState*& currentState, Resources& res, GameObject& obj);

