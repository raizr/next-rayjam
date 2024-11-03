#pragma once
#include <vector>
#include "raylib.h"

namespace core
{

    #define MAX(a, b) ((a)>(b)? (a) : (b))
    #define MIN(a, b) ((a)<(b)? (a) : (b))
    enum class GameScreen
    {
        SCREEN_LOGO = 0,
        SCREEN_TITLE,
        SCREEN_GAMEPLAY
    };

    enum class GameState
    {
        None,
        Tutorial,
        Playing,
        Paused,
        ChangingLevel,
        Lose
    };

    static constexpr int screenWidth = 1280;
    static constexpr int screenHeight = 720;
    static constexpr int scaleGameScreen = 4;
    static constexpr int gameScreenWidth = 240 * scaleGameScreen;
    static constexpr int gameScreenHeight = 135 * scaleGameScreen;
    constexpr int FIXED_FRAME_RATE = 60;

    class Core
    {
    public:
        static Core* getInstance();
        static void cleanup();
        void Init();
        double GetCurrentTime() const;
        float GetDeltaTime() const;

        void Update();
        void DrawMenu();
        void DrawGUI();
        void DrawLevelPassed();
        void DrawLoseMenu();
        void OnLose();
        bool AcceptPressed();

        bool IsPaused() { return gameState == GameState::Paused; }

        static void CenterWindow();
        bool isTouch();
        bool isDragGesture();
        Vector2 getDragVector();
        bool GetTouchTap();
    private:
        Core() = default;
        ~Core();

        inline static Core* instance = nullptr;
        GameState gameState = GameState::Paused;
        RenderTexture2D target;
        double time = 0;
        int uiShieldCurrentFrame = 0;
        int uiLifeCurrentFrame = 0;

        int currentGesture = GESTURE_NONE;
        int lastGesture = GESTURE_NONE;
        Shader postShader;

        Vector2 touchPosition = { 0, 0 };
        Vector2 touchRightPosition = { 0, 0 };
        Vector2 holdLast = { 0, 0 };
        Vector2 rightTap = { 0, 0 };
        Vector2 dragCurrent = { 0, 0 };
        Rectangle touchArea = { 0.0f, 0.0f, screenWidth / 2.0f, screenHeight};
        Rectangle touchRight = { screenWidth / 2.0f, 0.0f, screenWidth / 2.0f, screenHeight};
        Rectangle pauseBound;
        bool isDrag = false;
        bool touchTap = false;
        bool tutorial = false;
    };
    
}