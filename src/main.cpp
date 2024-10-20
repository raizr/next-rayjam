#include "raylib.h"
#include "core.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

static void UpdateDrawFrame(void);      // Update and Draw one frame


int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);
#endif
    
    core::Core::getInstance()->Init();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, core::FIXED_FRAME_RATE, 1);
#else

    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif
    core::Core::cleanup();
    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void)
{
    core::Core::getInstance()->Update();
}