#if !defined(IMAGE_H)

#include <SDL\SDL.h>

struct texture_panel
{
    SDL_Texture *texture;
    SDL_Rect area;
};


SDL_Texture* LoadTexture(SDL_Renderer *renderer, const char* fileName);

void DrawTexture(texture_panel panel);

#define IMAGE_H
#endif