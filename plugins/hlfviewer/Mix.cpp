#ifndef __mix_cpp
#define __mix_cpp

const TCHAR *GetMsg(int MsgId)
{
	return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

BOOL FileExists(const TCHAR* Name)
{
	return GetFileAttributes(Name)!=0xFFFFFFFF;
}

void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber)
{
	int I;
	struct FarDialogItem *PItem=Item;
	const struct InitDialogItem *PInit=Init;

	for (I=0; I<ItemsNumber; I++,PItem++,PInit++)
	{
		PItem->Type=PInit->Type;
		PItem->X1=PInit->X1;
		PItem->Y1=PInit->Y1;
		PItem->X2=PInit->X2;
		PItem->Y2=PInit->Y2;
		PItem->Focus=PInit->Focus;
		PItem->History=(const TCHAR *)PInit->Selected;
		PItem->Flags=PInit->Flags;
		PItem->DefaultButton=PInit->DefaultButton;
#ifndef UNICODE
		lstrcpy(PItem->Data,((DWORD_PTR)PInit->Data<2000)?GetMsg((unsigned int)(DWORD_PTR)PInit->Data):PInit->Data);
#else
		PItem->MaxLen=0;
		PItem->PtrData = ((DWORD_PTR)PInit->Data<2000)?GetMsg((unsigned int)(DWORD_PTR)PInit->Data):PInit->Data;
#endif
	}
}

BOOL CheckExtension(const TCHAR *ptrName)
{
	return Info.CmpName(_T("*.hlf"), ptrName, TRUE);
}

void ShowHelp(const TCHAR *fullfilename,const TCHAR *topic, bool CmdLine)
{
	if (CmdLine || CheckExtension(fullfilename))
	{
		const TCHAR *Topic=topic;

		if (NULL == Topic)
			Topic=GetMsg(MDefaultTopic);

		Info.ShowHelp(fullfilename,Topic,FHELP_CUSTOMFILE);
	}
}

#endif
