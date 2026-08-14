#pragma once

class PlayerState {
	virtual void enter() = 0; // load tex and animations
	virtual void handleEvent(SDL_Event& e) = 0; // handle key inputs
	virtual void update(float deltaTime) = 0; // if x happens -> change state
};

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

//void setNextState(GameState* newState)
//{
//    //If the user doesn't want to exit
//    if (gNextState != ExitState::get())
//    {
//        //Set the next state
//        gNextState = newState;
//    }
//}