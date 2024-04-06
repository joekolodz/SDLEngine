@echo off

echo ===========================
echo Building SDLEngine.cpp
echo ===========================

REM If you want to try to get the tasks.json (CTRL+SHIFT+B) working use the tasks.json file in the root .vscode folder not the src/.vscode folder. should just delete the second one
REM
REM Need to run the vcvars batch file for each instance of VSCode or launch VSCode from the developer command prompt (which already ran vsvars)
REM This is where the compiler gets setup: (include quotes when copying)
rem call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
REM
REM cl.exe found here: "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.35.32215\bin\Hostx64\x64"

rem set commonCompilerFlags=-Fm -Gm- -MT -Od -Oi -W4 -WX -wd4201 -wd4100 -wd4189 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -nologo -FC -Z7
rem set commonLinkerFlags=-opt:ref user32.lib gdi32.lib winmm.lib

set commonCompilerFlags=^
    -Fmbin\ ^
    -Fobin\ ^
    -Gm- ^
    -MTd ^
    -Od ^
    -Oi ^
    -EHa ^
    -W4 ^
    -WX ^
    -wd4201 ^
    -wd4100 ^
    -wd4189 ^
    -wd4505 ^
    -nologo ^
    -fp:fast ^
    -FC ^
    -Z7 ^
    -I include

set commonLinkerFlags=^
    -incremental:no ^
    -SUBSYSTEM:CONSOLE ^
    -LIBPATH:"D:\Development\SDLEngine\lib" ^
    user32.lib ^
    shell32.lib ^
    SDL2.lib ^
    SDL2main.lib ^
    SDL2_ttf.lib ^
    SDL2_image.lib ^
    opengl32.lib ^
    glew32.lib


IF NOT EXIST bin mkdir bin
pushd bin

REM > redirects to the void
REM 2 > &1 redirects stderr to stdout (1 is stdout, 2 is stderr)
REM the debugger locks the PDB files, so when we live code edit, we stack up old PDB files
del *.pdb > NUL 2>&1
popd

REM ECHO 32-bit build
REM cl.exe %commonCompilerFlags% ..\src\win32_handmade.cpp /link -subsystem:windows,5.1 %commonLinkerFlags%


ECHO 64-bit build
REM optimization switches: /O2 /Oi /fp:fast
REM cl.exe %commonCompilerFlags% ..\src\SDLTest.cpp -LD /link -incremental:no -PDB:SDLTest_%random%.pdb
cl.exe %commonCompilerFlags% imgui\imgui_impl_sdl2.cpp imgui\imgui_impl_opengl3.cpp imgui\imgui_draw.cpp imgui\imgui_tables.cpp imgui\imgui_widgets.cpp imgui\imgui.cpp src\image.cpp src\game_types.cpp src\recording.cpp src\game.cpp src\SDLEngine.cpp /link %commonLinkerFlags% /out:bin\SDLEngine.exe
REM cl.exe %commonCompilerFlags% src\SDLTest.cpp /link /LIBPATH:D:\Development\SDLTest\lib /SUBSYSTEM:WINDOWS


REM "D:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.35.32215/bin/Hostx64/x64/cl.exe" %commonCompilerFlags% imgui\imgui_impl_sdl2.cpp imgui\imgui_impl_opengl3.cpp imgui\imgui_draw.cpp imgui\imgui_tables.cpp imgui\imgui_widgets.cpp imgui\imgui.cpp src\image.cpp src\game_types.cpp src\recording.cpp src\game.cpp src\SDLEngine.cpp /link %commonLinkerFlags% /out:bin\SDLEngine.exe



copy lib\SDL2.dll bin\
copy lib\SDL2_ttf.dll bin\
copy lib\SDL2_image.dll bin\
copy lib\glew32.dll bin\

REM del *.map
REM del *.obj


rem for 32 bit builds link with this instead:
rem     /link -opt:ref -subsystem:windows,5.1 ^

REM When used at the command line, the linker defaults to /OPT:REF,ICF,LBR. 
REM If /DEBUG is specified, the default is /OPT:NOREF,NOICF,NOLBR.

rem winmm.lib - for timeBeginPeriod windows function

rem compiler switches
rem W4 == limited warning (WALL all warning, W1, W2, W3 are limited at different levels) 
rem WX == treat warning as errors
rem wd#### == disable specific complier warning
rem Oi == optimization on for intrinsics (use the assembly directly to perform operation instead of calling a C library)
rem Od == disable all optimizations. good for debugging
rem Zi == create PDB files
rem Z7 == older format for PDB files that works better for larger debugging environments
rem GR- == turn off C++'s  runtime type information 
rem EHa- == do not generate exception handling overhead (wont compile try/catch)
rem nologo == turn off the 'Microsoft label'
rem MD == usually the VS IDE runs with this option witch expects a c runtime library to already exist so doesn't link the dll directly
rem       use depends.exe on your executable to tell you what your exe actually depends on
rem MT == use the static library and link it with the exe (this is actually the default when compiling from our command line)
rem MTd == use the debug version of the static library
rem Gm- == minimal build - turns off any incremental build tracking
rem Fm == tells the linker a location to output a map file


rem linker switches
rem /link -subsystem:console == build for console?
rem /link -subsystem:windows,5.1 == actually let's the exe run on windows!
rem /link -opt:ref == try to discard things that aren't actually used when linking