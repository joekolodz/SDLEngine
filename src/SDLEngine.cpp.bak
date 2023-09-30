#include "image.h"
#include "game_types.h"
#include "game.h"
#include "recording.h"

#include "wtypes.h" //for GetDesktopResolution
#include <iostream>
#include <string>
#include <SDL\SDL.h>
#include <GLEW\glew.h>
#include <SDL\SDL_ttf.h>
#include <SDL\SDL_image.h>


#define refreshRateHz 60
#define gameUpdateHz (refreshRateHz / 1)


uint64 _performanceCounterFrequency;
float _targetSecondsPerFrame = 1.0f/(float)gameUpdateHz;
uint64 _lastframeTimeStart;
float _workProessInput;
float _workGameState;
float _workRender;
float _deltaTimeSeconds;

const int CONTROL_BOX_HEIGHT = 29;
const int USE_FULL_SCREEN = false;
const int TARGET_FRAME_TIME_MILLISECONDS = 16;
bool _isCtrlDown = false;


static bool _isStopRequested = false;

game_window _gameWindow;
file_info exeFileInfo = {};
game_input inputState = {};
InputRecordState _recordingState = None;
SDL_RWops *_recordingFile;
SDL_RWops *titsup;


float GetSecondsElapsed(uint64 startTicks, uint64 endTicks)
{
    uint64 difference = endTicks - startTicks;
    return (float)difference / _performanceCounterFrequency;
}

void GetDesktopResolution(uint32& horizontal, uint32& vertical)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "DPI Coount?");
    SetProcessDPIAware();
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "DPI Set");

    int monitorCount = GetSystemMetrics(SM_CMONITORS);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Monitors: %d", monitorCount);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Getting primary display resolution");
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

bool InitializeSDL()
{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_Log("Startup");

    int sdlResult = SDL_Init(SDL_INIT_EVERYTHING);
    if(sdlResult != 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    sdlResult = TTF_Init();
    if(sdlResult != 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize font library: %s", SDL_GetError());
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if(!(IMG_Init(imgFlags) & imgFlags))
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize image library: %s", SDL_GetError());
        return false;
    }

    _performanceCounterFrequency = SDL_GetPerformanceFrequency();

    return true;
}

bool InitializeWindow()
{
    GetDesktopResolution(_gameWindow.screenWidth, _gameWindow.screenHeight);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    if(USE_FULL_SCREEN)
    {
        _gameWindow.window = SDL_CreateWindow("Asteroids", 0, 0, 0, 0, SDL_WINDOW_OPENGL);
    }
    else
    {
        //TODO forcing this resolution for now
        _gameWindow.screenWidth = 1024;
        _gameWindow.screenHeight = 768;
        _gameWindow.window = SDL_CreateWindow("Asteroids", 0, CONTROL_BOX_HEIGHT, _gameWindow.screenWidth, _gameWindow.screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    }

    if(NULL == _gameWindow.window)
    {
        SDL_Quit();
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to create window: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool InitializeGLContext()
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Creating GL context");
    _gameWindow.glContext = SDL_GL_CreateContext(_gameWindow.window);

    if(NULL == _gameWindow.glContext)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to create GL Context: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool InitializeGlew()
{
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize GLEW");
        return false;
    }
    
    glGetError();
    return true;
}

bool IniitializeRenderer()
{
    _gameWindow.renderer = SDL_CreateRenderer(_gameWindow.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(NULL == _gameWindow.renderer)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to create renderer: %s", SDL_GetError());
        return false;
    }
    return true;
}

void EngineProcessPollEvent(SDL_Event event, game_controller *keyboardController)
{
    //
    //engine controls
    //
    switch(event.type)
    {
        case SDL_KEYDOWN:
            SDL_Log("Input: %d", inputState.windowEvent.key.keysym.sym);

            switch( event.key.keysym.sym )
            {
                case SDLK_ESCAPE:
                    _isStopRequested = true;
                    break;

                case SDLK_LCTRL:
                case SDLK_RCTRL:
                    _isCtrlDown = true;
                    break;

                case SDLK_r:                
                    if(!event.key.repeat && _isCtrlDown)
                    {
                        if(_recordingState == Recording)
                        {
                            _recordingState = None;
                            EndFileAccess(_recordingFile);
                        }
                        else if(_recordingState == None)
                        {
                            _recordingFile = BeginRecordingInput();
                            _recordingState = Recording;
                        }
                    }

                    break;

                case SDLK_p:
                    if(!event.key.repeat && _isCtrlDown)
                    {
                        if(_recordingState == Playback)
                        {
                            _recordingState = None;                            
                            EndFileAccess(_recordingFile);
                        }
                        else if (_recordingState == None)
                        {
                            _recordingFile = BeginReadingInput();
                            _recordingState = Playback;
                        }
                    }
                    break;
            }
            break;

        case SDL_KEYUP:

            switch( event.key.keysym.sym )
            {
                case SDLK_LCTRL:
                case SDLK_RCTRL:
                    _isCtrlDown = false;
                    break;
            }
            break;

        case SDL_QUIT:
            _isStopRequested = true;
            break;

        case SDL_MOUSEMOTION:
            //SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\tMouse: %d, %d", event.motion.x, event.motion.y);
            break;
    }


//
//game controls
//
    switch(event.type)
    {
        case SDL_KEYUP:        
        case SDL_KEYDOWN:
            switch( event.key.keysym.sym )
            {
                case SDLK_UP:
                case SDLK_w:
                    keyboardController->joystickUp.isDown = event.type == SDL_KEYDOWN ? true : false;
                    break;
                case SDLK_DOWN:
                case SDLK_s:
                    keyboardController->joystickDown.isDown = event.type == SDL_KEYDOWN ? true : false;
                    break;
                case SDLK_LEFT:
                case SDLK_a:
                    keyboardController->joystickLeft.isDown = event.type == SDL_KEYDOWN ? true : false;
                    break;
                case SDLK_RIGHT:
                case SDLK_d:
                    keyboardController->joystickRight.isDown = event.type == SDL_KEYDOWN ? true : false;
                    break;
                case SDLK_SPACE:
                    keyboardController->Button1.isDown = event.type == SDL_KEYDOWN ? true : false;
            }
            break;

        case SDL_MOUSEMOTION:
            //SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\tMouse: %d, %d", event.motion.x, event.motion.y);
            break;
    }
}

void EngineProcessInputs()
{
    uint64 startWork = SDL_GetPerformanceCounter();

    SDL_PollEvent(&inputState.windowEvent);
    EngineProcessPollEvent(inputState.windowEvent, &inputState.controllers[0]);
    

    if(_recordingState == Recording)
    {
        RecordInput(_recordingFile, &inputState);
        SDL_Log("RECORDING: %d", (uint32)inputState.windowEvent.key.keysym.sym);
    }
    else if(_recordingState == Playback)
    {        
        ReadRecordedInput(_recordingFile, &inputState, &_recordingState);
        SDL_Log("Read input: %d", inputState.windowEvent.key.keysym.sym);
    }

    _workProessInput = GetSecondsElapsed(startWork, SDL_GetPerformanceCounter()) * 1000.0f;
}

void EngineUpdateAndRenderFrame()
{
    uint64 startWork = SDL_GetPerformanceCounter();
    GameUpdate(_gameWindow, inputState, _deltaTimeSeconds);
    _workRender = GetSecondsElapsed(startWork, SDL_GetPerformanceCounter()) * 1000.0f;
}

static void CalculateTiming()
{
    uint64 frameTimeEnd = SDL_GetPerformanceCounter();
    float frameTimeElapsed = GetSecondsElapsed(_lastframeTimeStart, frameTimeEnd);
    float work = frameTimeElapsed * 1000.0f;

    float sleepTime = 0.0f;
    uint64 sleepTimeStart = SDL_GetPerformanceCounter();
    while(frameTimeElapsed < _targetSecondsPerFrame)
    {
        frameTimeEnd = SDL_GetPerformanceCounter();
        frameTimeElapsed = GetSecondsElapsed(_lastframeTimeStart, frameTimeEnd);
    }
    sleepTime = GetSecondsElapsed(sleepTimeStart, SDL_GetPerformanceCounter()) * 1000.0f;

    float fps = 1.0f / frameTimeElapsed;

    _deltaTimeSeconds = frameTimeElapsed;
    if(_deltaTimeSeconds > 0.04f)
    {
        _deltaTimeSeconds = 0.04f;
    }

    _lastframeTimeStart = SDL_GetPerformanceCounter();
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\nFrame Complete");
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - Input Work:%fms", _workProessInput);
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - GameState Work:%fms", _workGameState);
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - Render Work:%fms", _workRender);
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - Total Work:%fms, Slept for %fms, d(T): %fs, Frame Time:%fms, FPS:%f", work, sleepTime, _deltaTimeSeconds, frameTimeElapsed * 1000.0f, fps);
}

void Shutdown()
{
    SDL_Log("Shutting down");
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(_gameWindow.renderer);
    SDL_GL_DeleteContext(_gameWindow.glContext);
    SDL_DestroyWindow(_gameWindow.window);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    exeFileInfo = {};
    GetExeFileName(&exeFileInfo);

    SDL_Log("FileInfo.fileName:%s", exeFileInfo.absoluteFileName);
    SDL_Log("FileInfo.path:%s", exeFileInfo.path);


    if(!InitializeSDL())
    {
        SDL_Log("Exiting application");
        return 0;
    }

    if(!InitializeWindow())
    {
        SDL_Log("Exiting application");
        return 0;
    }

    // if(!IniitializeRenderer())
    // {
    //     _isStopRequested = false;    
    // }

    if(!InitializeGLContext())
    {
        _isStopRequested = false;
    }

    if(!InitializeGlew())
    {
        _isStopRequested = false;
    }

    glViewport(0, 0, _gameWindow.screenHeight, _gameWindow.screenWidth);  

    while(_isStopRequested == false)
    {        
        EngineProcessInputs();
        
        if(_isStopRequested)
        {
            continue;
        }

        EngineUpdateAndRenderFrame();

        CalculateTiming();
    }

    SDL_Log("Stop requested");

    Shutdown();

    SDL_Log("Exiting application");
    return 0;
}