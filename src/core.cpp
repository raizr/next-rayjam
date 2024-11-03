#include "core.h"

#include <string>
#include <cmath>
#include "raymath.h"
#include "raygui.h"
#include "resource.h"
#include "scene_manager.h"

using namespace core;

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
    InitWindow(screenWidth, screenHeight, "Next Bridge");
    InitAudioDevice();
    SearchAndSetResourceDir("resources");
    std::string dir = GetWorkingDirectory();
    GuiLoadStyle((dir + "/style.rgs").c_str());
    Resources::LoadFonts();
    Resources::LoadTextures();
    Resources::LoadMusic();
    //HideCursor();
    SetExitKey(KEY_NULL);
    SetTargetFPS(FIXED_FRAME_RATE);
    target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    //SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);
    scene::SceneManager::getInstance()->Load();
    PlayMusicStream(Resources::music);
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
    float spacing = 2;
    Vector2 size = MeasureTextEx(Resources::baseFont, text, textSize, spacing);
    Vector2 pos = { GetScreenWidth() * xOffset - size.x / 2.0f, GetScreenHeight() * yOffset - size.y / 2.0f };
    DrawTextEx(Resources::baseFont, text, pos, (float)textSize, spacing, R_YELLOW);
    return pos;
}

void Core::Update()
{
    UpdateMusicStream(Resources::music);
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
            PlaySound(Resources::effect3);
            tutorial = true;
            gameState = GameState::Playing;
            scene::SceneManager::getInstance()->Load();
        }
    }
    if (gameState == GameState::Paused)
    {
        if (AcceptPressed())
        {
            PlaySound(Resources::effect3);
            gameState = GameState::Playing;
        }
    }
    if (gameState == GameState::Lose)
    {
        if (AcceptPressed())
        {
            PlaySound(Resources::effect3);
            scene::SceneManager::getInstance()->Reset();
            scene::SceneManager::getInstance()->Load();
            gameState = GameState::Playing;
        }
    }

    if (gameState == GameState::Playing)
    {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_TAB))
        {
            gameState = GameState::Paused;
        }
        if (scene::SceneManager::getInstance()->IsLevelClear())
        {
            gameState = GameState::ChangingLevel;
        }
    }

    if (gameState == GameState::Playing)
    {
        scene::SceneManager::getInstance()->Update();
    }
    if (gameState == GameState::ChangingLevel)
    {
        if (AcceptPressed()) {
            PlaySound(Resources::effect3);
            if (scene::SceneManager::getInstance()->IsLastLevel()) {
                gameState = GameState::Paused;
                scene::SceneManager::getInstance()->setLevel(-1);
                scene::SceneManager::getInstance()->NextLevel();
            }
            else {
                scene::SceneManager::getInstance()->NextLevel();
                gameState = GameState::Playing;
            }
        }
    }
    float scale = MIN((float)GetScreenWidth() / gameScreenWidth, (float)GetScreenHeight() / gameScreenHeight);
    BeginTextureMode(target);
        ClearBackground(BLACK);
        scene::SceneManager::getInstance()->Draw();
    EndTextureMode();
    BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(target.texture,
            Rectangle { 0.0f, 0.0f, (float)target.texture.width, (float)-target.texture.height },
            Rectangle{ (GetScreenWidth() - ((float)gameScreenWidth * scale)) * 0.5f,
                    (GetScreenHeight() - ((float)gameScreenHeight * scale)) * 0.5f,
                    (float)gameScreenWidth* scale, (float)gameScreenHeight* scale
        }, Vector2 { 0, 0 }, 0.0f, WHITE);

        DrawGUI();
        if (gameState == GameState::ChangingLevel)
        {
            if (scene::SceneManager::getInstance()->IsLastLevel()) {
                DrawCredits();
            }
            else {
                DrawLevelPassed();
            }
        }
        if (gameState == GameState::Paused)
        {
            DrawMenu();
        }
        if (gameState == GameState::Lose)
        {
            DrawLoseMenu();
        }
    EndDrawing();
}

void Core::DrawMenu()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), R_DDBLUE);
    DrawCenteredText("Next Bridge", 60);
    DrawCenteredText("press to play",30, 0.6f, 0.5f);
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
    if (GuiButton({ Rectangle { 10.0f, 10.0f, 150.0f, 50.0f } }, GuiIconText(132, "PAUSE"))) {
        gameState = GameState::Paused;
        PlaySound(Resources::effect2);
    }
    if (GuiButton({ Rectangle { GetScreenWidth() - 310.0f, 10.0f, 150.0f, 50.0f } }, GuiIconText(77, "RESTART"))) {
        PlaySound(Resources::effect2);
        scene::SceneManager::getInstance()->Reset();
        scene::SceneManager::getInstance()->Load();
    }
    if (GuiButton({ Rectangle { GetScreenWidth() - 160.0f, 10.0f, 150.0f, 50.0f }}, GuiIconText(131, "PLAY"))) {
        PlaySound(Resources::effect);
        scene::SceneManager::getInstance()->MoveCar();
        scene::SceneManager::getInstance()->SetTutotrialPassed();
    }
}

void core::Core::DrawLevelPassed()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), R_D_BLUE);
    DrawCenteredText("Level complete!", 30, 0.6f, 0.5f);
    DrawCenteredText("press to go to the NEXT Bridge", 30, 0.7f, 0.5f);
}

void Core::DrawLoseMenu()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), R_D_BLUE);
    DrawCenteredText("You Lose", 30, 0.6f, 0.5f);
    DrawCenteredText("press to restart", 30, 0.7f, 0.5f);
}

void core::Core::DrawCredits()
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), R_D_BLUE);
    DrawCenteredText("Thanks for playing", 30, 0.6f, 0.5f);
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
