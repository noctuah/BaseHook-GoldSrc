#include <Windows.h>

void Error(const char * msg) {
	MessageBox(0, msg, "Fatal Error", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
}

bool isValidEnt(cl_entity_s *ent) {
	if( (ent->index!=g_Local.iIndex) && !(ent->curstate.effects & EF_NODRAW) && ent->player && (ent->model->name !=0 || ent->model->name !="")  && !(ent->curstate.messagenum < PEngine->GetLocalPlayer()->curstate.messagenum))
		return true;
	else
		return false;
}


/*vec3_t CalcScreen(Vector Origin)
//var vec: vec3_t;
{
	//vec3_t *vec ;
	//vec3_t *org ;
	/*if (PEngine->pTriAPI->WorldToScreen(org, vec) == 0)
	{
		if( (vec->x < 1) && (vec->y < 1) && (vec->z > -1) )
		{
			vec->x = vec->x * WinCenterX + WinCenterX;
			vec->y = -vec->y * WinCenterY + WinCenterY;
			vec->z = 0;
		}
	}
	else
		vec->z = -1;
	return Origin;
}*/

bool CalcScreen2( float *origin, float * vecScreen) {
	int cResult = Engine.pTriAPI->WorldToScreen( origin, vecScreen );
	if( vecScreen[0] < 1 && vecScreen[1] < 1 && vecScreen[0] > -1 && vecScreen [1] > -1 && !cResult ) {
		vecScreen[0] = vecScreen[0] * WinCenterX + WinCenterX;
		vecScreen[1] = -vecScreen[1] * WinCenterY + WinCenterY;
		return true;
	} return false;
}