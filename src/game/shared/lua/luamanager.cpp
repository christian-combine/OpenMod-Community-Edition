//========= Copyright OpenMod, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "luamanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define LUA_COLOR Color(0, 128, 255, 255)

bool LuaManager::printMessage = true;
void LuaManager::SetPrintMessage(bool printMessage)
{
	LuaManager::printMessage = printMessage;
}

static LuaManager* globalLuaManager = nullptr;

const char* directories[] = {
	"lua/autorun",
#ifdef CLIENT_DLL
	"lua/gameui",
#endif
};

std::string CharArrayToString(const char* charArray)
{
	return std::string(charArray ? charArray : "");
}

bool LoadLuaFile(lua_State* L, const char* relativePath, const char* pathID, const char* fileType)
{
	char fullPath[MAX_PATH];
	filesystem->RelativePathToFullPath(relativePath, pathID, fullPath, sizeof(fullPath));

	ConMsg("[LUA] Loading %s file: %s\n", fileType, fullPath);

	int result = luaL_loadfile(L, fullPath);
	if (result == LUA_OK)
	{
		result = lua_pcall(L, 0, 0, 0);
		if (result != LUA_OK)
		{
			Warning("[LUA] Error executing %s file %s: %s\n", fileType, fullPath, lua_tostring(L, -1));
			lua_pop(L, 1);
			return false;
		}
	}
	else
	{
		Warning("[LUA] Error loading %s file %s: %s\n", fileType, fullPath, lua_tostring(L, -1));
		lua_pop(L, 1);
		return false;
	}

	return true;
}

int LRequire(lua_State* L)
{
#ifdef CLIENT_DLL
	const char* pathID = "GAME";
#else
	const char* pathID = "MOD";
#endif
	const char* filename = luaL_checkstring(L, 1);
	char includesPath[MAX_PATH];

	sprintf_s(includesPath, sizeof(includesPath), filename);
	LoadLuaFile(L, includesPath, pathID, "utilities");

	return 1;
}

void LuaManager::LoadDirectories(lua_State* L)
{
#ifdef CLIENT_DLL
	const char* pathID = "GAME";
#else
	const char* pathID = "MOD";
#endif

	// stuff
	for (int i = 0; i < sizeof(directories) / sizeof(directories[0]); ++i)
	{
		const char* directory = directories[i];
		char path[MAX_PATH];
		sprintf_s(path, sizeof(path), "%s/*.lua", directory);

		FileFindHandle_t findHandle;
		const char* filename = filesystem->FindFirst(path, &findHandle);
		while (filename)
		{
			char relativePath[MAX_PATH];
			sprintf_s(relativePath, sizeof(relativePath), "%s/%s", directory, filename);

			if (!LoadLuaFile(L, relativePath, pathID, "file"))
			{
				Warning("[LUA] Failed to load file: %s\n", relativePath);
			}

			filename = filesystem->FindNext(findHandle);
		}

		filesystem->FindClose(findHandle);
	}

	char addonsPath[MAX_PATH];
	sprintf_s(addonsPath, sizeof(addonsPath), "addons/*");

	FileFindHandle_t addonFindHandle;
	const char* addonDir = filesystem->FindFirst(addonsPath, &addonFindHandle);
	while (addonDir)
	{
		if (filesystem->IsDirectory(addonDir))
		{
			char addonDirPath[MAX_PATH];
			sprintf_s(addonDirPath, sizeof(addonDirPath), "addons/%s", addonDir);

			char luaPath[MAX_PATH];
			sprintf_s(luaPath, sizeof(luaPath), "%s/*.lua", addonDirPath);

			FileFindHandle_t luaFindHandle;
			const char* luaFile = filesystem->FindFirst(luaPath, &luaFindHandle);
			while (luaFile)
			{
				char relativeAddonPath[MAX_PATH];
				sprintf_s(relativeAddonPath, sizeof(relativeAddonPath), "%s/%s", addonDirPath, luaFile);

				if (!LoadLuaFile(L, relativeAddonPath, pathID, "addon"))
				{
					Warning("[LUA] Failed to load addon file: %s\n", relativeAddonPath);
				}

				luaFile = filesystem->FindNext(luaFindHandle);
			}

			filesystem->FindClose(luaFindHandle);
		}

		addonDir = filesystem->FindNext(addonFindHandle);
	}

	filesystem->FindClose(addonFindHandle);
}

LuaManager::LuaManager()
{
	L = luaL_newstate();

	if (L == nullptr)
	{
		Warning("Unable to create Lua state!\n");
		return;
	}

	luaL_openlibs(L);
	luaopen_base(L);

	lua_register(L, "require", LRequire);
	lua_register(L, "print", LuaPrint);
	lua_register(L, "warn", LuaWarn);

    // oh well..
    LSteamAPI::Register(L);

#ifdef CLIENT_DLL
	lua_pushboolean(L, true);
	lua_setglobal(L, "CLIENT");
	lua_pushboolean(L, false);
	lua_setglobal(L, "SERVER");
#else
	lua_pushboolean(L, false);
	lua_setglobal(L, "CLIENT");
	lua_pushboolean(L, true);
	lua_setglobal(L, "SERVER");
#endif

	lua_newthread(L);
	lua_pushvalue(L, -2);

	globalLuaManager = this;

	if (printMessage)
	{
#ifdef CLIENT_DLL
		ConColorMsg(LUA_COLOR, "[LUA] client initialized (" LUA_VERSION ")\n");
#else
		ConColorMsg(LUA_COLOR, "[LUA] server initialized (" LUA_VERSION ")\n");
#endif
	}
}

lua_State* LuaManager::GetState() const
{
	return L;  // a shitty workaround, because you can't access the L in the .h file
}

int LuaManager::LuaPrint(lua_State* L)
{
	int numArgs = lua_gettop(L);

	for (int i = 1; i <= numArgs; ++i)
	{
		if (lua_isstring(L, i))
		{
			const char* str = lua_tostring(L, i);
			Msg("%s\n", str);
		}
		else
		{
			lua_getglobal(L, "tostring");
			lua_pushvalue(L, i);

			if (lua_pcall(L, 1, 1, 0) != LUA_OK)
			{
				lua_pushfstring(L, "error calling tostring: %s", lua_tostring(L, -1));
				lua_error(L);
			}

			const char* str = lua_tostring(L, -1);
			ConColorMsg(LUA_COLOR, "%s\n", str);
			lua_pop(L, 1);
		}
	}

	return 0;
}

int LuaManager::LuaWarn(lua_State* L)
{
	int numArgs = lua_gettop(L);

	for (int i = 1; i <= numArgs; ++i)
	{
		if (lua_isstring(L, i))
		{
			const char* str = lua_tostring(L, i);
			Warning("%s\n", str);
		}
		else
		{
			lua_getglobal(L, "tostring");
			lua_pushvalue(L, i);

			if (lua_pcall(L, 1, 1, 0) != LUA_OK)
			{
				lua_pushfstring(L, "error calling tostring: %s", lua_tostring(L, -1));
				lua_error(L);
			}

			const char* str = lua_tostring(L, -1);
			Warning("%s\n", str);
			lua_pop(L, 1);
		}
	}

	return 0;
}

LuaManager::~LuaManager()
{
	if (L)
	{
		lua_close(L);
	}
}

#ifdef CLIENT_DLL
static ConCommand lua_dumpstack_c("lua_dumpstack_c", [](const CCommand& args)
    {
        if (globalLuaManager) {
            lua_State* L = globalLuaManager->GetState();
            if (L) {
                int top = lua_gettop(L);
                for (int i = 1; i <= top; ++i) {
                    int type = lua_type(L, i);
                    const char* typeName = lua_typename(L, type);
                    Msg("[%d] %s: ", i, typeName);
                    if (type == LUA_TSTRING) {
                        Msg("%s\n", lua_tostring(L, i));
                    }
                    else if (type == LUA_TNUMBER) {
                        Msg("%g\n", lua_tonumber(L, i));
                    }
                    else if (type == LUA_TBOOLEAN) {
                        Msg("%s\n", lua_toboolean(L, i) ? "true" : "false");
                    }
                    else {
                        Msg("unknown type\n");
                    }
                }
            }
            else {
                Msg("[LUA] State is not initialized\n");
            }
        }
        else {
            Warning("[LUA] Instance is not available\n");
        }
    }, "Dumps Lua stack (clientside)", FCVAR_NONE);

#else

static ConCommand lua_dumpstack("lua_dumpstack", [](const CCommand& args)
    {
        if (globalLuaManager) {
            lua_State* L = globalLuaManager->GetState();
            if (L) {
                int top = lua_gettop(L);
                for (int i = 1; i <= top; ++i) {
                    int type = lua_type(L, i);
                    const char* typeName = lua_typename(L, type);
                    Msg("[%d] %s: ", i, typeName);
                    if (type == LUA_TSTRING) {
                        Msg("%s\n", lua_tostring(L, i));
                    }
                    else if (type == LUA_TNUMBER) {
                        Msg("%g\n", lua_tonumber(L, i));
                    }
                    else if (type == LUA_TBOOLEAN) {
                        Msg("%s\n", lua_toboolean(L, i) ? "true" : "false");
                    }
                    else {
                        Msg("unknown type\n");
                    }
                }
            }
            else {
                Warning("[LUA] State is not initialized\n");
            }
        }
        else {
            Warning("[LUA] Instance is not available\n");
        }
    }, "Dumps Lua stack (serverside)", FCVAR_NONE);
#endif

#ifdef CLIENT_DLL
static ConCommand lua_restart_c("lua_restart_c", [](const CCommand& args)
    {
        if (globalLuaManager) {
            delete globalLuaManager;
            bool previousPrintMessage = LuaManager::printMessage;
            LuaManager::SetPrintMessage(false);
            globalLuaManager = new LuaManager();
            globalLuaManager->LoadDirectories(globalLuaManager->GetState());
            LuaManager::SetPrintMessage(previousPrintMessage);
            if (globalLuaManager) {
                Msg("[LUA] Lua restarted successfully\n");
            }
        }
        else {
            Warning("[LUA] Lua is not currently available\n");
        }
    }, "Restarts the Lua state (clientside)", FCVAR_CHEAT);
#else
static ConCommand lua_restart("lua_restart", [](const CCommand& args)
    {
        if (globalLuaManager) {
            delete globalLuaManager;
            bool previousPrintMessage = LuaManager::printMessage;
            LuaManager::SetPrintMessage(false);
            globalLuaManager = new LuaManager();
            globalLuaManager->LoadDirectories(globalLuaManager->GetState());
            LuaManager::SetPrintMessage(previousPrintMessage);
            if (globalLuaManager) {
                Msg("[LUA] Lua restarted successfully\n");
            }
        }
        else {
            Warning("[LUA] Lua is not currently available.\n");
        }
    }, "Restarts the Lua state (serverside)", FCVAR_CHEAT);
#endif

#ifndef CLIENT_DLL
static ConCommand lua_dofile("lua_dofile", [](const CCommand& args)
    {
        if (args.ArgC() < 2) {
            Msg("Usage: lua_dofile <file_path>\n");
            return;
        }

        if (globalLuaManager) {
            lua_State* L = globalLuaManager->GetState();
            if (L) {
                const char* filePath = args.ArgS();
                int result = luaL_dofile(L, filePath);
                if (result != LUA_OK) {
                    Warning("[LUA] Error executing file %s: %s\n", filePath, lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
            else {
                Warning("[LUA] State is not initialized\n");
            }
        }
        else {
            Warning("[LUA] Instance is not available\n");
        }
    }, "Executes a Lua file (serverside)", FCVAR_CHEAT);
#else
static ConCommand lua_dofile_c("lua_dofile_c", [](const CCommand& args)
    {
        if (args.ArgC() < 2) {
            Msg("Usage: lua_dofile_c <file_path>\n");
            return;
        }

        if (globalLuaManager) {
            lua_State* L = globalLuaManager->GetState();
            if (L) {
                const char* filePath = args.ArgS();
                int result = luaL_dofile(L, filePath);
                if (result != LUA_OK) {
                    Warning("[LUA] Error executing file %s: %s\n", filePath, lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
            else {
                Warning("[LUA] State is not initialized\n");
            }
        }
        else {
            Warning("[LUA] Instance is not available\n");
        }
    }, "Executes a Lua file (clientside)", FCVAR_CHEAT);
#endif

#ifdef CLIENT_DLL
static ConCommand lua_dostring_c("lua_dostring_c", [](const CCommand& args)
    {
        if (args.ArgC() < 2) {
            Msg("Usage: lua_dostring_c <code>\n");
            return;
        }

        if (globalLuaManager) {
            bool previousPrintMessage = LuaManager::printMessage;
            LuaManager::SetPrintMessage(false);

            lua_State* L = globalLuaManager->GetState();
            if (L) {
                const char* luaCode = args.ArgS();
                int result = luaL_dostring(L, luaCode);
                if (result != LUA_OK) {
                    Warning("[LUA] Error: %s\n", lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
            else {
                Warning("[LUA] State is not initialized\n");
            }

            LuaManager::SetPrintMessage(previousPrintMessage);
        }
        else {
            Warning("[LUA] Instance is not available\n");
        }
    }, "Executes Lua code (clientside)", FCVAR_CHEAT);
#else
static ConCommand lua_dostring("lua_dostring", [](const CCommand& args)
    {
        if (args.ArgC() < 2) {
            Msg("Usage: lua_dostring <code>\n");
            return;
        }

        if (globalLuaManager) {
            bool previousPrintMessage = LuaManager::printMessage;
            LuaManager::SetPrintMessage(false);

            lua_State* L = globalLuaManager->GetState();
            if (L) {
                const char* luaCode = args.ArgS();
                int result = luaL_dostring(L, luaCode);
                if (result != LUA_OK) {
                    Warning("[LUA] Error: %s\n", lua_tostring(L, -1));
                    lua_pop(L, 1);
                }
            }
            else {
                Warning("[LUA] State is not initialized\n");
            }

            LuaManager::SetPrintMessage(previousPrintMessage);
        }
        else {
            Warning("[LUA] Instance is not available\n");
        }
    }, "Executes Lua code (serverside)", FCVAR_CHEAT);
#endif
