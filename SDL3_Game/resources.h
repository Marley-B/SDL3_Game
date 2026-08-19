#pragma once
#include <vector>
#include <filesystem>
#include <string>
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "animation.h"  
#include "tmx.h"
#include "sdlstate.h"

struct TileSetTextures
{
    int firstGid;
    std::vector<SDL_Texture*> textures;
};

struct Resources {
    const int ANIM_PLAYER_IDLE = 0;
    const int ANIM_PLAYER_RUN = 1;
    const int ANIM_PLAYER_SLIDE = 2;

    std::vector<Animation> playerAnims;
    std::vector<SDL_Texture*> textures;
    SDL_Texture* texIdle, * texRun, * texSlide, * texBg1, * texBg2, * texBg3, * texBg4, *texButtHov, *texButtDown, *texButt, 
        * texGameOverScreen, * texIntroScreen, * texLevelMenuScreen, * texWinScreen;

    std::vector<MIX_Audio*> audioEffects;
    MIX_Audio* audioShoot, * audioShootHit, * audioEnemyHit;
    std::vector<MIX_Track*> tracks;
    MIX_Track* musicMain;

    std::vector<TileSetTextures> tilesetTextures;
    std::vector<std::unique_ptr<tmx::Map>> maps;
    std::unique_ptr<tmx::Map> map1;
    std::unique_ptr<tmx::Map> map2;

    // Method declarations
    SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& filepath);
    MIX_Audio* loadAudio(MIX_Mixer* mixer, const std::string& filepath);
    MIX_Track* loadTrack(MIX_Mixer* mixer, const std::string& filepath);
    void load(SDLState& state);
    void unload();
};

