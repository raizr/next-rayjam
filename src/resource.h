#pragma once
#include "raylib.h"
#include <string>
#include <array>

#define R_DDBLUE  CLITERAL(Color){ 8, 20, 30, 255 }
#define R_D_BLUE  CLITERAL(Color){ 15, 42, 63, 255 }
#define R_BLUE    CLITERAL(Color){ 32, 57, 79, 255 }
#define R_YELLOW  CLITERAL(Color){ 246, 214, 189, 255 }
#define R_GOLD    CLITERAL(Color){ 195, 163, 138, 255 }
#define R_ORANGE  CLITERAL(Color){ 153, 117, 119, 255 }
#define R_PINK    CLITERAL(Color){ 129, 98, 113, 255 }
#define R_RED     CLITERAL(Color){ 78, 73, 95, 255 }

inline bool SearchAndSetResourceDir(const char* folderName)
{
    // check the working dir
    if (DirectoryExists(folderName))
    {
        ChangeDirectory(TextFormat("%s/%s", GetWorkingDirectory(), folderName));
        return true;
    }

    const char* appDir = GetApplicationDirectory();

    // check the applicationDir
    const char* dir = TextFormat("%s%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    // check one up from the app dir
    dir = TextFormat("%s../%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    // check two up from the app dir
    dir = TextFormat("%s../../%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    // check three up from the app dir
    dir = TextFormat("%s../../../%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    dir = TextFormat("%s../../../src/%s", appDir, folderName);
    if (DirectoryExists(dir))
    {
        ChangeDirectory(dir);
        return true;
    }

    return false;
}

inline size_t AddEffect(const char* name, float volume = 1.0f);

struct Resources
{
    static void LoadTextures()
    {
        using std::string_literals::operator""s;
        std::string dir = GetWorkingDirectory();
        car = LoadTexture((dir + "/car.png").c_str());
        wheel = LoadTexture((dir + "/wheel.png").c_str());
    }

    static void LoadFonts()
    {
        std::string dir = GetWorkingDirectory();
        baseFont = LoadFontEx((dir + "/SCHABO-Condensed.otf").c_str(), 64, 0, 250);
    }

    inline static Texture car;
    inline static Texture wheel;
    inline static Font baseFont;
};
