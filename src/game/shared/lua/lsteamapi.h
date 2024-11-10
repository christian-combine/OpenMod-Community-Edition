//========= Copyright Wheatley Laboratories, All rights reserved. ============//
//
// Purpose: stupid source modder 👍 
// 
// Purpose: developer of:
// Purpose: alter source, half life 2 community edition, artemus mod
//
//=============================================================================
#ifndef LSTEAMAPI_H
#define LSTEAMAPI_H

#include <steam/steam_api.h>
#include "luamanager.h"

class LSteamAPI
{
 public:
  static int GetSteamID( lua_State* L );
  static int GetFriendCount( lua_State* L );
  static int GetFriendName( lua_State* L );
  static int IsOnline( lua_State* L );
  static int GetAppOwnership( lua_State* L );
  static int GetPlayerName( lua_State* L );
  static int GetSteamAppID( lua_State* L );
  static int GetAchievement( lua_State* L );
  static int SetAchievement( lua_State* L );
  static int ClearAchievement( lua_State* L );
  static int GetAllAchievements( lua_State* L );
  static int FindLeaderboard( lua_State* L );

  static void Register( lua_State* L );

 private:
  static bool IsSteamInitialized();
};

#endif  // LSTEAMAPI_H
