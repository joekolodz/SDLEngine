#include "image.h"
#include "game.h"
#include "game_types.h"
#include "game_component.h"

#include <math.h>
#include <string>
#include <iostream>
#include <SDL\SDL.h>
#include <GLEW\glew.h>
#include <SDL\SDL_ttf.h>

#define ASTEROID_COUNT 20
#define LASER_COUNT 10
#define LASER_COOLDOWN 0.10f
// #define DRAW_COLLIDERS

texture_panel _background;
texture_panel _a_large;
texture_panel _a_medium;
texture_panel _a_small;

struct asteroids
{
    bool isActive[ASTEROID_COUNT];
    texture_panel texture;
    SDL_Rect area[ASTEROID_COUNT];
    rotation_component rotation[ASTEROID_COUNT];
    int forwardSpeed[ASTEROID_COUNT];
    vector2 position[ASTEROID_COUNT];
    float direction[ASTEROID_COUNT];
    circle_collider_component collider[ASTEROID_COUNT];
};
asteroids _asteroids = {};

struct ship
{
    bool isActive;
    float deathCoolDown;
    texture_panel texture;
    animation_component animation;
    movement_component movement;
    float laserCoolDown;
    circle_collider_component collider;
};
ship _ship = {};

struct laser
{
    bool isActive[LASER_COUNT];
    texture_panel texture;
    movement_component movement[LASER_COUNT];
    float timeToLive[LASER_COUNT];
    circle_collider_component collider[LASER_COUNT];
};
laser _laser = {};

game_window _window;

bool _isGameInitialized = false;
bool _resetAsteroidField = false;
int _deadAsteroids = 0;

void InitializeGameState(game_window gameWindow)
{
    if (_isGameInitialized)
        return;

    srand((uint8)time(NULL));

    _isGameInitialized = true;
}

void ProcessInput(game_input input)
{
    // just care about keyboard controller for now. future state will loop through each attached controller
    game_controller keyboard = input.controllers[0];

    _ship.movement.forwardSpeed = 0;

    if (!_ship.isActive)
    {
        return;
    }

    if (keyboard.joystickUp.state == SPressed)
    {
        _ship.movement.forwardSpeed += 180;
    }
    if (keyboard.joystickDown.state == SPressed)
    {
        _ship.movement.forwardSpeed -= 180;
    }
    if (keyboard.joystickLeft.state == SPressed)
    {
        _ship.movement.direction += 0.1f;
    }
    if (keyboard.joystickRight.state == SPressed)
    {
        _ship.movement.direction -= 0.1f;
    }

    if (keyboard.Button1.state == SPressed && _ship.laserCoolDown <= 0.0f)
    {
        _ship.laserCoolDown = LASER_COOLDOWN;

        // find an inactive laser ready to shoot
        for (int i = 0; i < LASER_COUNT; i++)
        {
            if (!_laser.isActive[i])
            {
                _laser.isActive[i] = true;
                _laser.timeToLive[i] = 3.0f;
                _laser.movement[i].direction = _ship.movement.direction;
                _laser.movement[i].position.x = _ship.movement.position.x - (_laser.texture.area.w / 2);
                _laser.movement[i].position.y = _ship.movement.position.y - (_laser.texture.area.h / 2);

                _laser.movement[i].forwardSpeed = 500;

                break;
            }
        }
    }
}

void UpdateState(float deltaTimeSeconds)
{
    if (_ship.laserCoolDown > 0.0f)
    {
        _ship.laserCoolDown -= deltaTimeSeconds;
    }

    for (int i = 0; i < LASER_COUNT; i++)
    {
        if (_laser.isActive)
        {
            _laser.timeToLive[i] -= deltaTimeSeconds;
            if (_laser.timeToLive[i] <= 0.0f)
            {
                _laser.isActive[i] = false;
                _laser.movement[i].forwardSpeed = 0;
            }
            vector2 forward = _laser.movement[i].position.GetForward(_laser.movement[i].direction);
            float f = (float)_laser.movement[i].forwardSpeed * deltaTimeSeconds;
            vector2 magnitude = forward * f;
            _laser.movement[i].position += magnitude;
        }
    }

    // asteroid hits ship detection
    if (_ship.isActive)
    {
        for (int i = 0; i < ASTEROID_COUNT; i++)
        {
            if (_asteroids.isActive[i])
            {
                bool hit = IsIntersect(_asteroids.position[i], _asteroids.collider[i].radius, _ship.movement.position, _ship.collider.radius);
                if (hit)
                {
                    SDL_Log("Ship hit an asteroid!!");
                    _deadAsteroids++;
                    _ship.isActive = false;
                    _ship.movement.forwardSpeed = 0;
                    _ship.movement.momentum = 0;
                    _ship.deathCoolDown = 1.5f;
                    _ship.laserCoolDown = 0.0f;
                    _asteroids.isActive[i] = false;
                }
            }
        }
    }
    else
    {
        _ship.deathCoolDown -= deltaTimeSeconds;
        if (_ship.deathCoolDown <= 0.0f)
        {
            _ship.isActive = true;
            _ship.movement.position = {(float)_window.screenWidth / 2, (float)_window.screenHeight / 2};
            _ship.movement.forwardSpeed = 0;
            _ship.movement.direction = TAU_QUARTER;
            _ship.deathCoolDown = 0.0f;
        }
    }

    // move the ship
    if (_ship.isActive)
    {
        vector2 forward = _ship.movement.position.GetForward(_ship.movement.direction);
        _ship.movement.velocity.x += _ship.movement.forwardSpeed * forward.x * deltaTimeSeconds;
        _ship.movement.velocity.y += _ship.movement.forwardSpeed * forward.y * deltaTimeSeconds;

        if (_ship.movement.forwardSpeed == 0)
        {
            _ship.movement.velocity *= 0.999f;
        }

        _ship.movement.position += _ship.movement.velocity * deltaTimeSeconds;

        // (Screen wrapping code only for ship)
        if (_ship.movement.position.x < -15.0f)
        {
            _ship.movement.position.x = 1024.0f + 4;
        }
        else if (_ship.movement.position.x > 1024.0f + 4)
        {
            _ship.movement.position.x = -14.0f;
        }

        if (_ship.movement.position.y < -25.0f)
        {
            _ship.movement.position.y = 768.0f + 4;
        }
        else if (_ship.movement.position.y > 768.0f + 4)
        {
            _ship.movement.position.y = -24.0f;
        }
    }

    // move the asteroids
    for (int i = 0; i < ASTEROID_COUNT; i++)
    {
        _asteroids.rotation[i].rotationAngle += (_asteroids.rotation[i].rotationSpeed * deltaTimeSeconds);
        vector2 forward = _asteroids.position[i].GetForward(_asteroids.direction[i]);
        float f = (float)_asteroids.forwardSpeed[i] * deltaTimeSeconds;
        vector2 magnitude = forward * f;
        _asteroids.position[i] += magnitude;

        // (Screen wrapping code only for asteroids)
        if (_asteroids.position[i].x < -120.0f)
        {
            _asteroids.position[i].x = 1030.0f;
        }
        else if (_asteroids.position[i].x > 1030.0f)
        {
            _asteroids.position[i].x = -120.0f;
        }

        if (_asteroids.position[i].y < -120.0f)
        {
            _asteroids.position[i].y = 776.0f;
        }
        else if (_asteroids.position[i].y > 776.0f)
        {
            _asteroids.position[i].y = -120.0f;
        }

        //_asteroids.area[i].x = (int)roundf(_asteroids.position[i].x);
        //_asteroids.area[i].y = (int)roundf(_asteroids.position[i].y);
    }

    // lazer collision check with asteroids
    for (int n = 0; n < LASER_COUNT; n++)
    {
        if (_laser.isActive[n])
        {
            for (int i = 0; i < ASTEROID_COUNT; i++)
            {
                if (_asteroids.isActive[i])
                {
                    bool hit = IsIntersect(_asteroids.position[i], _asteroids.collider[i].radius, _laser.movement[n].position, _laser.collider[n].radius);
                    if (hit)
                    {
                        _deadAsteroids++;
                        SDL_Log("Laser hit the asteroid!! %d", _deadAsteroids);
                        _laser.isActive[n] = false;
                        _laser.movement[n].forwardSpeed = 0;
                        _laser.timeToLive[n] = 0;
                        _asteroids.isActive[i] = false;
                    }
                }
            }
        }
    }

    // reset the asteroid field when all are dead
    if (_deadAsteroids == ASTEROID_COUNT)
    {
        // below is just copied from the initialize method, so compress
        float scale;
        for (int i = 0; i < ASTEROID_COUNT; i++)
        {
            _asteroids.isActive[i] = true;
            scale = 0.10f + GetRandomFloat(0.05f, 1.0f);
            _asteroids.area[i].w = int(124 * scale);
            _asteroids.area[i].h = int(103 * scale);
            _asteroids.area[i].x = GetRandomInt(0, 1024);
            _asteroids.area[i].y = GetRandomInt(0, 768);
            _asteroids.collider[i].radius = (float)_asteroids.area[i].w / 2; // taking Y because it is larger on average than height

            // don't place an asteroid near the ship
            if ((_asteroids.area[i].x < ((int)_window.screenWidth / 2) + 100) && (_asteroids.area[i].x > ((int)_window.screenWidth / 2) - 100) &&
                (_asteroids.area[i].y < ((int)_window.screenHeight / 2) + 100) && (_asteroids.area[i].y > ((int)_window.screenHeight / 2) - 100))
            {
                int push = GetRandomInt(100, 200);
                _asteroids.area[i].x = push + (int)_window.screenWidth / 2;
                _asteroids.area[i].y = push + (int)_window.screenHeight / 2;
                SDL_Log("Pushing an asteroid away");
            }

            _asteroids.rotation[i].rotationAngle = GetRandomFloat(0.0f, TAU);
            _asteroids.rotation[i].rotationSpeed = GetRandomFloat(0.15f, 0.7f);
            _asteroids.rotation[i].rotationSpeed *= CoinFlip() > 50 ? 1 : -1;

            _asteroids.direction[i] = _asteroids.rotation[i].rotationAngle;

            _asteroids.forwardSpeed[i] = GetRandomInt(80, 150);
        }
        _deadAsteroids = 0;
        SDL_Log("Asteroid field reset %d", _deadAsteroids);
    }
}

animation_component _test = {};

void putpixel(int x, int y)
{
    SDL_RenderDrawPoint(_window.renderer, x, y);
}

void drawcircle(int x0, int y0, int radius)
{
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    while (x >= y)
    {
        putpixel(x0 + x, y0 + y);
        putpixel(x0 + y, y0 + x);
        putpixel(x0 - y, y0 + x);
        putpixel(x0 - x, y0 + y);
        putpixel(x0 - x, y0 - y);
        putpixel(x0 - y, y0 - x);
        putpixel(x0 + y, y0 - x);
        putpixel(x0 + x, y0 - y);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }

        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

float _a = 0.0f;
float _x = 512.0f;
float _y = 384.0f;

void RenderFrame(float deltaTimeSeconds)
{
    SDL_SetRenderDrawColor(_window.renderer, 20, 20, 20, 255);
    SDL_RenderClear(_window.renderer);
    SDL_RenderCopy(_window.renderer, _background.texture, NULL, &_background.area);

    if (_ship.isActive)
    {
        _ship.texture.area.x = (int)_ship.movement.position.x - (_ship.texture.area.w / 2);
        _ship.texture.area.y = (int)_ship.movement.position.y - (_ship.texture.area.h / 2);
        SDL_RenderCopyEx(_window.renderer, _ship.texture.texture, NULL, &_ship.texture.area, -GetDegreesFromRadians(_ship.movement.direction - (float)TAU_QUARTER), nullptr, SDL_FLIP_NONE);

#ifdef DRAW_COLLIDERS
        SDL_SetRenderDrawColor(_window.renderer, 255, 20, 20, 255);
        for (float a = 0.0f; a < TAU; a += 0.01f)
        {
            vector2 forward = _ship.movement.position.GetForward(a);
            float x = _ship.movement.position.x + _ship.collider.radius * forward.x;
            float y = _ship.movement.position.y + _ship.collider.radius * forward.y;
            SDL_RenderDrawPointF(_window.renderer, x, y);
        }
#endif
    }

    for (int i = 0; i < LASER_COUNT; i++)
    {
        if (_laser.isActive[i])
        {
            // might have to have a stand alone SDL_Rect to hold each position
            _laser.texture.area.x = (int)_laser.movement[i].position.x;
            _laser.texture.area.y = (int)_laser.movement[i].position.y;
            SDL_RenderCopyEx(_window.renderer, _laser.texture.texture, NULL, &_laser.texture.area, -GetDegreesFromRadians(_laser.movement[i].direction - (float)TAU_QUARTER), nullptr, SDL_FLIP_NONE);
        }
    }

    // loop through all the asteroids
    for (int i = 0; i < ASTEROID_COUNT; i++)
    {
        if (_asteroids.isActive[i])
        {
            _asteroids.area[i].x = (int)_asteroids.position[i].x - (_asteroids.area[i].w / 2);
            _asteroids.area[i].y = (int)_asteroids.position[i].y - (_asteroids.area[i].h / 2);
            SDL_RenderCopyEx(_window.renderer, _asteroids.texture.texture, NULL, &_asteroids.area[i], -GetDegreesFromRadians(_asteroids.rotation[i].rotationAngle), nullptr, SDL_FLIP_NONE);
        }
    }

#ifdef DRAW_COLLIDERS
    SDL_SetRenderDrawColor(_window.renderer, 20, 255, 20, 255);
    for (int i = 0; i < ASTEROID_COUNT; i++)
    {
        if (_asteroids.isActive[i])
        {
            for (float a = 0.0f; a < TAU; a += 0.01f)
            {
                vector2 forward = _asteroids.position[i].GetForward(a);
                float x = _asteroids.position[i].x + _asteroids.collider[i].radius * forward.x;
                float y = _asteroids.position[i].y + _asteroids.collider[i].radius * forward.y;
                SDL_RenderDrawPointF(_window.renderer, x, y);
            }
        }
    }
#endif

    SDL_RenderPresent(_window.renderer);
}

void LoadImage(texture_panel &item, const char *file)
{
    item.texture = LoadTexture(_window.renderer, file);
    item.area = {0, 0, 32, 32};
}

void TestAnimation()
{
    LoadImage(_test.line[0], "D:\\Development\\SDLEngine\\assets\\images\\line_test_001.png");
    LoadImage(_test.line[1], "D:\\Development\\SDLEngine\\assets\\images\\line_test_002.png");
    LoadImage(_test.line[2], "D:\\Development\\SDLEngine\\assets\\images\\line_test_003.png");
    LoadImage(_test.line[3], "D:\\Development\\SDLEngine\\assets\\images\\line_test_004.png");
    LoadImage(_test.line[4], "D:\\Development\\SDLEngine\\assets\\images\\line_test_005.png");
    LoadImage(_test.line[5], "D:\\Development\\SDLEngine\\assets\\images\\line_test_006.png");
    LoadImage(_test.line[6], "D:\\Development\\SDLEngine\\assets\\images\\line_test_007.png");
    LoadImage(_test.line[7], "D:\\Development\\SDLEngine\\assets\\images\\line_test_008.png");
    LoadImage(_test.line[8], "D:\\Development\\SDLEngine\\assets\\images\\line_test_009.png");
    LoadImage(_test.line[9], "D:\\Development\\SDLEngine\\assets\\images\\line_test_010.png");

    LoadImage(_test.line[10], "D:\\Development\\SDLEngine\\assets\\images\\line_test_011.png");
    LoadImage(_test.line[11], "D:\\Development\\SDLEngine\\assets\\images\\line_test_012.png");
    LoadImage(_test.line[12], "D:\\Development\\SDLEngine\\assets\\images\\line_test_013.png");
    LoadImage(_test.line[13], "D:\\Development\\SDLEngine\\assets\\images\\line_test_014.png");
    LoadImage(_test.line[14], "D:\\Development\\SDLEngine\\assets\\images\\line_test_015.png");
    LoadImage(_test.line[15], "D:\\Development\\SDLEngine\\assets\\images\\line_test_016.png");
    LoadImage(_test.line[16], "D:\\Development\\SDLEngine\\assets\\images\\line_test_017.png");
    LoadImage(_test.line[17], "D:\\Development\\SDLEngine\\assets\\images\\line_test_018.png");
    LoadImage(_test.line[18], "D:\\Development\\SDLEngine\\assets\\images\\line_test_019.png");
    LoadImage(_test.line[19], "D:\\Development\\SDLEngine\\assets\\images\\line_test_020.png");

    LoadImage(_test.line[20], "D:\\Development\\SDLEngine\\assets\\images\\line_test_021.png");
    LoadImage(_test.line[21], "D:\\Development\\SDLEngine\\assets\\images\\line_test_022.png");
    LoadImage(_test.line[22], "D:\\Development\\SDLEngine\\assets\\images\\line_test_023.png");
    LoadImage(_test.line[23], "D:\\Development\\SDLEngine\\assets\\images\\line_test_024.png");
    LoadImage(_test.line[24], "D:\\Development\\SDLEngine\\assets\\images\\line_test_025.png");
    LoadImage(_test.line[25], "D:\\Development\\SDLEngine\\assets\\images\\line_test_026.png");
    LoadImage(_test.line[26], "D:\\Development\\SDLEngine\\assets\\images\\line_test_027.png");
    LoadImage(_test.line[27], "D:\\Development\\SDLEngine\\assets\\images\\line_test_028.png");
    LoadImage(_test.line[28], "D:\\Development\\SDLEngine\\assets\\images\\line_test_029.png");
    LoadImage(_test.line[29], "D:\\Development\\SDLEngine\\assets\\images\\line_test_030.png");
}

void GameUpdate(game_window gameWindow, game_input inputState, float deltaTimeSeconds)
{
    _window = gameWindow;

    if (!_isGameInitialized)
    {
        InitializeGameState(gameWindow);
    }

    ProcessInput(inputState);
    UpdateState(deltaTimeSeconds);
    RenderFrame(deltaTimeSeconds);
}
