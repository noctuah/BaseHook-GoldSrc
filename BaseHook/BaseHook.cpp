#pragma warning( disable : 4244)
#pragma warning( disable : 4018)
#pragma warning( disable : 4996)

#include "cvar.h"
#include <Windows.h>
#include "HLSDK.h"
#include "BaseHook.h"
#include "MemSearch.h"

#include <fstream>
#include <iostream>

void AdjustSpeed(double x) {
	if(g_Local.dwSpeedptr == 0)
		g_Local.dwSpeedptr = g_Local.dwSpeedPtrOld;

	static double LastSpeed=1;
	if(x!=LastSpeed) {
		*(double*)g_Local.dwSpeedptr = (x * 1000);
		LastSpeed=x;
	}
}

using namespace std;

config_s CFG[5];

pfnUserMsgHook pTeamInfo = NULL;
pfnUserMsgHook pDeathMsg = NULL;
pfnUserMsgHook pCurWeapon = NULL;
pfnUserMsgHook pSetFOV = NULL;

cl_enginefunc_t Engine;
cl_enginefunc_t *PEngine = PEngine;

cl_clientfunc_t *PClient = NULL;
cl_clientfunc_t Client;

engine_studio_api_t Studio;
engine_studio_api_t *PStudio = PStudio;

cs_player_info_s * PPlayerInfo = NULL;

local_s g_Local;
player_s g_Player[33];

ofstream ofile;
HINSTANCE NewhinstDLL;

float fAngleBetvenVectors(Vector a,Vector b,float lena,float lenb) {
	float l1=0.0f, l2=0.0f;
	if(lena>0)l1=lena;else l1=a.Length();
	if(lenb>0)l2=lenb;else l2=b.Length();
	float sc=a.x*b.x+a.y*b.y+a.z*b.z;
	return acos(sc/(l1*l2))*(180.0/M_PI);
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved ) {
	if( dwReason == DLL_PROCESS_ATTACH ) {
		NewhinstDLL = (HINSTANCE) hModule;
		CreateThread(0,0,dwMainThread,0,0,0);
	}
	if (dwReason == DLL_PROCESS_DETACH) {
		TerminateProcess(GetCurrentProcess(),0);
	} return TRUE;
}

void enableDebugPrivileges() {
    HANDLE hcurrent=GetCurrentProcess();
    HANDLE hToken;
    BOOL bret=OpenProcessToken(hcurrent,40,&hToken);
    LUID luid;
    bret=LookupPrivilegeValue(NULL,"SeDebugPrivilege",&luid);
    TOKEN_PRIVILEGES NewState,PreviousState;
    DWORD ReturnLength;
    NewState.PrivilegeCount =1;
    NewState.Privileges[0].Luid =luid;
    NewState.Privileges[0].Attributes=2;
    AdjustTokenPrivileges(hToken,FALSE,&NewState,28,&PreviousState,&ReturnLength);
}

DWORD WINAPI dwMainThread( LPVOID ) {
	FindAll();
	return TRUE;
}

int AddCommand( char *cmd_name, void (*function)(void) ){return 0;}

cvar_t* RegisterVariable ( char *szName, char *szValue, int flags )  { 
	cvar_t* pResult = Engine.pfnGetCvarPointer(szName); 
	if(pResult != NULL) 
		return pResult; 
	return Engine.pfnRegisterVariable(szName, szValue, flags); 
}

void StudioEntityLight(struct alight_s *plight) {
	Studio.StudioEntityLight(plight);

	cl_entity_s *ent = Studio.GetCurrentEntity();

	if(isValidEnt(ent) && CFG[0].light != 0) {
		plight->ambientlight = CFG[0].light_value; 
		plight->color[0]=255/255.0f;
		plight->color[1]=255/255.0f;
		plight->color[2]=255/255.0f;
	}
}

void FindAll() {	
	// CreateConsole() ;
	 enableDebugPrivileges();
	while (!Initialize())
		Sleep(500);
	Sleep(2000);
	GetDLLBaseDir();
	
	PEngine = (cl_enginefunc_t*)FindEngine();
	PClient = (cl_clientfunc_t*)FindClient();
	PStudio = (engine_studio_api_t*)FindStudio();

	Hook:
	RtlCopyMemory(&Client, PClient, sizeof(cl_clientfunc_t));
	RtlCopyMemory(&Engine, PEngine, sizeof(cl_enginefunc_t));	
	RtlCopyMemory(&Studio, PStudio, sizeof(engine_studio_api_t));


	if (!Client.V_CalcRefdef || !Engine.V_CalcShake || !Studio.StudioSetupSkin)
		goto Hook;


	//VirtualProtect((LPVOID)PClient,sizeof(cl_clientfunc_t), PAGE_EXECUTE_WRITECOPY, 0);

	g_Local.dwSpeedptr = 0;
	g_Local.dwSpeedPtrOld = 0;
	g_Local.dwSpeedPtrOld = (DWORD)SpeedHackPtr();
	

	PClient->HUD_Redraw = HUD_Redraw;
	PClient->HUD_Frame = HUD_Frame;
	PClient->HUD_PlayerMove = HUD_PlayerMove;
	PClient->V_CalcRefdef = V_CalcRefdef;
	PClient->CL_CreateMove = CL_CreateMove;

	PEngine->pfnHookUserMsg = &pfnHookUserMsg;
	PEngine->pfnRegisterVariable = &RegisterVariable;
	PEngine->pfnAddCommand = &AddCommand;

	PStudio->StudioEntityLight = StudioEntityLight;

	Client.Initialize(PEngine,CLDLL_INTERFACE_VERSION);
	Client.HUD_Init();
	PEngine->pfnHookUserMsg = Engine.pfnHookUserMsg;

	PEngine->pfnAddCommand = Engine.pfnAddCommand; 
	PEngine->pfnRegisterVariable = Engine.pfnRegisterVariable;
}

void GetDLLBaseDir() {
	// DisableThreadLibraryCalls( (HMODULE)NewhinstDLL );
	GetModuleFileNameA( (HMODULE)NewhinstDLL, g_szHackDir, 256 );
	char *p = g_szHackDir + strlen(g_szHackDir);
	while( p >= g_szHackDir && *p != '\\' ) { --p; }
	p[1] = '\0';

	// strcat(g_szHackDir,"cstrike\\");
	char LogFile[1024];
	strcpy(LogFile,g_szHackDir);
	strcat(LogFile,"\\LogSignature.txt");

	 // remove(LogFile);
}

void GetPlayerHeight(struct playermove_s *ppmove) {
	Vector vTemp = ppmove->origin; vTemp[2] -= 8192;
	pmtrace_t *trace = Engine.PM_TraceLine(ppmove->origin, vTemp, 1, ppmove->usehull, -1); 
	g_Local.fGroundAngle = acos(trace->plane.normal[2])/M_PI*180;

	Vector vTemp1=trace->endpos;
	pmtrace_t pTrace;

	Engine.pEventAPI->EV_SetTraceHull(ppmove->usehull);
	Engine.pEventAPI->EV_PlayerTrace(ppmove->origin, vTemp1, PM_GLASS_IGNORE | PM_STUDIO_BOX, g_Local.iIndex, &pTrace );

	if(pTrace.fraction < 1.0f) {
		g_Local.fHeight = abs(pTrace.endpos.z - ppmove->origin.z);
		int ind = Engine.pEventAPI->EV_IndexFromTrace(&pTrace);
		if(ind >= 1 && ind <= 32) {
			float dst = ppmove->origin.z-(ppmove->usehull==0?32:18)-g_Player[ind].vOrigin.z-g_Local.fHeight;
			if(dst<30) {
				g_Local.bOnPlayer = true;
				g_Local.fHeight -= 14.0000;
			} else g_Local.bOnPlayer = false;
		}
	} else {
		if(g_Local.fGroundAngle>1) {
			Vector vTemp2 = ppmove->origin;
			vTemp2[2] -= 8192;
			pmtrace_t *trace4 = Engine.PM_TraceLine(ppmove->origin, vTemp2, 1, 2, -1);
			g_Local.fHeight=abs(ppmove->origin.z-trace4->endpos.z-(ppmove->usehull==1?18.0f:36.0f));
		} else {
			Vector vTemp5 = ppmove->origin;
			vTemp5[2] -= 8192;
			pmtrace_t *trace5 = Engine.PM_TraceLine(ppmove->origin, vTemp5, 1, ppmove->usehull, -1); 
			g_Local.fHeight=abs(trace5->endpos.z - ppmove->origin.z);
		}
	}
}

void __cdecl HUD_PlayerMove(struct playermove_s *ppmove, int server) {	
	Client.HUD_PlayerMove( ppmove, server );

	Engine.pEventAPI->EV_LocalPlayerViewheight(g_Local.vEye);
	g_Local.vEye = g_Local.vEye + ppmove->origin;
	g_Local.iFlags = ppmove->flags;
	g_Local.vVelocity = ppmove->velocity;
	g_Local.iOnLadder = ppmove->movetype==5;
	g_Local.fFallSpeed = ppmove->flFallVelocity;
	g_Local.fSpeed = sqrt(POW(ppmove->velocity[0]) + POW(ppmove->velocity[1]));

	GetPlayerHeight(ppmove);
}

bool bPathFree( float *pflFrom, float *pflTo ) {
	if( !pflFrom || !pflTo ) { return false; }
	pmtrace_t pTrace;
	Engine.pEventAPI->EV_SetTraceHull( 2 );
	Engine.pEventAPI->EV_PlayerTrace( pflFrom, pflTo, PM_GLASS_IGNORE | PM_STUDIO_BOX, g_Local.iIndex, &pTrace );
	return ( pTrace.fraction == 1.0f );
}

bool InTargetDiap(int x, int y, int Distance) {
	float Width = (1.0f / (Distance)) * 100000;
	float W = Width / 2;
	if (WinCenterX < (x - W))
		return false;
	if (WinCenterX > (x + W))
		return false;
	if (WinCenterY < (y - W))
		return false;
	if (WinCenterY > (y + W))
		return false;
	return true;
}

#define VectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}

bool bBadWpn() {
	if(g_Local.iWeaponID == WEAPONLIST_C4 ||
		g_Local.iWeaponID == WEAPONLIST_FLASHBANG ||
		g_Local.iWeaponID == WEAPONLIST_HEGRENADE ||
		g_Local.iWeaponID == WEAPONLIST_KNIFE ||
		g_Local.iWeaponID == WEAPONLIST_SMOKEGRENADE)
		return true;

	else return false;
}

bool bSniper() {
	if(!bBadWpn() && (g_Local.iWeaponID == WEAPONLIST_AWP ||
		g_Local.iWeaponID == WEAPONLIST_SCOUT ||
		g_Local.iWeaponID == WEAPONLIST_G3SG1 ||
		g_Local.iWeaponID == WEAPONLIST_SG550))
		return true;

	else return false;
}

bool bPistol() {
	if(!bBadWpn() && !bSniper() && (g_Local.iWeaponID == WEAPONLIST_GLOCK18 ||
		g_Local.iWeaponID == WEAPONLIST_USP ||
		g_Local.iWeaponID == WEAPONLIST_P228 ||
		g_Local.iWeaponID == WEAPONLIST_DEAGLE ||
		g_Local.iWeaponID == WEAPONLIST_ELITE ||
		g_Local.iWeaponID == WEAPONLIST_FIVESEVEN))
		return true;

	else return false;
}

bool bAuto() {
	if(!bBadWpn() && !bSniper() && !bPistol())
		return true;
	
	else return false;
}

void LoadConfig() {
	
	char inifile[1024];
	strcpy(inifile, g_szHackDir);

	strcat(inifile, "config.ini");

	//BAD WEAPON SETTINGS
	CFG[1].recoil = GetPrivateProfileInt("Bad", "a_adjustment_recoil", 0, inifile);
	CFG[1].calc_recoil = GetPrivateProfileInt("Bad", "a_automatic_calc_recoil", 0, inifile);

	CFG[1].aim_active = GetPrivateProfileInt("Bad", "a_aim_active", 0, inifile);
	CFG[1].aim_smooth = GetPrivateProfileInt("Bad", "a_aim_smooth", 0, inifile);
	CFG[1].aim_x_stand = GetPrivateProfileInt("Bad", "a_aim_x_stand", 0, inifile);
	CFG[1].aim_y_stand = GetPrivateProfileInt("Bad", "a_aim_y_stand", 0, inifile);
	CFG[1].aim_z_stand = GetPrivateProfileInt("Bad", "a_aim_z_stand", 0, inifile);
	CFG[1].aim_x_duck = GetPrivateProfileInt("Bad", "a_aim_x_duck", 0, inifile);
	CFG[1].aim_y_duck = GetPrivateProfileInt("Bad", "a_aim_y_stand", 0, inifile);
	CFG[1].aim_z_duck = GetPrivateProfileInt("Bad", "a_aim_z_stand", 0, inifile);
	CFG[1].aim_x_jump = GetPrivateProfileInt("Bad", "a_aim_x_jump", 0, inifile);
	CFG[1].aim_y_jump = GetPrivateProfileInt("Bad", "a_aim_y_jump", 0, inifile);
	CFG[1].aim_z_jump = GetPrivateProfileInt("Bad", "a_aim_z_jump", 0, inifile);


	//PISTOLS SETTINGS
	CFG[2].recoil = GetPrivateProfileInt("Pistol", "a_adjustment_recoil", 0, inifile);
	CFG[2].calc_recoil = GetPrivateProfileInt("Pistol", "a_automatic_calc_recoil", 0, inifile);

	CFG[2].aim_active = GetPrivateProfileInt("Pistol", "a_aim_active", 0, inifile);
	CFG[2].aim_smooth = GetPrivateProfileInt("Pistol", "a_aim_smooth", 0, inifile);
	CFG[2].aim_x_stand = GetPrivateProfileInt("Pistol", "a_aim_x_stand", 0, inifile);
	CFG[2].aim_y_stand = GetPrivateProfileInt("Pistol", "a_aim_y_stand", 0, inifile);
	CFG[2].aim_z_stand = GetPrivateProfileInt("Pistol", "a_aim_z_stand", 0, inifile);
	CFG[2].aim_x_duck = GetPrivateProfileInt("Pistol", "a_aim_x_duck", 0, inifile);
	CFG[2].aim_y_duck = GetPrivateProfileInt("Pistol", "a_aim_y_stand", 0, inifile);
	CFG[2].aim_z_duck = GetPrivateProfileInt("Pistol", "a_aim_z_stand", 0, inifile);
	CFG[2].aim_x_jump = GetPrivateProfileInt("Pistol", "a_aim_x_jump", 0, inifile);
	CFG[2].aim_y_jump = GetPrivateProfileInt("Pistol", "a_aim_y_jump", 0, inifile);
	CFG[2].aim_z_jump = GetPrivateProfileInt("Pistol", "a_aim_z_jump", 0, inifile);

	//AUTOMATIC WEAPONS SETTINGS
	CFG[3].recoil = GetPrivateProfileInt("Automatic", "a_adjustment_recoil", 0, inifile);
	CFG[3].calc_recoil = GetPrivateProfileInt("Automatic", "a_automatic_calc_recoil", 0, inifile);

	CFG[3].aim_active = GetPrivateProfileInt("Automatic", "a_aim_active", 0, inifile);
	CFG[3].aim_smooth = GetPrivateProfileInt("Automatic", "a_aim_smooth", 0, inifile);
	CFG[3].aim_x_stand = GetPrivateProfileInt("Automatic", "a_aim_x_stand", 0, inifile);
	CFG[3].aim_y_stand = GetPrivateProfileInt("Automatic", "a_aim_y_stand", 0, inifile);
	CFG[3].aim_z_stand = GetPrivateProfileInt("Automatic", "a_aim_z_stand", 0, inifile);
	CFG[3].aim_x_duck = GetPrivateProfileInt("Automatic", "a_aim_x_duck", 0, inifile);
	CFG[3].aim_y_duck = GetPrivateProfileInt("Automatic", "a_aim_y_stand", 0, inifile);
	CFG[3].aim_z_duck = GetPrivateProfileInt("Automatic", "a_aim_z_stand", 0, inifile);
	CFG[3].aim_x_jump = GetPrivateProfileInt("Automatic", "a_aim_x_jump", 0, inifile);
	CFG[3].aim_y_jump = GetPrivateProfileInt("Automatic", "a_aim_y_jump", 0, inifile);
	CFG[3].aim_z_jump = GetPrivateProfileInt("Automatic", "a_aim_z_jump", 0, inifile);

	//SNIPER SETTINGS
	CFG[4].recoil = GetPrivateProfileInt("Sniper", "a_adjustment_recoil", 0, inifile);
	CFG[4].calc_recoil = GetPrivateProfileInt("Sniper", "a_automatic_calc_recoil", 0, inifile);

	CFG[4].aim_active = GetPrivateProfileInt("Sniper", "a_aim_active", 0, inifile);
	CFG[4].aim_smooth = GetPrivateProfileInt("Sniper", "a_aim_smooth", 0, inifile);
	CFG[4].aim_x_stand = GetPrivateProfileInt("Sniper", "a_aim_x_stand", 0, inifile);
	CFG[4].aim_y_stand = GetPrivateProfileInt("Sniper", "a_aim_y_stand", 0, inifile);
	CFG[4].aim_z_stand = GetPrivateProfileInt("Sniper", "a_aim_z_stand", 0, inifile);
	CFG[4].aim_x_duck = GetPrivateProfileInt("Sniper", "a_aim_x_duck", 0, inifile);
	CFG[4].aim_y_duck = GetPrivateProfileInt("Sniper", "a_aim_y_stand", 0, inifile);
	CFG[4].aim_z_duck = GetPrivateProfileInt("Sniper", "a_aim_z_stand", 0, inifile);
	CFG[4].aim_x_jump = GetPrivateProfileInt("Sniper", "a_aim_x_jump", 0, inifile);
	CFG[4].aim_y_jump = GetPrivateProfileInt("Sniper", "a_aim_y_jump", 0, inifile);
	CFG[4].aim_z_jump = GetPrivateProfileInt("Sniper", "a_aim_z_jump", 0, inifile);

	//OTHER SETTINGS
	CFG[0].bhop = GetPrivateProfileInt("Base", "m_bhop", 0, inifile);
	CFG[0].str = GetPrivateProfileInt("Base", "m_strafehelper", 0, inifile);
	CFG[0].str_key = GetPrivateProfileInt("Base", "m_strafehelper_key", 0, inifile);
	CFG[0].light = GetPrivateProfileInt("Base", "m_players_light", 0, inifile);
	CFG[0].light_value = GetPrivateProfileInt("Base", "m_players_light_key", 0, inifile);
	CFG[0].vector = GetPrivateProfileInt("Base", "m_recoil_vector", 0, inifile);
}

Vector vCalcOriginOffset(int iIndex) {
	cl_entity_s *ent = Engine.GetEntityByIndex(iIndex);

	Vector vAngles,vF,vR,vU,vOut;

	vAngles=Vector(0.0f,ent->angles[1],0.0f);

	Engine.pfnAngleVectors(vAngles,vF,vR,vU);

	if(ent->curstate.gaitsequence == 2 || ent->curstate.gaitsequence == 5) vOut = ent->origin+vF*CFG[iMode].aim_x_duck+vR*CFG[iMode].aim_y_duck+vU*CFG[iMode].aim_z_duck; 
	else if(ent->curstate.gaitsequence == 6) vOut = ent->origin+vF*CFG[iMode].aim_x_jump+vR*CFG[iMode].aim_y_jump+vU*CFG[iMode].aim_z_jump; 
	else vOut = ent->origin+vF*CFG[iMode].aim_x_stand+vR*CFG[iMode].aim_y_stand+vU*CFG[iMode].aim_z_stand; 
	
	return vOut;
}

void SmoothAimAngles(float* MyViewAngles, float* AimAngles, float* OutAngles, float Smoothing) {
	float *Out= new float[3];

	VectorSubtract(AimAngles, MyViewAngles, Out)

		//-3 и 3 - диапазон, в котором будет наводка
		//по идее чем он больше, тем меньше трясёт

	if(Out[0] < -8)
		OutAngles[0] = MyViewAngles[0] - 50/Smoothing;
	else if(Out[0] > 8)
		OutAngles[0] = MyViewAngles[0] + 50/Smoothing;
	else 
		OutAngles[0] = AimAngles[0];

	if(Out[1] < -8)
		OutAngles[1] = MyViewAngles[1] - 50/Smoothing;
	else if(Out[1] > 8)
		OutAngles[1] = MyViewAngles[1] + 50/Smoothing;
	else 
		OutAngles[1] = AimAngles[1];
}

void __cdecl HUD_Redraw(float time,int intermission) {
	Client.HUD_Redraw( time, intermission );

	cl_entity_t * Local = Engine.GetLocalPlayer();
	g_Local.iIndex = Local->index;

	float recoil = CFG[iMode].calc_recoil == 1 ? (((WinCenterX * 2.0f) * 
		(WinCenterY * 2.0f)) * 20.0f) / 2073600.0f : CFG[iMode].recoil;

	float y = (PunchAngles[0] * recoil);
	float x = (PunchAngles[1] * recoil);

	if(CFG[0].vector)
		Engine.pfnFillRGBA(WinCenterX - x-1,WinCenterY + y-1,3,3,255,1,1,255);

	int NearestPlayerID = 0;
	int SmallestDistance = 99999999;
	bool TargetFound = false;

	//update players.
	for (unsigned int i = 1; i <= Engine.GetMaxClients(); ++i) {
		cl_entity_s* ent = Engine.GetEntityByIndex(i);
		Engine.pfnGetPlayerInfo(i,&g_Player[i].Info);	 // get info
		g_Player[i].bVisible = bPathFree(g_Local.vEye,vCalcOriginOffset(i)); // виден ли игрок?
		g_Player[i].fov = fAngleBetvenVectors(g_Local.vForward,vCalcOriginOffset(i)-g_Local.vEye);
		g_Player[i].distance = VectorDistance(g_Local.vEye,vCalcOriginOffset(i));
		g_Player[i].iTeam = g_PlayerTeam[i]; // team gg
		g_Player[i].vOrigin = ent->origin; // положения игрока в пространстве
		g_Player[i].bAlive = ent && !(ent->curstate.effects & EF_NODRAW) && ent->player && ent->curstate.movetype !=6 && ent->curstate.movetype != 0;
		g_Player[i].vAimOrigin = vCalcOriginOffset(i);
	}
	float fSmoothAngles[2] = {};

	//Get nearest player id
	for (unsigned int i = 1; i <= Engine.GetMaxClients(); ++i) {
		if (i == Local->index) continue;
		if (((Local->curstate.iuser1 != 0) && (Local->curstate.iuser2 == i)) || 
			(Local->curstate.iuser1 == 5) || (Local->curstate.iuser1 == 6)) continue;
		if (!g_Player[i].bAlive) continue;

		cl_entity_s * pEnt = Engine.GetEntityByIndex(i);
		if (!isValidEnt(pEnt)) continue;

		float * vPlayerPos = new float[3];
		//pEnt->origin.z += 21;
		//Local->origin.z += 21;
		if (!CalcScreen2(vCalcOriginOffset(i),vPlayerPos)) continue;;
		if (!(vPlayerPos[0] > 0 && vPlayerPos[0] < WinCenterX*2 && 
			vPlayerPos[1] > 0 && vPlayerPos[1] < WinCenterY*2)) continue;
		if (vPlayerPos[2] == -1) continue;
		short Team = g_PlayerTeam[i];//PPlayerInfo[i]->Team;
		short PlayerTeam = g_PlayerTeam[Local->index];//PPlayerInfo[Local->index]->Team;
		if ((Team != 0) && (Team != 1) && (Team != 2) && (Team != 3)) continue;
		if ((PlayerTeam != 0) && (PlayerTeam != 1) && (PlayerTeam != 2) && (PlayerTeam != 3)) continue;
		int Dist =  VectorDistance(Local->origin,pEnt->origin);// CalcDistance(Local->origin,pEnt->origin);

		bool bInTargetDiap = false;
		if (x > vPlayerPos[0])
			bInTargetDiap = InTargetDiap(vPlayerPos[0] - x,vPlayerPos[1] - y,Dist);
		else
			bInTargetDiap = InTargetDiap(vPlayerPos[0] + x,vPlayerPos[1] - y,Dist);

		if ((Dist < SmallestDistance)  && (bInTargetDiap) && 
			(g_Player[i].bVisible) && (PlayerTeam != Team) && (Team != PlayerTeam)) {
			SmallestDistance = Dist;
			NearestPlayerID = i;
			TargetFound = true;
		}
	}
	
	if (TargetFound && CFG[iMode].aim_active) {
		cl_entity_s * pEnt = Engine.GetEntityByIndex(NearestPlayerID);
		float * vPlayerPos = new float[3];

		CalcScreen2(vCalcOriginOffset(NearestPlayerID),vPlayerPos);

		if(CFG[iMode].aim_smooth > 0) {
			POINT cursor;
			GetCursorPos(&cursor);	

			SmoothAimAngles(Vector(cursor.x,cursor.y,0),vPlayerPos,fSmoothAngles,CFG[iMode].aim_smooth);
		} else {
			fSmoothAngles[0] = vPlayerPos[0];
			fSmoothAngles[1] = vPlayerPos[1];
		}
		
		if (GetAsyncKeyState(1) != 0 && g_Local.iClip)  {
			if (x > fSmoothAngles[0])
				SetCursorPos(fSmoothAngles[0] - x,fSmoothAngles[1] - y);
			else
				SetCursorPos(fSmoothAngles[0] + x,fSmoothAngles[1] - y);
		}
	}
}

void __cdecl HUD_Frame(double time) {
	if (FirstFrame) {

		Engine.pfnClientCmd("clear");
		Engine.pfnClientCmd("cl_minmodels 1");
		Engine.pfnClientCmd("m_rawinput 0");
		Engine.pfnClientCmd("hud_fastswith 1");
		Engine.pfnClientCmd("cl_righthand 0");
		Engine.pfnClientCmd("rate 100000");

		FirstFrame = false;

		LoadConfig();

	}
	
	WinCenterX = Engine.GetWindowCenterX();
	WinCenterY = Engine.GetWindowCenterY();

	Client.HUD_Frame(time);
}

void __cdecl V_CalcRefdef( struct ref_params_s *pparams ) {
	PunchAngles[0] = pparams->punchangle[0];
	PunchAngles[1] = pparams->punchangle[1];
	PunchAngles[2] = pparams->punchangle[2];

	g_Local.vForward = pparams->forward;
	g_Local.vPunchangle = pparams->punchangle;

	Client.V_CalcRefdef(pparams);
}

void BunnyHop(struct usercmd_s *cmd) {

	static bool lastFramePressedJump=false;
	static bool JumpInNextFrame=false;
	bool curFramePressedJump=cmd->buttons&IN_JUMP;

	if(JumpInNextFrame) {
		JumpInNextFrame=false;

		cmd->buttons|=IN_JUMP;

		goto bhopfuncend;
	}
	static int inAirBhopCnt=0;bool isJumped=false;

	if(CFG[0].bhop!=0 && curFramePressedJump ) {

		cmd->buttons &= ~IN_JUMP;

		if((!lastFramePressedJump || g_Local.iFlags&FL_ONGROUND || g_Local.iWaterLevel >= 2 || 
			g_Local.iOnLadder==1 || g_Local.fHeight<=2)) {
			
			if(true) {
				static int bhop_jump_number=0;
				bhop_jump_number++;
				if(bhop_jump_number>=3) {
					bhop_jump_number = 0;
					JumpInNextFrame = true; 
					goto bhopfuncend;
				}
			} {
				inAirBhopCnt=4;isJumped=true;
				cmd->buttons |= IN_JUMP;

			}
		}
	} 
	if(!isJumped) {
		if(inAirBhopCnt>0) {
			if(inAirBhopCnt%2==0) cmd->buttons |= IN_JUMP;
			else cmd->buttons &= ~IN_JUMP;

			inAirBhopCnt--;
		}
	} bhopfuncend:lastFramePressedJump=curFramePressedJump;
}

float fYawForVec(float *fwd)
{
	if (fwd[1] == 0 && fwd[0] == 0) return 0;
	else
	{
		float yaw = (atan2(fwd[1], fwd[0]) * 180 / M_PI);

		if(yaw < 0) yaw += 360;

		return yaw;
	}
}

void RotateInvisible(float yaw, float pitch, struct usercmd_s *cmd) {
	Vector viewforward, viewright, viewup, aimforward, aimright, aimup, vTemp,tipo_real_va;

	float newforward, newright, newup;
	float forward = cmd->forwardmove;
	float right = cmd->sidemove;
	float up = cmd->upmove;
	
	tipo_real_va = cmd->viewangles;

	Engine.pfnAngleVectors(Vector(0.0f, tipo_real_va.y, 0.0f), viewforward, viewright, viewup);
	tipo_real_va.y += yaw;
	tipo_real_va.x += pitch;
	Engine.pfnAngleVectors(Vector(0.0f, tipo_real_va.y, 0.0f), aimforward, aimright, aimup);

	newforward = DotProduct(forward * viewforward.Normalize(), aimforward) + DotProduct(right * viewright.Normalize(), aimforward) + DotProduct(up * viewup.Normalize(), aimforward);
	newright = DotProduct(forward * viewforward.Normalize(), aimright) + DotProduct(right * viewright.Normalize(), aimright) + DotProduct(up * viewup.Normalize(), aimright);
	newup = DotProduct(forward * viewforward.Normalize(), aimup) + DotProduct(right * viewright.Normalize(), aimup) + DotProduct(up * viewup.Normalize(), aimup);

	if(pitch>81)
		cmd->forwardmove = -newforward;
	else 
		cmd->forwardmove = newforward;

	cmd->sidemove = newright;
	cmd->upmove = newup;
}

void Strafe(float frametime, struct usercmd_s *cmd) {
	if(GetAsyncKeyState(CFG[0].str_key) && !(g_Local.iFlags&FL_ONGROUND) && g_Local.iOnLadder==0) {
		
		if(g_Local.fSpeed<15) {	
			cmd->forwardmove=400;
			cmd->sidemove=0;

			return;
		}

		float va_real[3]={};Engine.GetViewAngles(va_real);
		float vspeed[3]={g_Local.vVelocity.x/g_Local.fSpeed,g_Local.vVelocity.y/g_Local.fSpeed,0.0f};
		float va_speed=fYawForVec(vspeed);

		float adif=va_speed-va_real[1];
		while(adif<-180)adif+=360;
		while(adif>180)adif-=360;


		cmd->sidemove=(435)*(adif>0?1:-1);
		cmd->forwardmove=0;

		bool onlysidemove=(abs(adif)>=atan(30.0/g_Local.fSpeed)/M_PI*180);
		int aaddtova=0;

		RotateInvisible(-(adif),0,cmd);

		float fs=0;
		if(!onlysidemove) {
			static float lv=0;
			Vector fw=g_Local.vForward;fw[2]=0;fw=fw.Normalize();
			float vel=0;
			
			vel = POW(fw[0]*g_Local.vVelocity[0])+POW(fw[1]*g_Local.vVelocity[1]);
			lv=sqrt(6900000/vel);
			fs=lv;

			static float lastang=0;
			float ca=abs(adif);

			lastang=ca;
		} cmd->forwardmove+=fs;
	}
}

void __cdecl CL_CreateMove( float frametime, struct usercmd_s *cmd, int active ) {
	Client.CL_CreateMove( frametime, cmd, active );

	g_Local.angles = cmd->viewangles;

	if(bBadWpn()) iMode = 1;
	else if(bPistol()) iMode = 2;
	else if(bAuto()) iMode = 3;
	else iMode = 4;

	AdjustSpeed(1);
	
	if(CFG[0].bhop) BunnyHop(cmd);
	if(CFG[0].str) Strafe(frametime,cmd);

	if ((GetAsyncKeyState(VK_HOME) & 0x8000)) {
		LoadConfig();
		Engine.Con_NPrintf(1, " \n Config reloaded! \n ");
	}
	/* 
	
	//SOME DEBUG INFO :)

	if(bBadWpn())
		Engine.Con_NPrintf(1,"Bad Weapon!");
	else if(bPistol())
		Engine.Con_NPrintf(1,"Pistols now!");
	else if(bSniper())
		Engine.Con_NPrintf(1,"Snipers now!");
	else if(bAuto())
		Engine.Con_NPrintf(1,"Automats now!");
	else 
		Engine.Con_NPrintf(1,"ERROR!");
		*/
}

int TeamInfo( const char *pszName, int iSize, void *pbuf ) {
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();
	char *szTeam = READ_STRING();
	_strlwr( szTeam );
	if( !strcmp( szTeam, "terrorist" ) ) {
		g_Player[iIndex].iTeam = 1;
		if( iIndex == g_Local.iIndex ) { g_Local.iTeam = 1; }
		g_PlayerTeam[iIndex] = 1;
	} else if( !strcmp( szTeam, "ct" ) ) {
		g_Player[iIndex].iTeam = 2;
		if( iIndex == g_Local.iIndex ) { g_Local.iTeam = 2; }
		g_PlayerTeam[iIndex]=2;
	} else {
		g_Player[iIndex].iTeam = 0;
		if( iIndex == g_Local.iIndex ) { g_Local.iTeam = 0; }
		g_PlayerTeam[iIndex] = 0;
	}
	return (*pTeamInfo)( pszName, iSize, pbuf );
}

int DeathMsg( const char *pszName, int iSize, void *pbuf ) {
	BEGIN_READ( pbuf, iSize );
	int iKiller = READ_BYTE();
	int iVictim = READ_BYTE();
	int iHeadshot = READ_BYTE();
	char* pszWeaponName = READ_STRING();
	
	g_Player[iVictim].bAlive = false;
	g_Player[iVictim].bVisible = false;
	if( iVictim == g_Local.iIndex ) { g_Local.bAlive = false; }
	return (*pDeathMsg)( pszName, iSize, pbuf );
}

int CurWeapon(const char *pszName, int iSize, void *pbuf) {
	BEGIN_READ(pbuf, iSize);
	
	int iState = READ_BYTE();
	int iWeaponID = READ_CHAR();
	int iClip = READ_CHAR();
	
	if(iState) {
		g_Local.iClip = iClip;
		g_Local.iWeaponID = iWeaponID;
	}
	return (*pCurWeapon)(pszName, iSize, pbuf);
}

int pfnHookUserMsg( char *szMsgName, pfnUserMsgHook pfn ) {
	if( !strcmp( szMsgName, "TeamInfo" ) ) {
		pTeamInfo = pfn;
		return Engine.pfnHookUserMsg( szMsgName, TeamInfo );
	} else if( !strcmp( szMsgName, "DeathMsg" ) ) {
		pDeathMsg = pfn;
		return Engine.pfnHookUserMsg( szMsgName, DeathMsg );
	}
	else if (!strcmp(szMsgName, "CurWeapon")) {
		pCurWeapon = pfn;
		return Engine.pfnHookUserMsg(szMsgName, CurWeapon);
	}
	return Engine.pfnHookUserMsg( szMsgName, pfn );
}