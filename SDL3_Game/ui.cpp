#include "ui.h"

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
    //TTF_Text* mana = TTF_CreateText()
    //TTF_DrawRendererText("Mana", mPosition.x + 20, mPosition.y + 20);
}

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
    SDL_DestroyTexture(textTexture); // Clean up
}
SDL_Texture* TextRenderer::crateTexture(SDLState& state, const std::string& text, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, text.c_str(), text.size(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);
    SDL_DestroySurface(textSurface);
    return textTexture;
}

SDL_Texture* TextRenderer::crateTextureVar(SDLState& state, const std::string& text, auto var, SDL_Color color) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s%d", text.c_str(), var);
    SDL_Surface* textSurface = TTF_RenderText_Solid(mFont, buffer, 64, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(state.renderer, textSurface);
    SDL_DestroySurface(textSurface);
    return textTexture;
}