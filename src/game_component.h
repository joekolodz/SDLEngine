#if !defined(GAME_COMPONENT_H)

#include "game_types.h"
#include "image.h"

struct rotation_component
{
    float rotationAngle;//radians
    float rotationSpeed;
};

struct movement_component
{
    int forwardSpeed;
    vector2 position;
    float direction;

    vector2 acceleration;
    vector2 sumOfForces;
    float mass;
    vector2 velocity;
};

struct animation_component
{
    float frameIndex = 0;
    float animationFPS = 30;
    int16 animationFrameSize = 30;
    texture_panel line[30];

    void Animate(SDL_Renderer *renderer, float deltaTimeSeconds)
    {
        frameIndex += animationFPS * deltaTimeSeconds;
        if(frameIndex >= animationFrameSize)
        {
            frameIndex = 0;
        }

        texture_panel x = line[static_cast<int>(frameIndex)];
        SDL_RenderCopy(renderer, x.texture, NULL, &x.area);
    }
};

struct circle_collider_component
{
    float radius;
};

bool IsIntersect(vector2 a, float radiusA, vector2 b, float radiusB)
{
    vector2 diff = b - a;
    float distSq = diff.LengthSquared();
    float radiusSq = radiusA + radiusB;
    radiusSq *= radiusSq;
    return distSq < radiusSq;
}

#define GAME_COMPONENT_H
#endif