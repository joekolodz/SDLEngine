#include "recording.h"
#include "game_types.h"

#include <windows.h>

static void CatStrings(int sourceACount, char *sourceA, int sourceBCount, char* sourceB, int destCount, char *dest)
{
    for(int index = 0; index < sourceACount; index++)
    {
        *dest++ = *sourceA++;
    }

    for(int index = 0; index < sourceBCount; index++)
    {
        *dest++ = *sourceB++;
    }

    *dest++ = 0;
}

void GetExeFileName(file_info *fileInfo)
{
    //get path and name of this executable
    DWORD sizeOfFileName = GetModuleFileNameA(0, (LPSTR)fileInfo->absoluteFileName, sizeof(fileInfo->absoluteFileName));
    //if there is no path, then just point to the file name;
    char *tempSource = fileInfo->absoluteFileName;
    char *pathEnd = fileInfo->absoluteFileName;


    //find last slash so we can just work with the path
    for(char * scan = fileInfo->absoluteFileName; *scan; scan++)
    {
        if(*scan == '\\')
        {
            pathEnd = scan + 1;
        }
    }

    int64 pathLength = pathEnd - fileInfo->absoluteFileName;

    char *dest = fileInfo->path;


    for(int index = 0; index <= pathLength - 1; index++)
    {
        *dest++ = *tempSource++;
    }
}

static int StringLength(char *string)
{
    int count = 0;
    while(*string++)
    {
        count++;
    }
    return count;
}

// static void BuildExePathFileName(win32_state *state, char *fileName, int destCount, char *dest)
// {
//     CatStrings( state->onePastLastExeFileNameSlash - state->exeFullPathFileName, 
//                 state->exeFullPathFileName, 
//                 StringLength(fileName), fileName, 
//                 destCount, dest
//                 );    
// }

void EndFileAccess(SDL_RWops* file)
{
    SDL_RWclose(file);
}

SDL_RWops* BeginRecordingInput()
{
    return SDL_RWFromFile("input.bin", "wb");
}

SDL_RWops* BeginReadingInput()
{
    return SDL_RWFromFile("input.bin", "rb");
}

void RecordInput(SDL_RWops* file, game_input *input)
{
    size_t written = SDL_RWwrite(file, input, sizeof(*input), 1);
}

void ReadRecordedInput(SDL_RWops* file, game_input *input, InputRecordState *state)
{
    if(*state == Playback)
    {
        uint8 countRead = (uint8)SDL_RWread(file, input, sizeof(*input), 1 );
        if(countRead == 0)
        {
            *state = None;
            EndFileAccess(file);
            SDL_Log("Playback ended, file closed");
        }
    }
}