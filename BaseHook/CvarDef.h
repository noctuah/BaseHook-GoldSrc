#pragma once
#include "HLSDK.h"

extern cl_enginefunc_t Engine;
extern cl_enginefunc_t *PEngine;

extern cl_clientfunc_t *PClient;
extern cl_clientfunc_t Client;

extern engine_studio_api_t Studio;
extern engine_studio_api_t *PStudio;

extern cs_player_info_s * PPlayerInfo;
extern player_s g_Player[33];

static int g_PlayerTeam[33];


extern local_s g_Local;

static bool FirstFrame = true;

//usermsgs_s UserMsgs;

extern pfnUserMsgHook pTeamInfo;
extern pfnUserMsgHook pDeathMsg;
extern pfnUserMsgHook pCurWeapon;
extern pfnUserMsgHook pSetFOV;

static int WinCenterX = 0;
static int WinCenterY = 0;


#define WEAPONLIST_P228 1
#define WEAPONLIST_UNKNOWN1 2
#define WEAPONLIST_SCOUT 3
#define WEAPONLIST_HEGRENADE 4
#define WEAPONLIST_XM1014 5
#define WEAPONLIST_C4 6
#define WEAPONLIST_MAC10 7
#define WEAPONLIST_AUG 8
#define WEAPONLIST_SMOKEGRENADE 9
#define WEAPONLIST_ELITE 10
#define WEAPONLIST_FIVESEVEN 11
#define WEAPONLIST_UMP45 12
#define WEAPONLIST_SG550 13
#define WEAPONLIST_UNKNOWN2 14
#define WEAPONLIST_UNKNOWN3 15
#define WEAPONLIST_USP 16
#define WEAPONLIST_GLOCK18 17
#define WEAPONLIST_AWP 18
#define WEAPONLIST_MP5 19
#define WEAPONLIST_M249 20
#define WEAPONLIST_M3 21
#define WEAPONLIST_M4A1 22
#define WEAPONLIST_TMP 23
#define WEAPONLIST_G3SG1 24
#define WEAPONLIST_FLASHBANG 25
#define WEAPONLIST_DEAGLE 26
#define WEAPONLIST_SG552 27
#define WEAPONLIST_AK47 28
#define WEAPONLIST_KNIFE 29
#define WEAPONLIST_P90 30

static int iMode = 0, iOldMode = 0;

float fAngleBetvenVectors(Vector a,Vector b,float lena = 0,float lenb = 0);