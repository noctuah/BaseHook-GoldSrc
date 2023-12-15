#pragma once
#include "HLSDK.h"
void HookUserMsgs();

typedef struct usermsg_s {
	int number;
	int size;
	char name[16];
	struct usermsg_s* next;
	pfnUserMsgHook pfn;
} 
usermsg_t;

struct usermsgs_s {
	pfnUserMsgHook TeamInfo;
	pfnUserMsgHook CurWeapon;
	pfnUserMsgHook ScoreAttrib;                                              
	pfnUserMsgHook SetFOV;
	pfnUserMsgHook Health;
	pfnUserMsgHook Battery;
	pfnUserMsgHook ScoreInfo;
	pfnUserMsgHook DeathMsg;
	pfnUserMsgHook SayText;
	pfnUserMsgHook TextMsg;
	pfnUserMsgHook ResetHUD;
	pfnUserMsgHook Damage;
};

extern usermsgs_s g_UserMsgs;

struct player_s {
	bool bUpdated;
	bool bDucked;
	bool bAlive;
	bool bVisible;
	float distance,fov;
	int iTeam;
	vec3_t vOrigin,vAimOrigin;
	hud_player_info_t Info;
};

typedef struct cl_clientfunc_s {
	int(*Initialize)(cl_enginefunc_t *pEnginefuncs,int iVersion);//0
	int(*HUD_Init)(void );//1
	int(*HUD_VidInit)(void );//2
	void(*HUD_Redraw)(float time,int intermission);//3
	int(*HUD_UpdateClientData)(client_data_t *pcldata,float flTime);//4
	int(*HUD_Reset)(void);//5
	void(*HUD_PlayerMove)(struct playermove_s *ppmove,int server);//6
	void(*HUD_PlayerMoveInit)(struct playermove_s *ppmove);//7
	char(*HUD_PlayerMoveTexture)(char *name);//8
	void(*IN_ActivateMouse)(void);//9
	void(*IN_DeactivateMouse)(void);//10
	void(*IN_MouseEvent)(int mstate);//11
	void(*IN_ClearStates)(void);//12
	void(*IN_Accumulate)(void);//13
	void(*CL_CreateMove)(float frametime,struct usercmd_s *cmd,int active);//14
	int(*CL_IsThirdPerson)(void);//15
	void(*CL_CameraOffset)(float *ofs);//16
	struct kbutton_s *(*KB_Find)(const char *name);//17
	void(*CAM_Think)(void);//18
	void(*V_CalcRefdef)(struct ref_params_s *pparams);//19
	int(*HUD_AddEntity)(int type,struct cl_entity_s *ent,const char *modelname);//20
	void(*HUD_CreateEntities)(void);//21
	void(*HUD_DrawNormalTriangles)(void);//22
	void(*HUD_DrawTransparentTriangles)(void);//23
	void(*HUD_StudioEvent)(const struct mstudioevent_s *event,const struct cl_entity_s *entity);//24
	void(*HUD_PostRunCmd)(struct local_state_s *from,struct local_state_s *to,struct usercmd_s *cmd,int runfuncs,double time,unsigned int random_seed);//25
	void(*HUD_Shutdown)(void);//26
	void(*HUD_TxferLocalOverrides)(struct entity_state_s *state,const struct clientdata_s *client);//27
	void(*HUD_ProcessPlayerState)(struct entity_state_s *dst,const struct entity_state_s *src);//28
	void(*HUD_TxferPredictionData)(struct entity_state_s *ps,const struct entity_state_s *pps,struct clientdata_s *pcd,const struct clientdata_s *ppcd,struct weapon_data_s *wd,const struct weapon_data_s *pwd);//29
	void(*Demo_ReadBuffer)(int size,unsigned char *buffer);//30
	int(*HUD_ConnectionlessPacket)(struct netadr_s *net_from,const char *args,char *response_buffer,int *response_buffer_size);//31
	int(*HUD_GetHullBounds)(int hullnumber,float *mins,float *maxs);//32
	void(*HUD_Frame)(double time);//33
	int(*HUD_Key_Event)(int down,int keynum,const char *pszCurrentBinding);
	void(*HUD_TempEntUpdate)(double frametime,double client_time,double cl_gravity,struct tempent_s **ppTempEntFree,struct tempent_s **ppTempEntActive,int (*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity),void(*Callback_TempEntPlaySound)(struct tempent_s *pTemp,float damp));
	struct cl_entity_s *(*HUD_GetUserEntity)(int index);
	int(*HUD_VoiceStatus)(int entindex,qboolean bTalking);
	int(*HUD_DirectorMessage)(unsigned char command,unsigned int firstObject,unsigned int secondObject,unsigned int flags);
	int(*HUD_GetStudioModelInterface)(int version,struct r_studio_interface_s **ppinterface,struct engine_studio_api_s *pstudio);
	void(*HUD_CHATINPUTPOSITION_FUNCTION)(int *x,int *y);
	int(*HUD_GETPLAYERTEAM_FUNCTION)(int iplayer);
	void(*CLIENTFACTORY)(void);

} cl_clientfunc_t;


typedef struct cs_player_info_record_s {
  short Kills, Deaths;
  int ClassID, IsVIP, HasBomb;
  Vector RadarPos;
  int UpdateCount, MinUpdate, MaxUpdate;
  short SBarTeam, Team;
  char TeamName[16];
  int IsDead;
  float NextUpdateTime;
  int Health;
  char Location[32];
} cs_player_info_record_t;

struct local_s {
	long dwSpeedptr, dwSpeedPtrOld;

	int iIndex;
	int iTeam;
	int iClip;
	int iInReload;
	float flNextAttack;
	int iWeaponID;
	int iFOV,iOnLadder;
	bool bAlive,bOnPlayer;
	Vector vOrigin;
	Vector vEye;

	int iFlags;
	int iWaterLevel;
	int iUseHull;

	float fGroundAngle, fFallSpeed, fSpeed;
	float fOnLadder;
	float fHeight;

	Vector vPunchangle,vForward,angles;
	Vector vVelocity;
};

typedef	cs_player_info_record_t cs_player_info_t[32];
typedef cs_player_info_t cs_player_info_s;

extern cl_clientfunc_t *pClient;
extern cl_clientfunc_t g_Client;