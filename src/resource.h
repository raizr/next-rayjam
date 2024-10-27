#pragma once
#include "raylib.h"
#include <string>
#include <array>


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
        //using std::string_literals::operator""s;
        std::string dir = GetWorkingDirectory();
       
    }
    inline static Texture test;
};
