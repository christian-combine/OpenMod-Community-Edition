//========= Copyright OpenMod, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "mountsteamcontent.h"
#include "steam\steam_api.h"
#include "filesystem.h"

// from HL2SB, because i am a lazy ass
typedef struct
{
	const char* m_pPathName;
	int m_nAppId;
} gamePaths_t;

gamePaths_t g_GamePaths[11] =
{
	{ "hl2",		220 },
	{ "cstrike",	240 },
	{ "hl1",		280 },
	{ "dod",		300 },
	{ "lostcoast",	340 },
	{ "hl1mp",		360 },
	{ "episodic",	380 },
	{ "portal",		400 },
	{ "ep2",		420 },
	{ "hl2mp",		320 },
	{ "tf",			440 }
};

bool mountContent( int nExtraAppId )
{
	char szInstallDir[1024];
	if (!steamapicontext->SteamApps()->GetAppInstallDir(nExtraAppId, szInstallDir, sizeof(szInstallDir)))
	{
		return false; // fail..
	}
	char szSearchPath[1024];
	Q_snprintf(szSearchPath, sizeof(szSearchPath), "%s/*_dir.vpk", szInstallDir);

	FileFindHandle_t hFind;
	const char* pFilename = g_pFullFileSystem->FindFirst(szSearchPath, &hFind);

	if (!pFilename)
		return false;

	do
	{
		char szVPKFile[1024];
		Q_strncpy(szVPKFile, pFilename, sizeof(szVPKFile));

		szVPKFile[strlen(szVPKFile) - 8] = '\0';
		Q_strcat(szVPKFile, ".vpk", sizeof(szVPKFile));

		g_pFullFileSystem->AddSearchPath(szVPKFile, "VPK");

	} while (g_pFullFileSystem->FindNext(hFind));

	g_pFullFileSystem->FindClose(hFind);

	return true;
};

void addSearchPathByAppId( int nAppId )
{
	for (int i = 0; i < ARRAYSIZE(g_GamePaths); i++)
	{
		int iVal = g_GamePaths[i].m_nAppId;
		if (iVal == 360)
		{
			const char* pathName = g_GamePaths[2].m_pPathName;
			g_pFullFileSystem->AddSearchPath( pathName, "GAME", PATH_ADD_TO_TAIL );
		}
		if (iVal == nAppId)
		{
			const char* pathName = g_GamePaths[i].m_pPathName;
			g_pFullFileSystem->AddSearchPath( pathName, "GAME", PATH_ADD_TO_TAIL );
		}
	}
};

void mountGames(void)
{
	KeyValues* pMainFile, * pFileSystemInfo;
#ifdef CLIENT_DLL
	const char* gamePath = engine->GetGameDirectory();
#else
	char gamePath[256];
	engine->GetGameDir(gamePath, 256);
	Q_StripTrailingSlash(gamePath);
#endif

	pMainFile = new KeyValues("gamecontent.txt");
	if (pMainFile->LoadFromFile(filesystem, VarArgs("%s/gamecontent.txt", gamePath), "MOD"))
	{
		pFileSystemInfo = pMainFile->FindKey("FileSystem");
		if (pFileSystemInfo)
		{
			for (KeyValues* pKey = pFileSystemInfo->GetFirstSubKey(); pKey; pKey = pKey->GetNextKey())
			{
				if (strcmp(pKey->GetName(), "appid") == 0)
				{
					int nExtraContentId = pKey->GetInt();
					if (nExtraContentId)
					{
						addSearchPathByAppId(nExtraContentId);
						mountContent(nExtraContentId);
					}
				}
			}
		}
	}
	pMainFile->deleteThis();
};