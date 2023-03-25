#include "image.h"
#include <string>
#include <SDL\SDL.h>
#include <GLEW\glew.h>
#include <SDL\SDL_image.h>

SDL_Texture* LoadTexture(SDL_Renderer *renderer, const char* fileName)
{
    SDL_Surface *surface = IMG_Load(fileName);
    if(!surface)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image because a surface could not be created: %s", fileName);
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if(!texture)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to load image because texture couldn't be created from surface: %s", fileName);
        return nullptr;
    }

    return texture;
}

void DrawTexture(SDL_Renderer *renderer, texture_panel panel)
{
    // int width;
    // int height;
    // float scaleWidth = 1.0f;
    // float scaleHeight = 1.0f;

    // SDL_QueryTexture(panel.texture, 0, 0, &width, &height);
    // panel.area.w = static_cast<int>(width * scaleWidth);
    // panel.area.h = static_cast<int>(height * scaleHeight);
    // panel.area.x = 0;
    // panel.area.y = 0;

    SDL_RenderCopyEx(renderer, panel.texture, 0, &panel.area, 0, 0, SDL_FLIP_NONE);    
}