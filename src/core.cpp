#include "core.h"

#include <string>
#include <cmath>
#include "raygui.h"
#include "raymath.h"

using namespace core;

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

Core* Core::getInstance()
{
    if (!instance)
    {
        instance = new Core();
    }
    return instance;
}

void Core::cleanup()
{
    
    CloseAudioDevice();
    delete instance;
}

void Core::Init()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Next Jam");

    std::string dir = GetWorkingDirectory();

    HideCursor();
    SetExitKey(KEY_NULL);
    SetTargetFPS(FIXED_FRAME_RATE);
    target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
}


void Core::CenterWindow()
{
    int monitor = GetCurrentMonitor();

    int x = GetMonitorWidth(monitor) / 2 - screenWidth / 2;
    int y = GetMonitorHeight(monitor) / 2 - screenHeight / 2;

    SetWindowPosition(x, y);
}

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
EM_JS(bool, is_touch, (), {
    return is_touch_screen = 'ontouchstart' in document.documentElement;
});
#endif
bool Core::isTouch()
{
#if defined(PLATFORM_WEB)
    return is_touch();
#endif
    return false;
}

bool Core::isDragGesture()
{
    return isDrag;
}

Vector2 Core::getDragVector()
{
    return Vector2Normalize(Vector2Subtract(holdLast, touchPosition));
}

bool Core::GetTouchTap()
{
    return touchTap;
}

Core::~Core()
{
    UnloadRenderTexture(target);
}

double Core::GetCurrentTime() const
{
    return time;
}

// to support pause, and make debugging easier, we track our own delta time
float Core::GetDeltaTime() const
{
    if (gameState == GameState::Paused)
    {
        return 0;
    }

#ifdef _DEBUG
    return 1.0f / FIXED_FRAME_RATE;
#else
    return GetFrameTime();
#endif
}

Vector2 DrawCenteredText(const char* text, float textSize = 20, float yOffset = 0.5f, float xOffset = 0.5f)
{
    Vector2 size = MeasureTextEx(GetFontDefault(), text, textSize, textSize / 10);

    Vector2 pos = { GetScreenWidth() * xOffset - size.x / 2.0f, GetScreenHeight() * yOffset - size.y / 2.0f };
    DrawText(text, int(pos.x), int(pos.y), int(textSize), WHITE);
    return pos;
}

void Core::Update()
{
    if (isTouch())
    {
        lastGesture = currentGesture;
        currentGesture = GetGestureDetected();
        touchPosition = GetTouchPosition(0);
        if (CheckCollisionPointRec(touchPosition, touchArea))
        {
            isDrag = (currentGesture != GESTURE_NONE);
        }
        if (isDrag)
        {
            if (currentGesture != lastGesture)
            {
                switch (currentGesture)
                {
                    case GESTURE_TAP:
                        break;
                    case GESTURE_HOLD: 
                        holdLast = touchPosition;
                        break;
                    case GESTURE_DRAG:
                        dragCurrent = touchPosition;
                        break;
                    default:
                        break;
                }
            }
        }
        touchRightPosition = GetTouchPosition(1);
        touchTap = (CheckCollisionPointRec(touchRightPosition, touchRight)
            || CheckCollisionPointRec(touchPosition, touchRight))
                && ((currentGesture == GESTURE_TAP) || (currentGesture == GESTURE_HOLD));
    }


    if (gameState != GameState::Paused)
    {
        time += GetDeltaTime();
    }
    if (gameState == GameState::Tutorial)
    {
        if (AcceptPressed())
        {
            tutorial = true;
            gameState = GameState::Playing;
        }
    }
    if (gameState == GameState::Paused)
    {
        if (AcceptPressed())
        {
            if (!tutorial)
            {
                gameState = GameState::Tutorial;
            }
            else
            {
                gameState = GameState::Playing;
            }
        }
    }
    if (gameState == GameState::Lose)
    {
        if (AcceptPressed())
        {
            
            gameState = GameState::Playing;
        }
    }

    if (gameState == GameState::Playing)
    {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB))
        {
            gameState = GameState::Paused;
        }
    }
    if (gameState == GameState::Playing)
    {
        //scene::SceneManager::getInstance()->Update();
    }
    if (gameState == GameState::ChangingLevel)
    {
        //scene::SceneManager::getInstance()->Update();
    }
    float scale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
    BeginTextureMode(target);
        ClearBackground(BLACK);
        //scene::SceneManager::getInstance()->Draw();
    EndTextureMode();
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture,
            Rectangle { 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
            Rectangle{ (GetScreenWidth() - ((float)gameScreenWidth * scale)) * 0.5f,
                    (GetScreenHeight() - ((float)gameScreenHeight * scale)) * 0.5f,
                    (float)gameScreenWidth* scale, (float)gameScreenHeight* scale
        }, Vector2 { 0, 0 }, 0.0f, WHITE);
        pauseBound = { 10.0f, 10.f,
                       100.0f, 200.0f
        };
        if (CheckCollisionPointRec(GetMousePosition(), pauseBound)
            || CheckCollisionPointRec(touchPosition, pauseBound))
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                gameState = GameState::Paused;
            }
        }
        DrawGUI();
    if (gameState == GameState::ChangingLevel)
    {
        
    }
    if (gameState == GameState::Paused)
    {
        DrawMenu();
    }
    if (gameState == GameState::Lose)
    {
        DrawLoseMenu();
    }
    if (IsCursorOnScreen())
    {
        GuiDrawIcon(50, GetMouseX(), GetMouseY(), 1, WHITE);
    }
    EndDrawing();
}

void Core::DrawMenu()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0, 0, 0, 255 });
    DrawCenteredText("Next jam", 60);
    DrawCenteredText("press to start",30, 0.6f, 0.5f);
}

void Core::DrawGUI()
{
    float scale = 3.0f;

    
    if (isTouch())
    {
        if (isDrag && currentGesture == GESTURE_DRAG)
        {
            
        }
    }
}

void Core::DrawLoseMenu()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{ 0, 0, 0, 128 });
    DrawCenteredText("You Lose", 30, 0.6f, 0.5f);
    DrawCenteredText("press to restart", 30, 0.7f, 0.5f);
}

void Core::OnLose()
{
    gameState = GameState::Lose;
}

bool Core::AcceptPressed()
{
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        return true;

    if (IsGamepadAvailable(0))
    {
        return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    }

    return false;
}
