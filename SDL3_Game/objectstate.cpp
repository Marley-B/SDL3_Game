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
    SDL_Log("Entered idle handle event");

    //If a key was pressed
    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0)
    {
        float currentDirectionH = 0;
        float currentDirectionV = 0;
        switch (e.key.key)
        {
        case SDLK_LEFT: currentDirectionH += -1; break;
        case SDLK_RIGHT: currentDirectionH += 1; break;
        case SDLK_UP: currentDirectionV += -1; break;
        case SDLK_DOWN: currentDirectionV += +1; break;
        }
        obj.directionH = currentDirectionH;
        obj.directionV = currentDirectionV;

        if (currentDirectionH != 0 || currentDirectionV != 0) { // if the player started moving
            changeState(PlayerFly::get(), gs.currentStatePlayer, res, obj);
        }
    }
}

void PlayerIdle::update(GameObject& obj, float deltaTime) {
    // deacelerate
    if (obj.velocity.x) {
        const float factor = obj.velocity.x > 0 ? -1.5f : 1.5f;
        float amount = factor * obj.acceleration.x * deltaTime;
        if (std::abs(obj.velocity.x) < std::abs(amount)) {
            obj.velocity.x = 0;
        }
        else {
            obj.velocity.x += amount;
        }
    }

    // apply gravity
    obj.velocity += glm::vec2(0, 0.5) * deltaTime;
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
        float currentDirectionH = 0;
        float currentDirectionV = 0;
        switch (e.key.key)
        {
        case SDLK_LEFT: currentDirectionH += -1; break;
        case SDLK_RIGHT: currentDirectionH += 1; break;
        case SDLK_UP: currentDirectionV += -1; break;
        case SDLK_DOWN: currentDirectionV += +1; break;
        }
        obj.directionH = currentDirectionH;
        obj.directionV = currentDirectionH;

        if (currentDirectionH == 0 && currentDirectionV == 0) { // if the player stoped moving
            changeState(PlayerIdle::get(), gs.currentStatePlayer, res, obj);
        }
    }
}

void PlayerFly::update(GameObject& obj, float deltaTime) {
    // add acceleration to velocity
    obj.velocity.x += obj.directionH * obj.acceleration.x * deltaTime;
    if (std::abs(obj.velocity.x) > obj.maxSpeedX) {
        obj.velocity.x = obj.directionH * obj.maxSpeedX;
    }
    obj.velocity.y += obj.directionV * obj.acceleration.y * deltaTime;
    if (std::abs(obj.velocity.y) > obj.maxSpeedY) {
        obj.velocity.y = obj.directionV * obj.maxSpeedY;
    }
    // add velocity to position
    obj.position += obj.velocity * deltaTime;
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


//void Dot::handleEvent(SDL_Event& e)
//{
//    //If a key was pressed
//    if (e.type == SDL_EVENT_KEY_DOWN && e.key.repeat == 0)
//    {
//        //Adjust the velocity
//        switch (e.key.key)
//        {
//        case SDLK_UP: mVelY -= kDotVel; break;
//        case SDLK_DOWN: mVelY += kDotVel; break;
//        case SDLK_LEFT: mVelX -= kDotVel; break;
//        case SDLK_RIGHT: mVelX += kDotVel; break;
//        }
//    }
//    //If a key was released
//    else if (e.type == SDL_EVENT_KEY_UP && e.key.repeat == 0)
//    {
//        //Adjust the velocity
//        switch (e.key.key)
//        {
//        case SDLK_UP: mVelY += kDotVel; break;
//        case SDLK_DOWN: mVelY -= kDotVel; break;
//        case SDLK_LEFT: mVelX += kDotVel; break;
//        case SDLK_RIGHT: mVelX -= kDotVel; break;
//        }
//    }
//}
//
//void Dot::move(int levelWidth, int levelHeight)
//{
//    //Move the dot left or right
//    mCollisionBox.x += mVelX;
//
//    //If the dot went too far to the left or right
//    if ((mCollisionBox.x < 0) || (mCollisionBox.x + kDotWidth > levelWidth))
//    {
//        //Move back
//        mCollisionBox.x -= mVelX;
//    }
//
//    //Move the dot up or down
//    mCollisionBox.y += mVelY;
//
//    //If the dot went too far up or down
//    if ((mCollisionBox.y < 0) || (mCollisionBox.y + kDotHeight > levelHeight))
//    {
//        //Move back
//        mCollisionBox.y -= mVelY;
//    }
//}
//
//void Dot::render(SDL_Rect camera)
//{
//    //Show the dot
//    gDotTexture.render(static_cast(mCollisionBox.x) - camera.x, static_cast(mCollisionBox.y) - camera.y);
//}

//void setNextState(ObjectState* newState)
//{
//    //If the user doesn't want to exit
//    if (gNextState != ExitState::get())
//    {
//        //Set the next state
//        gNextState = newState;
//    }
//}