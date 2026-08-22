#include "ui.h"

StaminaUi::StaminaUi(Resources& res) :
    mPosition{ 0.f, 0.f },
    mCurrentTexture{ res.texStGreen },
    mBowTexture{res.texStBox}
{
}

void StaminaUi::setPosition(float x, float y) {
    mPosition.x = x;
    mPosition.y = y;
}

void StaminaUi::update(Resources& res, GameState& gs) {
    //SDL_Log("Update");
    if (gs.player().data.player.staminaPoints >= gs.player().data.player.maxStamina * 0.75) {
        //SDL_Log("Green");
        mCurrentTexture = res.texStGreen;
    }
    else if (gs.player().data.player.staminaPoints >= gs.player().data.player.maxStamina * 0.25) {
        //SDL_Log("Yellow");
        mCurrentTexture = res.texStYellow;
    }
    else if (gs.player().data.player.staminaPoints <= gs.player().data.player.maxStamina * 0.25) {
        //SDL_Log("Red");
        mCurrentTexture = res.texStRed;
    }
}

void StaminaUi::render(SDLState& state, GameState& gs, GameObject& obj) {

    SDL_FRect dstI{ // Where on screen to render the sprite
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kStaminaWidth * abs(gs.player().data.player.staminaPoints / gs.player().data.player.maxStamina), // showing ip out of box
        .h = kStaminaHeight
    };
    SDL_RenderTexture(state.renderer, mCurrentTexture, nullptr, &dstI);

    SDL_FRect dstB{ // Where on screen to render the sprite
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kStaminaWidth,
        .h = kStaminaHeight
    };
    SDL_RenderTexture(state.renderer, mBowTexture, nullptr, &dstB);
}