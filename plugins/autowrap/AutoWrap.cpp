#include <cwchar>
#include <plugin.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "WrapLng.hpp"
#include "version.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

static PluginStartupInfo PsInfo;
static FarStandardFunctions FSF;

static struct Options
{
	wchar_t FileMasks[512];
	wchar_t ExcludeFileMasks[512];
	int RightMargin;
	int Wrap;
} Opt;

const wchar_t *GetCommaWord(const wchar_t *Src, wchar_t *Word);

static const wchar_t *GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid, MsgId);
}

void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const PluginStartupInfo *Info)
{
	PsInfo=*Info;
	FSF=*PsInfo.FSF;
	PsInfo.FSF=&FSF;
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);
	Opt.Wrap=settings.Get(0,L"Wrap",0);
	Opt.RightMargin=settings.Get(0,L"RightMargin",75);
	settings.Get(0,L"FileMasks",Opt.FileMasks,ARRAYSIZE(Opt.FileMasks),L"*.*");
	settings.Get(0,L"ExcludeFileMasks",Opt.ExcludeFileMasks,ARRAYSIZE(Opt.ExcludeFileMasks),L"");
}

void WINAPI GetPluginInfoW(PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MAutoWrap);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	PluginDialogBuilder Builder(PsInfo, MainGuid, DialogGuid, MAutoWrap, nullptr);
	Builder.AddCheckbox(MEnableWrap, &Opt.Wrap);
	FarDialogItem *RightMargin = Builder.AddIntEditField(&Opt.RightMargin, 3);
	Builder.AddTextAfter(RightMargin, MRightMargin);
	Builder.AddSeparator();
	Builder.AddText(MFileMasks);
	Builder.AddEditField(Opt.FileMasks, ARRAYSIZE(Opt.FileMasks), 65);
	Builder.AddText(MExcludeFileMasks);
	Builder.AddEditField(Opt.ExcludeFileMasks, ARRAYSIZE(Opt.ExcludeFileMasks), 65);
	Builder.AddOKCancel(MOk, MCancel);

	if(Builder.ShowDialog())
	{
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		settings.Set(0,L"Wrap",Opt.Wrap);
		settings.Set(0,L"RightMargin",Opt.RightMargin);
		settings.Set(0,L"FileMasks",Opt.FileMasks);
		settings.Set(0,L"ExcludeFileMasks",Opt.ExcludeFileMasks);
	}

	return nullptr;
}


intptr_t WINAPI ProcessEditorInputW(const ProcessEditorInputInfo *InputInfo)
{
	if(!Opt.Wrap)
		return FALSE;

	static int Reenter=FALSE;

	if(Reenter || InputInfo->Rec.EventType!=KEY_EVENT || !InputInfo->Rec.Event.KeyEvent.bKeyDown || InputInfo->Rec.Event.KeyEvent.wVirtualKeyCode==VK_F1)
		return FALSE;

	EditorInfo startei= {sizeof(EditorInfo)};
	PsInfo.EditorControl(-1,ECTL_GETINFO,0,&startei);
	EditorGetString prevegs= {sizeof(EditorGetString)};
	prevegs.StringNumber=-1;
	PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&prevegs);
	Reenter=TRUE;
	PsInfo.EditorControl(-1,ECTL_PROCESSINPUT,0,const_cast<INPUT_RECORD*>(&InputInfo->Rec));
	Reenter=FALSE;

	for(int Pass=1;; Pass++)
	{
		EditorInfo ei= {sizeof(EditorInfo)};
		PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);
		LPWSTR FileName=nullptr;
		size_t FileNameSize=PsInfo.EditorControl(-1,ECTL_GETFILENAME,0,{});

		if(FileNameSize)
		{
			FileName=new wchar_t[FileNameSize];
			PsInfo.EditorControl(-1,ECTL_GETFILENAME,FileNameSize,FileName);
		}

		if(Pass==1 && *Opt.FileMasks)
		{
			if(ei.CurLine!=startei.CurLine)
			{
				delete[] FileName;
				return TRUE;
			}

			bool Found=false;
			wchar_t FileMask[MAX_PATH];
			const wchar_t *MaskPtr=Opt.FileMasks;

			while((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=nullptr)
			{
				if(FSF.ProcessName(FileMask,FileName,0,PN_CMPNAME|PN_SKIPPATH))
				{
					Found=true;
					break;
				}
			}

			if(!Found)
			{
				delete[] FileName;
				return TRUE;
			}

			MaskPtr=Opt.ExcludeFileMasks;

			while((MaskPtr=GetCommaWord(MaskPtr,FileMask))!=nullptr)
			{
				if(FSF.ProcessName(FileMask,FileName,0,PN_CMPNAME|PN_SKIPPATH))
				{
					Found=false;
					break;
				}
			}

			if(!Found)
			{
				delete[] FileName;
				return TRUE;
			}
		}

		delete[] FileName;

		EditorGetString egs= {sizeof(EditorGetString)};
		egs.StringNumber=ei.CurLine;
		PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);
		bool TabPresent=wmemchr(egs.StringText,L'\t',egs.StringLength)!=nullptr;
		intptr_t TabLength=egs.StringLength;

		if(TabPresent)
		{
			EditorConvertPos ecp= {sizeof(EditorConvertPos)};
			ecp.StringNumber=-1;
			ecp.SrcPos=egs.StringLength;
			PsInfo.EditorControl(-1,ECTL_REALTOTAB,0,&ecp);
			TabLength=ecp.DestPos;
		}

		if((Pass!=1 || prevegs.StringLength!=egs.StringLength) &&
		        TabLength>=Opt.RightMargin && ei.CurPos>=egs.StringLength)
		{
			intptr_t SpacePos=-1;

			for(intptr_t I=egs.StringLength-1; I>0; I--)
			{
				if(egs.StringText[I]==L' ' || egs.StringText[I]==L'\t')
				{
					SpacePos=I;
					intptr_t TabPos=I;

					if(TabPresent)
					{
						EditorConvertPos ecp= {sizeof(EditorConvertPos)};
						ecp.StringNumber=-1;
						ecp.SrcPos=I;
						PsInfo.EditorControl(-1,ECTL_REALTOTAB,0,&ecp);
						TabPos=ecp.DestPos;
					}

					if(TabPos<Opt.RightMargin)
						break;
				}
			}

			if(SpacePos<=0)
				break;

			bool SpaceOnly=true;

			for(int I=0; I<SpacePos; I++)
			{
				if(egs.StringText[I]!=L' ' && egs.StringText[I]!=L'\t')
				{
					SpaceOnly=false;
					break;
				}
			}

			if(SpaceOnly)
				break;

			EditorSetPosition esp= {sizeof(EditorSetPosition),-1,-1,-1,-1,-1,-1};
			esp.CurPos=SpacePos+1;
			PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
			int Indent=TRUE;

			if(!PsInfo.EditorControl(-1,ECTL_INSERTSTRING,0,&Indent))
				break;

			if(ei.CurPos<SpacePos)
			{
				esp.CurLine=ei.CurLine;
				esp.CurPos=ei.CurPos;
				PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
			}
			else
			{
				egs.StringNumber=ei.CurLine+1;
				PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);
				esp.CurLine=ei.CurLine+1;
				esp.CurPos=egs.StringLength;
				PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
			}

			PsInfo.EditorControl(-1,ECTL_REDRAW,0,{});
		}
		else
			break;
	}

	return TRUE;
}

const wchar_t *GetCommaWord(const wchar_t *Src, wchar_t *Word)
{
	if(*Src==L'\0')
		return nullptr;

	int WordPos=0;
	bool SkipBrackets=false;

	for(; *Src!=L'\0'; Src++,WordPos++)
	{
		if(*Src==L'[' && wcschr(Src+1,L']')!=nullptr)
			SkipBrackets=true;

		if(*Src==L']')
			SkipBrackets=false;

		if(*Src==L',' && !SkipBrackets)
		{
			Word[WordPos]=0;
			Src++;

			while(iswspace(*Src))
				Src++;

			return Src;
		}
		else
		{
			Word[WordPos]=*Src;
		}
	}

	Word[WordPos]=0;
	return Src;
}
