//========= Copyright OpenMod, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "cbase.h"
#include "lsteamapi.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool LSteamAPI::IsSteamInitialized()
{
  return SteamAPI_IsSteamRunning();
}

int LSteamAPI::GetSteamID( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushnil( L );
    return 1;
  }

  CSteamID steamID = steamapicontext->SteamUser()->GetSteamID();
  lua_pushinteger( L, steamID.ConvertToUint64() );
  return 1;
}

int LSteamAPI::GetFriendCount( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushnil( L );
    return 1;
  }

  int friendCount = steamapicontext->SteamFriends()->GetFriendCount( k_EFriendFlagImmediate );
  lua_pushinteger( L, friendCount );
  return 1;
}

int LSteamAPI::GetFriendName( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushnil( L );
    return 1;
  }

  int friendIndex = luaL_checkinteger( L, 1 );
  CSteamID friendID = steamapicontext->SteamFriends()->GetFriendByIndex( friendIndex, k_EFriendFlagImmediate );
  const char* friendName = steamapicontext->SteamFriends()->GetFriendPersonaName( friendID );
  lua_pushstring( L, friendName );
  return 1;
}

int LSteamAPI::IsOnline( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushboolean( L, false );
    return 1;
  }

  bool isOnline = steamapicontext->SteamUser()->BLoggedOn();
  lua_pushboolean( L, isOnline );
  return 1;
}

int LSteamAPI::GetAppOwnership( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushboolean( L, false );
    return 1;
  }

  AppId_t appID = luaL_checkinteger( L, 1 );
  bool isOwned = steamapicontext->SteamApps()->BIsSubscribedApp( appID );
  lua_pushboolean( L, isOwned );
  return 1;
}

int LSteamAPI::GetPlayerName( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushnil( L );
    return 1;
  }

  const char* playerName = steamapicontext->SteamFriends()->GetPersonaName();
  lua_pushstring( L, playerName );
  return 1;
}

int LSteamAPI::GetSteamAppID( lua_State* L )
{
  AppId_t appID = steamapicontext->SteamUtils()->GetAppID();
  lua_pushinteger( L, appID );
  return 1;
}

int LSteamAPI::GetAchievement( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushboolean( L, false );
    return 1;
  }

  const char* achievementName = luaL_checkstring( L, 1 );
  bool achieved;
  steamapicontext->SteamUserStats()->GetAchievement( achievementName, &achieved );
  lua_pushboolean( L, achieved );
  return 1;
}

int LSteamAPI::SetAchievement( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushboolean( L, false );
    return 1;
  }

  const char* achievementName = luaL_checkstring( L, 1 );
  steamapicontext->SteamUserStats()->SetAchievement( achievementName );
  steamapicontext->SteamUserStats()->StoreStats();
  lua_pushboolean( L, true );
  return 1;
}

int LSteamAPI::ClearAchievement( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushboolean( L, false );
    return 1;
  }

  const char* achievementName = luaL_checkstring( L, 1 );
  steamapicontext->SteamUserStats()->ClearAchievement( achievementName );
  steamapicontext->SteamUserStats()->StoreStats();
  lua_pushboolean( L, true );
  return 1;
}

int LSteamAPI::GetAllAchievements( lua_State* L )
{
  if ( !IsSteamInitialized() )
  {
    lua_pushnil( L );
    return 1;
  }

  int achievementCount = steamapicontext->SteamUserStats()->GetNumAchievements();

  lua_newtable( L );
  for ( int i = 0; i < achievementCount; i++ )
  {
    const char* achievementName = steamapicontext->SteamUserStats()->GetAchievementName( i );
    bool achieved;
    steamapicontext->SteamUserStats()->GetAchievement( achievementName, &achieved );

    lua_pushstring( L, achievementName );
    lua_pushboolean( L, achieved );
    lua_settable( L, -3 );
  }

  return 1;
}

int LSteamAPI::FindLeaderboard( lua_State* L )
{
  const char* leaderboardName = luaL_checkstring( L, 1 );
  SteamAPICall_t hSteamAPICall = steamapicontext->SteamUserStats()->FindLeaderboard( leaderboardName );
  lua_pushinteger( L, hSteamAPICall );
  return 1;
}

void LSteamAPI::Register( lua_State* L )
{
  lua_newtable( L );
  luaL_newmetatable( L, "SteamServiceMeta" );

  lua_pushcfunction( L, GetSteamID );
  lua_setfield( L, -2, "GetSteamID" );

  lua_pushcfunction( L, GetFriendCount );
  lua_setfield( L, -2, "GetFriendCount" );

  lua_pushcfunction( L, GetFriendName );
  lua_setfield( L, -2, "GetFriendName" );

  lua_pushcfunction( L, IsOnline );
  lua_setfield( L, -2, "IsOnline" );

  lua_pushcfunction( L, GetAppOwnership );
  lua_setfield( L, -2, "GetAppOwnership" );

  lua_pushcfunction( L, GetPlayerName );
  lua_setfield( L, -2, "GetPlayerName" );

  lua_pushcfunction( L, GetSteamAppID );
  lua_setfield( L, -2, "GetSteamAppID" );

  lua_pushcfunction( L, GetAchievement );
  lua_setfield( L, -2, "GetAchievement" );

  lua_pushcfunction( L, SetAchievement );
  lua_setfield( L, -2, "SetAchievement" );

  lua_pushcfunction( L, ClearAchievement );
  lua_setfield( L, -2, "ClearAchievement" );

  lua_pushcfunction( L, GetAllAchievements );
  lua_setfield( L, -2, "GetAllAchievements" );

  lua_pushcfunction( L, FindLeaderboard );
  lua_setfield( L, -2, "FindLeaderboard" );

  lua_setfield( L, -2, "__index" );
  lua_setmetatable( L, -2 );
  lua_setglobal( L, "SteamService" );

  lua_pushcfunction( L, []( lua_State* L ) -> int
                     {
        lua_getglobal(L, "SteamService");
        return 1; } );
}