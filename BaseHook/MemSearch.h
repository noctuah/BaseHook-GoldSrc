#pragma once
#include <Windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <io.h>
#include <iostream>
#include <Commctrl.h>
 
BOOL __comparemem(const UCHAR *buff1, const UCHAR *buff2, UINT size);
ULONG __findmemoryclone(const ULONG start, const ULONG end, const ULONG clone, UINT size);
ULONG __findreference(const ULONG start, const ULONG end, const ULONG address);
 DWORD MEM_FindPattern( DWORD dwAddress, DWORD dwSize, BYTE* pbMask, char* szMask );
 bool bDataCompare( const BYTE* pData, const BYTE* bMask, const char* szMask );
 
#define CompareMemory(Buff1, Buff2, Size) __comparemem((const UCHAR *)Buff1, (const UCHAR *)Buff2, (UINT)Size)
#define FindMemoryClone(Start, End, Clone, Size) __findmemoryclone((const ULONG)Start, (const ULONG)End, (const ULONG)Clone, (UINT)Size)
#define FindReference(Start, End, Address) __findreference((const ULONG)Start, (const ULONG)End, (const ULONG)Address)

bool Initialize();
HANDLE FindEngine();
HANDLE FindClient();
HANDLE FindStudio();
HANDLE FindPlayersInfo();
PVOID SpeedHackPtr();