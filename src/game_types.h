#if !defined(GAME_TYPES_H)

#include <stdint.h>
#include <SDL\SDL_ttf.h>


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#define PI 3.141592653589793
#define TAU 6.283185307179586
#define TAU_QUARTER 1.570796326794897
#define ONE_RADIAN_IN_DEGREES 57.29577951308233 //360 degrees divided by TAU

struct game_window
{
    uint32 screenWidth;
    uint32 screenHeight;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GLContext glContext;
};

enum button_states
{
    SNone,
    SPressed,
    SReleased,
    SHeld
};

struct game_controller_button_state
{
    bool isDown;
    bool wasDown;
    button_states state;
};

struct game_controller
{
    game_controller_button_state joystickUp;
    game_controller_button_state joystickDown;
    game_controller_button_state joystickLeft;
    game_controller_button_state joystickRight;
    game_controller_button_state Button1;
};

struct game_input
{
    SDL_Event windowEvent;
    
    uint32 mouseX;
    uint32 mouseY;
    uint32 mouseZ; //scroll wheel
    game_controller_button_state mouseButtons[3];

    game_controller controllers[3];    //0 = keyboard, 1 = controller, 2 = controller
};

struct vector2
{
    float x;
    float y;

    friend vector2 operator + (const vector2 &left, const vector2 &right)
    {
        vector2 temp;
        temp.x = left.x + right.x;
        temp.y = left.y + right.y;
        return temp;
    }

    vector2& operator += (const vector2 &right)
    {
        x += right.x;
        y += right.y;
        return *this;
    }

    vector2 operator + (const float right)
    {
        vector2 temp;
        temp.x = x + right;
        temp.y = y + right;        
        return temp;
    }



    friend vector2 operator - (const vector2 &left, const vector2 &right)
    {
        vector2 temp;
        temp.x = left.x - right.x;
        temp.y = left.y - right.y;
        return temp;
    }

    vector2& operator -= (vector2 &right)
    {
        x -= right.x;
        y -= right.y;
        return *this;
    }

    vector2 operator - (const float right)
    {
        vector2 temp;
        temp.x = x - right;
        temp.y = y - right;        
        return temp;
    }




    friend vector2 operator * (const vector2& left, const vector2& right)
    {
        vector2 result;
        result.x = left.x * right.x;
        result.y = left.y * right.y;
        return result;
    }

    friend vector2 operator * (const vector2& v, float scalar)
    {
        vector2 temp;
        temp.x = v.x * scalar;
        temp.y = v.y * scalar;
        return temp;
    }

    friend vector2 operator * (float scalar, const vector2& v)
    {
        vector2 temp;
        temp.x = v.x * scalar;
        temp.y = v.y * scalar;
        return temp;
    }

    friend vector2 operator *= (const vector2& left, const vector2& right)
    {
        vector2 result;
        result.x = left.x * right.x;
        result.y = left.y * right.y;
        return result;
    }

    vector2& operator *= (const float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    friend vector2 operator / (const vector2& v, float scalar)
    {
        vector2 temp;
        temp.x = v.x / scalar;
        temp.y = v.y / scalar;
        return temp;
    }

    float Length()
    {
        float result;
        float squares = (x * x) + (y * y);
        result = sqrtf(squares);
        return result;
    }

    float LengthSquared()
    {
        float squares = (x * x) + (y * y);
        return squares;
    }

    float DistanceTo(const vector2& destination)
    {
        float result;
        vector2 me = {x, y};
        vector2 temp = destination - me;
        result = temp.Length();
        return result;
    }

    void Normalize()
    {
        float length = Length();
        x = x / length;
        y = y / length;
    }

    vector2 GetForward(float angleRadians)
    {
        vector2 temp;
        temp.x = cosf(angleRadians);
        temp.y = -sinf(angleRadians);
        return temp;
    }

    double GetAngleFromForward()
    {
        return atan2(y, x);
    }

    float DotProduct(vector2& left, vector2& right)
    {
        vector2 result;
        result.x = left.x * right.x;
        result.y = left.y * right.y;
        return result.x + result.y;
    }

    void print()
    {
        print("");
    }

    void print(const char* label)
    {
        SDL_Log("%s - x:%f, y:%f", label, x, y);
    }
};

vector2 ToNormal(vector2 a);

void DrawRectangle_2(SDL_Renderer *renderer, Uint8 r, Uint8 g, Uint8 b, float x, float y, int width, int height);

int GetRandomInt(int low, int high);

float GetRandomFloat(float low, float high);

int GetDegreesFromRadians(float radians);

int CoinFlip();

#define GAME_TYPES_H
#endif