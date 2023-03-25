#if !defined(RECORDING_H)

#include "game_types.h"

#include <windows.h>

struct file_info
{
    char absoluteFileName[MAX_PATH];
    char path[MAX_PATH];
};

enum InputRecordState
{
    None,
    Recording,
    Playback
};

static void CatStrings(int sourceACount, char *sourceA, int sourceBCount, char* sourceB, int destCount, char *dest);

void GetExeFileName(file_info *fileInfo);

static int StringLength(char *string);

void EndFileAccess(SDL_RWops* file);

SDL_RWops* BeginRecordingInput();

SDL_RWops* BeginReadingInput();

void RecordInput(SDL_RWops* file, game_input *input);

void ReadRecordedInput(SDL_RWops* file, game_input *input, InputRecordState *state);

#define RECORDING_H
#endif