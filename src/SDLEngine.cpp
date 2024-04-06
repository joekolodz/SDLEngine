// This is coming from MAIN - merge me plase

#include "image.h"
#include "game_types.h"
#include "game.h"
#include "recording.h"

#include "wtypes.h" //for GetDesktopResolution
#include <iostream>
#include <string>

#include "..\imgui\imgui.h"
#include "..\imgui\imgui_impl_sdl2.h"
#include "..\imgui\imgui_impl_opengl3.h"

#include <SDL\SDL.h>
#include <GLEW\glew.h>
#include <SDL\SDL_ttf.h>
#include <SDL\SDL_image.h>
#include <SDL\SDL_opengl.h>

#define refreshRateHz 60
#define gameUpdateHz (refreshRateHz / 1)

uint64 _performanceCounterFrequency;
float _targetSecondsPerFrame = 1.0f / (float)gameUpdateHz;
uint64 _lastframeTimeStart;
float _workProcessInput;
float _workGameState;
float _workRender;
float _deltaTimeSeconds;
float _frameTimeElapsedSeconds;
float _fps;
float _lockedfps;
float _lockedFrameTimeElapsedSeconds;

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

//
// ImGui
//
ImGuiIO *_ioMetrics;

float GetElapsedSeconds(uint64 startTicks, uint64 endTicks)
{
    return (endTicks - startTicks) / (float)_performanceCounterFrequency;
}

void GetDesktopResolution(uint32 &horizontal, uint32 &vertical)
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
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    SDL_Log("Startup");

    int sdlResult = SDL_Init(SDL_INIT_EVERYTHING);
    if (sdlResult != 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    sdlResult = TTF_Init();
    if (sdlResult != 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize font library: %s", SDL_GetError());
        return false;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags))
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

    if (USE_FULL_SCREEN)
    {
        _gameWindow.window = SDL_CreateWindow("Asteroids", 0, 0, 0, 0, SDL_WINDOW_OPENGL);
    }
    else
    {
        // TODO forcing this resolution for now
        _gameWindow.screenWidth = 1024;
        _gameWindow.screenHeight = 768;
        //_gameWindow.window = SDL_CreateWindow("Asteroids", 0, CONTROL_BOX_HEIGHT, _gameWindow.screenWidth, _gameWindow.screenHeight, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        _gameWindow.window = SDL_CreateWindow("OpenGL Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _gameWindow.screenWidth, _gameWindow.screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    }

    if (NULL == _gameWindow.window)
    {
        SDL_Quit();
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to create window: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool InitializeGLContext()
{
    SDL_Log("Creating GL context");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    _gameWindow.glContext = SDL_GL_CreateContext(_gameWindow.window);

    if (NULL == _gameWindow.glContext)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to create GL Context: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetSwapInterval(1);

    return true;
}

bool InitializeGlew()
{
    if (glewInit() != GLEW_OK)
    {
        SDL_Quit();
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Unable to initialize GLEW: %s", SDL_GetError());
        return false;
    }

    SDL_Log("OpenGL Vendor: %s", glGetString(GL_VENDOR));
    SDL_Log("OpenGL Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
    SDL_Log("OpenGL Shading Language: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

    return true;
}

bool _isMinimized = false;

void EngineProcessPollEvent(SDL_Event event, game_controller *keyboardController)
{
    if (event.key.repeat)
        return;

    //
    // engine controls
    //
    switch (event.type)
    {
    case SDL_KEYDOWN:
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Input: %d", inputState.windowEvent.key.keysym.sym);

        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE:
            _isStopRequested = true;
            SDL_Log("Stop requested - keyboard event - ESC");
            break;

        case SDLK_LCTRL:
        case SDLK_RCTRL:
            _isCtrlDown = true;
            break;

        case SDLK_r:
            if (_isCtrlDown)
            {
                if (_recordingState == Recording)
                {
                    _recordingState = None;
                    EndFileAccess(_recordingFile);
                }
                else if (_recordingState == None)
                {
                    _recordingFile = BeginRecordingInput();
                    _recordingState = Recording;
                }
            }

            break;

        case SDLK_p:
            if (_isCtrlDown)
            {
                if (_recordingState == Playback)
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

        switch (event.key.keysym.sym)
        {
        case SDLK_LCTRL:
        case SDLK_RCTRL:
            _isCtrlDown = false;
            break;
        }
        break;

    case SDL_WINDOWEVENT:
        switch (event.window.event)
        {
        case SDL_WINDOWEVENT_CLOSE:
            _isStopRequested = true;
            SDL_Log("Stop requested - window event");
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            _isMinimized = true;
            SDL_Log("Window minimized - not rendering 3D");
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESTORED:
            _isMinimized = false;
            SDL_Log("Window restored - resume rendering 3D");
            break;
        }
        break;

    case SDL_QUIT:
        _isStopRequested = true;
        SDL_Log("Stop requested - SDL event");
        break;

    case SDL_MOUSEMOTION:
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\tMouse: %d, %d", event.motion.x, event.motion.y);
        break;
    }

    //
    // game controls
    //
    switch (event.type)
    {
    case SDL_KEYUP:
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_UP:
        case SDLK_w:
            keyboardController->joystickUp.isDown = event.type == SDL_KEYDOWN ? true : false;
            keyboardController->joystickUp.state = event.type == SDL_KEYDOWN ? SPressed : SReleased;
            break;
        case SDLK_DOWN:
        case SDLK_s:
            keyboardController->joystickDown.isDown = event.type == SDL_KEYDOWN ? true : false;
            keyboardController->joystickDown.state = event.type == SDL_KEYDOWN ? SPressed : SReleased;
            break;
        case SDLK_LEFT:
        case SDLK_a:
            keyboardController->joystickLeft.isDown = event.type == SDL_KEYDOWN ? true : false;
            keyboardController->joystickLeft.state = event.type == SDL_KEYDOWN ? SPressed : SReleased;
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            keyboardController->joystickRight.isDown = event.type == SDL_KEYDOWN ? true : false;
            keyboardController->joystickRight.state = event.type == SDL_KEYDOWN ? SPressed : SReleased;
            break;
        case SDLK_SPACE:
            keyboardController->Button1.isDown = event.type == SDL_KEYDOWN ? true : false;
            keyboardController->Button1.state = event.type == SDL_KEYDOWN ? SPressed : SReleased;
        }
        break;

    case SDL_MOUSEMOTION:
        // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\tMouse: %d, %d", event.motion.x, event.motion.y);
        break;
    }
}

void EngineProcessInputs()
{
    uint64 startWork = SDL_GetPerformanceCounter();

    SDL_PollEvent(&inputState.windowEvent);
    EngineProcessPollEvent(inputState.windowEvent, &inputState.controllers[0]);

    if (_recordingState == Recording)
    {
        RecordInput(_recordingFile, &inputState);
        SDL_Log("RECORDING: %d", (uint32)inputState.windowEvent.key.keysym.sym);
    }
    else if (_recordingState == Playback)
    {
        ReadRecordedInput(_recordingFile, &inputState, &_recordingState);
        SDL_Log("Read input: %d", inputState.windowEvent.key.keysym.sym);
    }

    _workProcessInput = GetElapsedSeconds(startWork, SDL_GetPerformanceCounter()) * 1000.0f;
}

void EngineUpdateAndRenderFrame()
{
    uint64 startWork = SDL_GetPerformanceCounter();
    GameUpdate(_gameWindow, inputState, _deltaTimeSeconds);
    _workRender = GetElapsedSeconds(startWork, SDL_GetPerformanceCounter()) * 1000.0f;
}

static void CalculateTiming()
{
    // https://gafferongames.com/post/fix_your_timestep/

    _frameTimeElapsedSeconds = GetElapsedSeconds(_lastframeTimeStart, SDL_GetPerformanceCounter());
    _fps = 1.0f / _frameTimeElapsedSeconds;

    Uint32 sleep = (Uint32)floor(1000.0f * ((_targetSecondsPerFrame)-_frameTimeElapsedSeconds));
    SDL_Delay(sleep);

    _lockedFrameTimeElapsedSeconds = GetElapsedSeconds(_lastframeTimeStart, SDL_GetPerformanceCounter());
    _lockedfps = 1.0f / _lockedFrameTimeElapsedSeconds;

    _deltaTimeSeconds = _frameTimeElapsedSeconds;
    _lastframeTimeStart = SDL_GetPerformanceCounter();
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "\nFrame Complete");
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - Input Work:%fms", _workProcessInput);
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - GameState Work:%fms", _workGameState);
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - Render Work:%fms", _workRender);
    // SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, " - Slept for %fms, d(T): %fs, Frame Time:%fms, FPS:%f", work, sleepTime, _deltaTimeSeconds, frameTimeElapsed * 1000.0f, _fps);
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

static unsigned int CompileShader(unsigned int type, const std::string &source)
{
    unsigned int id = glCreateShader(type);
    const char *src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    const char *shaderType = (type == GL_VERTEX_SHADER ? "vertex" : "fragment");

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char *message = (char *)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);

        SDL_Log("Shader failed to compile:");
        SDL_Log(shaderType);
        SDL_Log(message);
        glDeleteShader(id);
        return 0;
    }

    SDL_Log(shaderType);
    SDL_Log("Shader compiled");

    return id;
}

static unsigned int CreateShader(const std::string &vertexShader, const std::string &fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    // glDetachShader(vs);
    // glDetachShader(fs);
    // glDeleteShader(vs);
    // glDeleteShader(fs);

    return program;
}

void TheChernoSetup()
{
    float positions[6] =
        {
            -0.5f, -0.5f,
            0.0f, 0.5f,
            0.5f, -0.5f};

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    std::string vertexShader =
        "#version 330 core\n"
        ""
        "layout(location = 0) in vec4 position;"
        ""
        "void main()"
        "{"
        "   gl_Position = position;"
        "}";

    std::string fragmentShader =
        "#version 330 core\n"
        ""
        "layout(location = 0) out vec4 color;"
        ""
        "void main()"
        "{"
        "   color = vec4(1.0, 0.0, 0.0, 1.0);"
        "}";

    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader);
}

void TheChernoDrawTriangle()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    SDL_GL_SwapWindow(_gameWindow.window);
}

unsigned int _vertexArrayObject = 0;
unsigned int _vertexBufferObject = 0;

void VertexSpecification()
{
    float positions[9] =
        {
            -0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f,
            0.5f, -0.5f, 0.0f};

    glGenVertexArrays(1, &_vertexArrayObject);
    glBindVertexArray(_vertexArrayObject);

    glGenBuffers(1, &_vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 9, positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
}

unsigned int _shaderProgram = 0;
void CreateGraphicsPipeline()
{
    std::string vertexShader =
        "#version 410 core\n"
        ""
        "in vec4 position;"
        ""
        "void main()"
        "{"
        "   gl_Position = vec4(position.x, position.y, position.z, position.w);"
        "}";

    std::string fragmentShader =
        "#version 410 core\n"
        ""
        "out vec4 color;"
        ""
        "void main()"
        "{"
        "   color = vec4(1.0f, 0.5f, 0.0f, 1.0f);"
        "}";

    _shaderProgram = CreateShader(vertexShader, fragmentShader);
}

void PreDraw(float r, float g, float b)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, _gameWindow.screenWidth, _gameWindow.screenHeight);
    glClearColor(r, g, b, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(_shaderProgram);
}

void Draw()
{
    glBindVertexArray(_vertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferObject);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // SDL_GL_SwapWindow(_gameWindow.window);
}

bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void ProcessImGui()
{
    // ImGui Syyle
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 4.0;
    auto &colors = style.Colors;

    ImVec4(&titsup)[53] = style.Colors;

    colors[ImGuiCol_WindowBg] = ImColor(5, 0, 0, 1);
    colors[ImGuiCol_Button] = ImColor(18, 18, 18, 100);
    colors[ImGuiCol_ButtonActive] = ImColor(21, 21, 21, 100);

    titsup[ImGuiCol_WindowBg] = ImColor(5, 0, 0, 1);
    titsup[ImGuiCol_Button] = ImColor(18, 18, 18, 100);
    titsup[ImGuiCol_ButtonActive] = ImColor(21, 21, 21, 100);

    ImGui_ImplSDL2_ProcessEvent(&inputState.windowEvent);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

    ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float *)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);
    // ImGui::Text("Fame Time = %d", (int)(_frameTimeElapsed * 1000.0f));

    ImGui::Text("GUI average %.3f ms/frame (%.1f FPS)", 1000.0f / _ioMetrics->Framerate, _ioMetrics->Framerate);
    ImGui::Text("Engine %.1f FPS (%.1f locked)", _fps, _lockedfps);
    ImGui::Text("Engine %.3f ms/frame (%.3f locked)", _frameTimeElapsedSeconds * 1000.0f, _lockedFrameTimeElapsedSeconds * 1000.0f);

    ImGui::End();

    if (show_another_window)
    {
        ImGui::SetNextWindowSize({200, 50});
        ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

int main(int argc, char *argv[])
{
    exeFileInfo = {};
    GetExeFileName(&exeFileInfo);

    SDL_Log("FileInfo.fileName:%s", exeFileInfo.absoluteFileName);
    SDL_Log("FileInfo.path:%s", exeFileInfo.path);

    if (!InitializeSDL())
    {
        SDL_Log("Exiting application");
        return 0;
    }

    if (!InitializeWindow())
    {
        SDL_Log("Exiting application");
        return 0;
    }

    if (!InitializeGLContext())
    {
        SDL_Log("Exiting application");
        return 0;
    }

    if (!InitializeGlew())
    {
        SDL_Log("Exiting application");
        return 0;
    }

    VertexSpecification();
    CreateGraphicsPipeline();

    const char *glsl_version = "#version 130";
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(_gameWindow.window, _gameWindow.glContext);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    _ioMetrics = &ImGui::GetIO();
    //(void)_ioMetrics;

    _lastframeTimeStart = SDL_GetPerformanceCounter();

    while (_isStopRequested == false)
    {
        EngineProcessInputs();

        if (_isStopRequested)
        {
            continue;
        }

        EngineUpdateAndRenderFrame();

        if (!_isMinimized)
        {
            PreDraw(clear_color.x, clear_color.y, clear_color.z);
        }

        ProcessImGui();

        if (!_isMinimized)
        {
            Draw();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        SDL_GL_SwapWindow(_gameWindow.window);

        CalculateTiming();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    Shutdown();

    SDL_Log("Exiting application");
    return 0;
}