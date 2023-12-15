#include "MemSearch.h"
#include "CvarDef.h"

DWORD ClBase = 0;
DWORD ClSize = 0;
DWORD ClEnd = 0;

DWORD HwBase = 0;
DWORD HwSize = 0;
DWORD HwEnd = 0;

DWORD GetModuleSize(const DWORD Address)
{
	return PIMAGE_NT_HEADERS(Address + (DWORD)PIMAGE_DOS_HEADER(Address)->e_lfanew)->OptionalHeader.SizeOfImage;
}


bool Initialize(void)
{
	HwBase = (DWORD)GetModuleHandle("hw.dll"); // Hardware
	if ( HwBase == NULL )
	{
		HwBase = (DWORD)GetModuleHandle("sw.dll"); // Software
		if ( HwBase == NULL )
		{
			HwBase = (DWORD)GetModuleHandle(NULL); // Non-Steam?
			if ( HwBase == NULL ) // Invalid module handle.
			{

			}
		}
	}

	HwSize = (DWORD)GetModuleSize(HwBase);

	if ( HwSize == NULL )
	{
		switch(HwSize)
		{
		case 2: HwSize = 0x122A000;break;
		case 0: HwSize = 0x2116000;break;
		case 1: HwSize = 0xB53000;break;
		}
	}
	HwEnd = HwBase + HwSize - 1;

	return (HwBase);
}


BOOL __comparemem(const UCHAR *buff1, const UCHAR *buff2, UINT size)
{
    for (UINT i = 0; i < size; i++, buff1++, buff2++)
    {
        if ((*buff1 != *buff2) && (*buff2 != 0xFF))
            return FALSE;
    }
    return TRUE;
}
 
 
ULONG __findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size)
{
    for (ULONG ul = start; (ul + size) < end; ul++)
    {
        if (CompareMemory(ul, clone, size))
            return ul;
    }
    return NULL;
}
 
 
ULONG __findreference(const ULONG start, const ULONG end, const ULONG address)
{
    UCHAR Pattern[5];
    Pattern[0] = 0x68;
    *(ULONG*)&Pattern[1] = address;
    return FindMemoryClone(start, end, Pattern, sizeof(Pattern)-1);
}

DWORD MEM_FindPattern( DWORD dwAddress, DWORD dwSize, BYTE* pbMask, char* szMask )
{
	for( DWORD i = NULL; i < dwSize; i++ )
	{
		if( bDataCompare( (BYTE*)( dwAddress + i ), pbMask, szMask ) )
		{
			return (DWORD)( dwAddress + i );
		}
	}
	return 0;
}

bool bDataCompare( const BYTE* pData, const BYTE* bMask, const char* szMask )
{
	for( ; *szMask; ++szMask, ++pData, ++bMask )
	{
		if( *szMask == 'x' && *pData != *bMask )
		{
			return false;
		}
	}
	return (*szMask) == NULL;
}

PVOID SpeedHackPtr()
{
	DWORD Old = NULL;
	PCHAR String = "Texture load: %6.1fms";
	DWORD Address = (DWORD)FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
	PVOID SpeedPtr = (PVOID)*(DWORD*)(FindReference(HwBase, HwBase+HwSize, Address) - 7);
	
	VirtualProtect(SpeedPtr,sizeof(double),PAGE_READWRITE,&Old);
	
	return SpeedPtr;
}

void * FindEngine()
{
	PCHAR String = "ScreenFade";
	DWORD Address = FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
	PVOID EnginePtr = (PVOID)*(PDWORD)(FindReference(HwBase, HwBase+HwSize, Address) + 0x0D);
	return EnginePtr;
}

HANDLE FindClient()
{
    PCHAR String = "ScreenFade";
    DWORD Address = FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
    return (void*)*(DWORD*)(FindReference(HwBase, HwBase+HwSize, Address) + 0x13);
}

DWORD FarProc(const DWORD Address, DWORD LB, DWORD HB)
{
	return ( (Address < LB) || (Address > HB) );
}

HANDLE FindStudio()
{
    /*
	NOT WORKING SHIT!
	
	PCHAR String = "Couldn't get client .dll studio model rendering interface.";
    DWORD Address = FindMemoryClone(HwBase, HwBase+HwSize, String, strlen(String));
    return (void*)*(DWORD*)(FindReference(HwBase, HwBase+HwSize, Address) - 0x14);
	
	NOT WORKING SHIT!
	*/

	PVOID StudioPtr = (engine_studio_api_t*)*(DWORD*)((DWORD)PClient->HUD_GetStudioModelInterface + 0x30); // old patch
	if(FarProc((DWORD)StudioPtr, HwBase, HwEnd))
		StudioPtr = (engine_studio_api_t*)*(DWORD*)((DWORD)PClient->HUD_GetStudioModelInterface + 0x1A); // new patch | steam
		
	
	return StudioPtr;
}

HANDLE FindPlayersInfo() {
    PCHAR String = "cl_career_difficulty";
    DWORD Address = FindMemoryClone(ClBase, ClBase+ClSize, String, strlen(String));

	ClEnd = ClBase + ClSize - 1;

	unsigned p = 47;

    return (void*)*(DWORD*)(FindReference(ClBase, ClBase+ClSize, Address) - 8 + p * sizeof( unsigned ) );
}
