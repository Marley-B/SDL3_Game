#include "ui.h"


// button related functions

Button::Button(Resources& res) :
    mPosition{ 0.f, 0.f },
    mCurrentTexture{ res.texButt },
    mStarTexture{ res.texBigButterfly }
{
}

void Button::setPosition(float x, float y) {
    mPosition.x = x;
    mPosition.y = y;
}

void Button::setDimensions(float w, float h) {
    kButtonWidth = w;
    kButtonHeight = h;
}

void Button::setText(SDLState& state, const std::string& text, SDL_Color color) {
    mText = state.text->crateTexture(state, text, color);
}

bool Button::handleEvent(SDL_Event* e, Resources& res, SDLState& state) {
    //If mouse event happened
    if (e->type == SDL_EVENT_MOUSE_MOTION || e->type == SDL_EVENT_MOUSE_BUTTON_DOWN || e->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        //Get mouse position
        float x = -1.f, y = -1.f;
        SDL_GetMouseState(&x, &y);

        float logicalMouseX = (x / (float)state.width) * (float)state.logW;
        float logicalMouseY = (y / (float)state.height) * (float)state.logH;

        //Check if mouse is in button
        bool inside = true;

        if (logicalMouseX < mPosition.x) { // left of the button
            inside = false;
        }
        else if (logicalMouseX > mPosition.x + kButtonWidth) { // right of the button
            inside = false;
        }
        else if (logicalMouseY < mPosition.y) { // above the button
            inside = false;
        }
        else if (logicalMouseY > mPosition.y + kButtonHeight) { //  below the button
            inside = false;
        }

        if (!inside) { //Mouse is outside button
            mCurrentTexture = res.texButt;
        }
        else { //Mouse is inside button
            //Set mouse over sprite
            switch (e->type) {
            case SDL_EVENT_MOUSE_MOTION:
                mCurrentTexture = res.texButtHov;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                mCurrentTexture = res.texButtDown;
                return true; // return true if the mouse is pressed
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                mCurrentTexture = res.texButtHov;
                break;
            }
        }
    }
    return false;
}

void Button::render(SDLState& state, GameState& gs) {
    SDL_FRect dst{ // Where on screen to render the sprite
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kButtonWidth,
        .h = kButtonHeight
    };
    SDL_RenderTexture(state.renderer, mCurrentTexture, nullptr, &dst);

    float centerX = mPosition.x + kButtonWidth / 2;
    float centerY = mPosition.y + kButtonHeight / 2;
    state.text->renderTextTexture(mText, state, mPosition.x, mPosition.y,
        true, centerX, centerY);
}

void Button::renderStar(SDLState& state) {

    SDL_FRect src{ // Portion of the texture to copy
        .x = 0,
        .y = 0,
        .w = 32,
        .h = 32
    };

    SDL_FRect dst{ // Where on screen to render the sprite
        .x = mPosition.x,
        .y = mPosition.y - 10,
        .w = (kButtonWidth/3),
        .h = (kButtonWidth / 3)
    };
    SDL_RenderTexture(state.renderer, mStarTexture, &src, &dst);
}

Button::~Button() {
    if (mText) {
        SDL_DestroyTexture(mText);
        mText = nullptr;
    }
}

// Stamina functions

StaminaUi::StaminaUi(Resources& res) :
    mPosition{ 0.f, 0.f },
    mCurrentTexture{ res.texStGreen },
    mBoxTexture{res.texStBox},
    mBgTexture{res.texStBg}
{
}

void StaminaUi::setPosition(float x, float y) {
    mPosition.x = x;
    mPosition.y = y;
}

void StaminaUi::update(Resources& res, GameState& gs) {
    if (gs.player().data.player.staminaPoints >= gs.player().data.player.maxStamina * 0.75) {
        mCurrentTexture = res.texStGreen;
    }
    else if (gs.player().data.player.staminaPoints >= gs.player().data.player.maxStamina * 0.25) {
        mCurrentTexture = res.texStYellow;
    }
    else if (gs.player().data.player.staminaPoints <= gs.player().data.player.maxStamina * 0.25) {
        mCurrentTexture = res.texStRed;
    }
}

void StaminaUi::render(SDLState& state, GameState& gs, GameObject& obj, Resources& res) {

    float percentage = ((float)gs.player().data.player.staminaPoints / (float)gs.player().data.player.maxStamina) > 0 ?
        ((float)gs.player().data.player.staminaPoints / (float)gs.player().data.player.maxStamina) : 0;

    SDL_FRect dstBg{ // Background of stamina bar
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kStaminaWidth,
        .h = kStaminaHeight
    };
    SDL_RenderTexture(state.renderer, mBgTexture, nullptr, &dstBg);

    float texWidth, texHeight;
    SDL_GetTextureSize(mCurrentTexture, &texWidth, &texHeight);

    SDL_FRect src{
        .x = 0.0f,
        .y = 0.0f,
        .w = texWidth * percentage,
        .h = static_cast<float>(texHeight)
    };

    SDL_FRect dstI{ // Bar progres
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kStaminaWidth * percentage,
        .h = kStaminaHeight
    };
    SDL_RenderTexture(state.renderer, mCurrentTexture, &src, &dstI);

    SDL_FRect dstBox{ // Outside box
        .x = mPosition.x,
        .y = mPosition.y,
        .w = kStaminaWidth,
        .h = kStaminaHeight
    };
    SDL_RenderTexture(state.renderer, mBoxTexture, nullptr, &dstBox);

    SDL_Color color = { 0, 0, 0, 0 };
    float centerX = mPosition.x + kStaminaWidth / 2;
    float centerY = mPosition.y + kStaminaHeight / 2;
    state.text->renderText(state, "Mana", mPosition.x, mPosition.y, color, true, centerX, centerY);
}

// Coin couter

CoinUi::CoinUi() :
    mPosition{ 0.f, 0.f }
{
}

void CoinUi::setPosition(float x, float y) {
    mPosition.x = x;
    mPosition.y = y;
}

void CoinUi::render(SDLState& state, GameState& gs) {
    SDL_Color color = { 255, 255, 255, 255 };
    state.text->renderTextVar(state, "Butterfly", gs.player().data.player.collectedCoins, mPosition.x, mPosition.y, color, false, 0, 0);
}

void CoinUi::renderWithCount(SDLState& state, int count) {
    SDL_Color color = { 10, 0, 255, 255 };
    state.text->renderTextVar(state, "Butterflies", count, mPosition.x, mPosition.y, color, false, 0, 0);
}

// text renderer

TextRenderer::TextRenderer(Resources& res) :
    mFont{ res.font }
{
}

void TextRenderer::renderText(SDLState& state, const std::string& text, float x, float y, SDL_Color color,
    bool centered, float centerX, float centerY){

    SDL_Texture* textTexture = crateTexture(state, text, color);
    if (!textTexture) {
        return;
    }

    // Get texture dimensions
    float textWidth, textHeight;
    SDL_GetTextureSize(textTexture, &textWidth, &textHeight);

    // Calculate position
    float renderX = x;
    float renderY = y;
    if (centered) {
        renderX = centerX - (textWidth / 2.0f);
        renderY = centerY - (textHeight / 2.0f);
    }
    SDL_FRect dstRect{
        .x = renderX,
        .y = renderY,
        .w = textWidth,
        .h = textHeight
    };

    SDL_RenderTexture(state.renderer, textTexture, nullptr, &dstRect); // Render texture
    SDL_DestroyTexture(textTexture); // Clean up
}

void TextRenderer::renderTextVar(SDLState& state, const std::string& text, auto var, float x, float y, SDL_Color color,
    bool centered, float centerX, float centerY) {

    SDL_Texture* textTexture = crateTextureVar(state, text, var, color);
    if (!textTexture) {
        return;
    }

    // Get texture dimensions
    float textWidth, textHeight;
    SDL_GetTextureSize(textTexture, &textWidth, &textHeight);

    // Calculate position
    float renderX = x;
    float renderY = y;
    if (centered) {
        renderX = centerX - (textWidth / 2.0f);
        renderY = centerY - (textHeight / 2.0f);
    }
    SDL_FRect dstRect{
        .x = renderX,
        .y = renderY,
        .w = textWidth,
        .h = textHeight
    };

    SDL_RenderTexture(state.renderer, textTexture, nullptr, &dstRect); // Render texture
    SDL_DestroyTexture(textTexture); // Clean up
}

void TextRenderer::renderTextTexture(SDL_Texture* textTexture, SDLState& state, float x, float y,
    bool centered, float centerX, float centerY) {

    // Get texture dimensions
    float textWidth, textHeight;
    SDL_GetTextureSize(textTexture, &textWidth, &textHeight);

    // Calculate position
    float renderX = x;
    float renderY = y;
    if (centered) {
        renderX = centerX - (textWidth / 2.0f);
        renderY = centerY - (textHeight / 2.0f);
    }
    SDL_FRect dstRect{
        .x = renderX,
        .y = renderY,
        .w = textWidth,
        .h = textHeight
    };

    SDL_RenderTexture(state.renderer, textTexture, nullptr, &dstRect); // Render texture
}
SDL_Texture* TextRenderer::crateTexture(SDLState& state, const std::string& text, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, text.c_str(), text.size(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);
    SDL_DestroySurface(textSurface);
    return textTexture;
}

SDL_Texture* TextRenderer::crateTextureVar(SDLState& state, const std::string& text, auto var, SDL_Color color) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s: %d", text.c_str(), (int)trunc(var));
    SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, buffer, strlen(buffer), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);
    SDL_DestroySurface(textSurface);
    return textTexture;
}