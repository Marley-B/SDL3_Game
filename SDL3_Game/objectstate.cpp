#include "objectstate.h"
#include "resources.h"
#include "gameobject.h"
#include "gamestate.h"
#include <glm/glm.hpp>

PlayerIdle PlayerIdle::sPlayerIdle;
PlayerFly PlayerFly::sPlayerFly;

PlayerIdle* PlayerIdle::get() {
    return &sPlayerIdle;
}

bool PlayerIdle::enter(Resources& res, GameObject& obj) {
    obj.texture = res.texIdle;
    obj.currentAnimation = res.ANIM_PLAYER_IDLE;
    return true;
}

void PlayerIdle::handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) {
    //SDL_Log("Entered idle handle event");

    //If a key was pressed
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0)
    {
        if (e.key.key == SDLK_LEFT) {
            obj.directionH += -1;
        }
        if (e.key.key == SDLK_RIGHT) {
            obj.directionH += 1;
        }
        if (e.key.key == SDLK_UP) {
            obj.directionV += -1;
        }
        if (e.key.key == SDLK_DOWN) {
            obj.directionV += 1;
        }

        if (obj.directionH != 0 || obj.directionV != 0) { // if the player started moving
            changeState(PlayerFly::get(), gs.currentStatePlayer, res, obj);
        }
    }
}

void PlayerIdle::update(GameObject& obj, float deltaTime) {
    // deacelerate
    if (obj.velocity.x) {
        const float factor = obj.velocity.x > 0 ? -1.2f : 1.2f;
        float amount = factor * obj.acceleration.x * deltaTime;
        if (std::abs(obj.velocity.x) < std::abs(amount)) {
            obj.velocity.x = 0;
        }
        else {
            obj.velocity.x += amount;
        }
    }
    //SDL_Log("Entered idle update");

    // apply gravity
    obj.velocity += glm::vec2(0, 30) * deltaTime;
    obj.position += obj.velocity * deltaTime;

    if (obj.velocity.x > 1.0f) {
        obj.visualDirectionH = 1;
    }
    else if (obj.velocity.x < -1.0f) {
        obj.visualDirectionH = -1;
    }
}


PlayerFly* PlayerFly::get() {
    return &sPlayerFly;
}

bool PlayerFly::enter(Resources& res, GameObject& obj) {
    obj.texture = res.texRun;
    obj.currentAnimation = res.ANIM_PLAYER_RUN;
    return true;
}

void PlayerFly::handleEvent(SDL_Event& e, Resources& res, GameObject& obj, GameState& gs) {
    //If a key was pressed
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0)
    {
        if (e.key.key == SDLK_LEFT) {
            obj.directionH += -1;
        }
        if (e.key.key == SDLK_RIGHT) {
            obj.directionH += 1;
        }
        if (e.key.key == SDLK_UP) {
            obj.directionV += -1;
        }
        if (e.key.key == SDLK_DOWN) {
            obj.directionV += 1;
        }
    }
    // Handle key release events
    else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0)
    {
        switch (e.key.key) {
        case SDLK_LEFT:
            if (obj.directionH == -1) obj.directionH = 0;
            break;
        case SDLK_RIGHT:
            if (obj.directionH == 1) obj.directionH = 0;
            break;
        case SDLK_UP:
            if (obj.directionV == -1) obj.directionV = 0;
            break;
        case SDLK_DOWN:
            if (obj.directionV == 1) obj.directionV = 0;
            break;
        }

        // Check if both directions are 0 (player stopped moving)
        if (obj.directionH == 0 && obj.directionV == 0) {
            changeState(PlayerIdle::get(), gs.currentStatePlayer, res, obj);
        }
    }
}

void PlayerFly::update(GameObject& obj, float deltaTime) {
    // Apply acceleration based on input direction
    if (obj.directionH != 0) {
        obj.velocity.x += obj.directionH * obj.acceleration.x * deltaTime;
        // Clamp horizontal speed
        if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
            obj.velocity.x = obj.directionH * obj.maxSpeedX;
        }
    }

    if (obj.directionV != 0) {
        obj.velocity.y += obj.directionV * obj.acceleration.y * deltaTime;
        // Clamp vertical speed
        if (std::abs(obj.velocity.y) > obj.maxSpeedY) {
            obj.velocity.y = obj.directionV * obj.maxSpeedY;
        }
    }

    // Update position
    obj.position += obj.velocity * deltaTime;

    if (obj.directionH != 0) {
        obj.visualDirectionH = obj.directionH; // Update visual direction when moving
    }
    // Also update based on velocity for smoother transitions:
    if (obj.velocity.x > 10.0f) {
        obj.visualDirectionH = 1;
    }
    else if (obj.velocity.x < -10.0f) {
        obj.visualDirectionH = -1;
    }
}


// on main we create both newState and currentState FUTURE
bool changeState(ObjectState* newState, ObjectState*& currentState, Resources& res, GameObject& obj) {
    if (newState != nullptr && newState != currentState) {
        currentState = newState;
        currentState->enter(res, obj);
        return true;
    }
    return false;
}
