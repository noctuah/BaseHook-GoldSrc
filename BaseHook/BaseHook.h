#include <Windows.h>
#include "HLSDK.h"
#include "CvarDef.h"
#include "MsgAPI.h"

#define	PITCH	0
#define	YAW		1
#define	ROLL	2 
#define POW(x) ((x)*(x))
#define M_PI 3.14159265358979323846
#define VectorDistance(a,b) sqrt(POW((a)[0]-(b)[0])+POW((a)[1]-(b)[1])+POW((a)[2]-(b)[2]))

void __cdecl HUD_Redraw(float time,int intermission);
void __cdecl HUD_Frame(double time);

void __cdecl HUD_PlayerMove(struct playermove_s *a, int b);

int pfnHookUserMsg( char *szMsgName, pfnUserMsgHook pfn );

void __cdecl V_CalcRefdef( struct ref_params_s *pparams );
void __cdecl CL_CreateMove( float frametime, struct usercmd_s *cmd, int active );

char g_szHackDir[256];
void GetDLLBaseDir();
void FindAll();
void LoadConfig();
DWORD WINAPI dwMainThread( LPVOID );

float PunchAngles[3];
float NoRecoilMultiplier = 0;
//void logme(char * fmt, ... );