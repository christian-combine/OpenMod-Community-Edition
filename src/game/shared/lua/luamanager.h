//========= Copyright OpenMod, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef LUAMANAGER_H
#define LUAMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include <lua.hpp> // basically does the struct "c" and stuff
#include <string>
#include <filesystem.h>

class LuaManager
{
public:
    LuaManager();
    ~LuaManager();

    static LuaManager* GetInstance()
    {
        static LuaManager instance;
        return &instance;
    }
    lua_State* GetState() const;

    static void SetPrintMessage(bool printMessage);
    static bool printMessage;

    static void LoadDirectories(lua_State* L);

private:
    lua_State* L;

    static int LuaPrint(lua_State* L);
    static int LuaWarn(lua_State* L);
};

#endif