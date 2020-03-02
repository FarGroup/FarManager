#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

BOOL WINAPI FP_CopyToClipboard(LPVOID Data, SIZE_T DataSize)
{
	HGLOBAL  hData;
	void    *GData;
	BOOL     rc = FALSE;

	if(!Data || !DataSize ||
	        !OpenClipboard(NULL))
		return FALSE;

	EmptyClipboard();

	do
	{
		if((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,DataSize+1))!=NULL)
		{
			if((GData=GlobalLock(hData))!=NULL)
			{
				memcpy(GData,Data,DataSize+1);
				GlobalUnlock(hData);
				SetClipboardData(CF_OEMTEXT,(HANDLE)hData);
				rc = TRUE;
			}
			else
			{
				GlobalFree(hData);
				break;
			}
		}

		if((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,DataSize+1))!=NULL)
		{
			if((GData=GlobalLock(hData))!=NULL)
			{
				memcpy(GData,Data,DataSize+1);
				OemToChar((LPCSTR)GData,(LPTSTR)GData);
				GlobalUnlock(hData);
				SetClipboardData(CF_TEXT,(HANDLE)hData);
				rc = TRUE;
			}
			else
			{
				GlobalFree(hData);
				break;
			}
		}

		if((hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,DataSize*2+2))!=NULL)
		{
			if((GData=GlobalLock(hData))!=NULL)
			{
				MultiByteToWideChar(CP_OEMCP,0,(LPCSTR)Data,-1,(LPWSTR)GData,(int)DataSize);
				GlobalUnlock(hData);
				SetClipboardData(CF_UNICODETEXT,(HANDLE)hData);
				rc = TRUE;
			}
			else
			{
				GlobalFree(hData);
				break;
			}
		}
	}
	while(0);

	CloseClipboard();
	return rc;
}

BOOL WINAPI FP_GetFromClipboard(LPVOID& Data, SIZE_T& DataSize)
{
	HANDLE   hData;
	void    *GData;
	BOOL     rc = FALSE;
	Data     = NULL;
	DataSize = 0;

	if(!OpenClipboard(NULL)) return FALSE;

	do
	{
		hData = GetClipboardData(CF_TEXT);

		if(!hData)
			break;

		DataSize = GlobalSize(hData);

		if(!DataSize)
			break;

		GData = GlobalLock(hData);

		if(!GData)
			break;

		Data = malloc(DataSize+1);
		memcpy(Data,GData,DataSize);
		rc = TRUE;
	}
	while(0);

	CloseClipboard();
	return rc;
}
