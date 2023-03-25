#include "game_types.h"
#include <SDL\SDL.h>
#include <iostream>

vector2 ToNormal(vector2 a)
{
    vector2 temp;
    float length = a.Length();
    temp.x = a.x / length;
    temp.y = a.y / length;
    return temp;
}

void DrawRectangle_2(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, float x, float y, int width, int height)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_Rect rect{
        static_cast<int>(x),
        static_cast<int>(y),
        width,
        height
        };
    SDL_RenderFillRect(renderer, &rect);
}

int GetRandomInt(int min, int max)
{
   return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

float GetRandomFloat(float min, float max)
{
    //use double during the math to get better precision than float
    double scale = (double)rand() / (double) RAND_MAX; /* [0, 1.0] */
    return (float)(min + scale * ( max - min ));      /* [min, max] */
}

int CoinFlip()
{
   return GetRandomInt(0, 100);
}


int GetDegreesFromRadians(float radians)
{
    return (int)(radians * ONE_RADIAN_IN_DEGREES);
}