#include <all_far.h>
#pragma hdrstop

#include "fstdlib.h"

//---------------------------------------------------------------------------------
FP_ItemList::FP_ItemList(BOOL NeedToDelete)
{
	needToDelete = NeedToDelete;
	List         = NULL;
	ItemsCount   = 0;
	MaxCount     = 0;
}

BOOL FP_ItemList::Realloc(int NewSize)
{
	if(!NewSize)
		return FALSE;

	if(NewSize <= MaxCount)
		return TRUE;

	MaxCount = NewSize;

	if(!List)
		List = (PluginPanelItem*)malloc(sizeof(PluginPanelItem)*MaxCount);
	else
		List = (PluginPanelItem *)realloc(List,sizeof(PluginPanelItem)*MaxCount);

	if(!List)
	{
		ItemsCount = 0;
		MaxCount   = 0;
		return FALSE;
	}

	return TRUE;
}
PluginPanelItem *FP_ItemList::Add(const PluginPanelItem *pi,int icn)
{
	if(!Realloc(ItemsCount+icn))
		return NULL;

	PluginPanelItem *p = List + ItemsCount; //!! Do not use Item(ItemsCount) because we need point after last element
	Copy(p, pi, icn);
	ItemsCount += icn;
	return p;
}

void FP_ItemList::Free(void)
{
	if(needToDelete)
		Free(List,ItemsCount);

	ItemsCount = 0;
	MaxCount   = 0;
	List       = NULL;
}

void FP_ItemList::Copy(PluginPanelItem *dest,const PluginPanelItem *src,int cn)
{
	if(!cn) return;

	memmove(dest, src, sizeof(*dest)*cn);

	for(; cn; cn--,src++,dest++)
	{
		//User data
		if(IS_FLAG(src->Flags,PPIF_USERDATA))
		{
			DWORD sz = (src->UserData && !IsBadReadPtr((void*)src->UserData,sizeof(DWORD)))
			           ? (*((DWORD*)src->UserData))
			           : 0;

			if(sz && !IsBadReadPtr((void*)src->UserData,sz))
			{
				dest->UserData = (DWORD_PTR)malloc(sz+1);
				memmove((char*)dest->UserData,(char*)src->UserData,sz);
			}
			else
			{
				dest->UserData = 0;
				CLR_FLAG(dest->Flags,PPIF_USERDATA);
			}
		}

		//CustomColumn
		if(src->CustomColumnNumber)
		{
			dest->CustomColumnData = (LPSTR*)malloc(sizeof(LPSTR*)*src->CustomColumnNumber);

			for(int n = 0; n < src->CustomColumnNumber; n++)
			{
				dest->CustomColumnData[n] = strdup(src->CustomColumnData[n]);
			}
		}

		//Description
		if(src->Description)
			dest->Description = strdup(src->Description);

		//Owner
		if(src->Owner)
			dest->Owner = strdup(src->Owner);

		//Additionals
		if(FPIL_ADDEXIST(src))
		{
			DWORD  sz  = FPIL_ADDSIZE(src);
			LPVOID ptr = malloc(sz);

			if(ptr)
			{
				memmove(ptr, FPIL_ADDDATA(src), sz);
				FPIL_ADDSET(dest, sz, ptr);
			}
			else
				FPIL_ADDSET(dest, 0, NULL);
		}
	}
}

void FP_ItemList::Free(PluginPanelItem *List,int count)
{
	PluginPanelItem *p = List;

	for(int i = 0; i < count; i++,List++)
	{
		//UserData
		if(IS_FLAG(List->Flags,PPIF_USERDATA))
		{
			free((char*)List->UserData);
			List->UserData = 0;
		}

		//CustomColumn
		for(int n = 0; n < List->CustomColumnNumber; n++)
			free(List->CustomColumnData[n]);

		if(List->CustomColumnData)
			free(List->CustomColumnData);

		//Description
		if(List->Description)
			free(List->Description), List->Description = NULL;

		//Owner
		if(List->Owner)
			free(List->Owner), List->Owner = NULL;

		//Additionals
		if(FPIL_ADDEXIST(List))
		{
			free(FPIL_ADDDATA(List));
			List->Reserved[0] = 0;
			List->Reserved[1] = 0;
		}
	}

	free(p);
}
