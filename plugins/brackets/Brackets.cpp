#include <algorithm>
#include <plugin.hpp>
#include <PluginSettings.hpp>
#include <DlgBuilder.hpp>
#include "Brackets.hpp"
#include "BrackLng.hpp"
#include "version.hpp"

#include "guid.hpp"
#include <initguid.h>
#include "guid.hpp"

static Options Opt;
static PluginStartupInfo PsInfo;

static const wchar_t *GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid,MsgId);
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
	PluginSettings settings(MainGuid, PsInfo.SettingsControl);
	Opt.IgnoreQuotes=settings.Get(0,L"IgnoreQuotes",0);
	Opt.IgnoreAfter=settings.Get(0,L"IgnoreAfter",0);
	Opt.BracketPrior=settings.Get(0,L"BracketPrior",1);
	Opt.JumpToPair=settings.Get(0,L"JumpToPair",1);
	Opt.Beep=settings.Get(0,L"Beep",0);
	settings.Get(0,L"QuotesType",Opt.QuotesType,ARRAYSIZE(Opt.QuotesType),L"''\"\"`'``„”");
	settings.Get(0,L"Brackets1",Opt.Brackets1,ARRAYSIZE(Opt.Brackets1),L"<>{}[]()\"\"''%%«»");
	settings.Get(0,L"Brackets2",Opt.Brackets2,ARRAYSIZE(Opt.Brackets2),L"/**/<\?\?><%%>");
}

void WINAPI GetPluginInfoW(PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MTitle);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
	Info->PluginConfig.Guids=&MenuGuid;
	Info->PluginConfig.Strings=PluginMenuStrings;
	Info->PluginConfig.Count=ARRAYSIZE(PluginMenuStrings);
}

static int Config()
{
	PluginDialogBuilder Builder(PsInfo, MainGuid, DialogGuid, MTitle, L"Config");
	Builder.StartSingleBox(MRules, true);
	Builder.AddCheckbox(MIgnoreQuotation, &Opt.IgnoreQuotes);
	Builder.AddCheckbox(MIgnoreAfter, &Opt.IgnoreAfter);
	Builder.AddCheckbox(MPriority, &Opt.BracketPrior);
	Builder.AddCheckbox(MJumpToPair, &Opt.JumpToPair);
	Builder.AddCheckbox(MBeep, &Opt.Beep);
	Builder.EndSingleBox();
	Builder.StartSingleBox(MDescriptions, true);
	Builder.AddText(MTypeQuotes);
	Builder.AddFixEditField(Opt.QuotesType, ARRAYSIZE(Opt.QuotesType), ARRAYSIZE(Opt.QuotesType)-1);
	Builder.AddText(MDescript1);
	Builder.AddFixEditField(Opt.Brackets1, ARRAYSIZE(Opt.Brackets1), ARRAYSIZE(Opt.Brackets1)-1);
	Builder.AddText(MDescript2);
	Builder.AddFixEditField(Opt.Brackets2, ARRAYSIZE(Opt.Brackets2), ARRAYSIZE(Opt.Brackets2)-1);
	Builder.EndSingleBox();
	Builder.AddOKCancel(MSave, MCancel);

	if(Builder.ShowDialog())
	{
		PluginSettings settings(MainGuid, PsInfo.SettingsControl);
		settings.Set(0,L"IgnoreQuotes",Opt.IgnoreQuotes);
		settings.Set(0,L"IgnoreAfter",Opt.IgnoreAfter);
		settings.Set(0,L"BracketPrior",Opt.BracketPrior);
		settings.Set(0,L"JumpToPair",Opt.JumpToPair);
		settings.Set(0,L"Beep",Opt.Beep);
		settings.Set(0,L"QuotesType",Opt.QuotesType);
		settings.Set(0,L"Brackets1",Opt.Brackets1);
		settings.Set(0,L"Brackets2",Opt.Brackets2);
		return TRUE;
	}

	return FALSE;
}

intptr_t WINAPI ConfigureW(const ConfigureInfo* Info)
{
	return Config();
}


static constexpr int MenuType_FindSelect = 0, MenuType_Direction = 1;
static constexpr int Direct_UNKNOWN  = -1, Direct_FORWARD = 0, Direct_BACKWARD = 1;
static constexpr int FindSel_UNKNOWN = -1, FindSel_FIND   = 0, FindSel_SELECT  = 1;
//
static int ShowMenu(int Type)
{
	FarMenuItem shMenu[4]= {};
	static const wchar_t *HelpTopic[2]= {L"Find", L"Direct"};
	shMenu[0].Text = GetMsg((Type == MenuType_Direction ? MBForward  : MBrackMath  ));
	shMenu[1].Text = GetMsg((Type == MenuType_Direction ? MBBackward : MBrackSelect));
	shMenu[2].Flags = MIF_SEPARATOR;
	shMenu[3].Text = GetMsg(MConfigure);
	int Ret;

	while(1)
	{
		Ret=(int)PsInfo.Menu(&MainGuid, {},-1,-1,0,FMENU_WRAPMODE,GetMsg(MTitle),{},HelpTopic[Type&1],{},{},shMenu,ARRAYSIZE(shMenu));

		if(Ret == 3)
			Config();
		else
			break;
	}

	return Ret;
}

HANDLE WINAPI OpenW(const OpenInfo *Info)
{
	EditorGetString egs={sizeof(EditorGetString)};
	EditorSetPosition esp={sizeof(EditorSetPosition)},espo={sizeof(EditorSetPosition)};
	EditorSelect es={sizeof(EditorSelect)};

	wchar_t Bracket,Bracket1,Bracket_1;
	wchar_t Ch,Ch1,Ch_1;
	wchar_t B21=0,B22=0,B23=0,B24=0;

	int nQuotes=0;
	int find_sel = FindSel_UNKNOWN;
	intptr_t CurPos;
	int i=3,j,k;
	int Direction = 0; // 0: unspecified, +1: forward, -1: backward
	int DirectQuotes = Direct_UNKNOWN; // Menu() order
	int types=BrZERO;
	BOOL found=FALSE;
	int MatchCount=1;

	int idxBrackets1=0;
	int lenBrackets1=0;
	int idxBrackets2=0;
	int lenBrackets2=0;

	EditorInfo ei={sizeof(EditorInfo)};
	PsInfo.EditorControl(-1,ECTL_GETINFO,0,&ei);

	espo.CurTabPos=ei.CurTabPos;
	espo.TopScreenLine=ei.TopScreenLine;
	espo.LeftPos=ei.LeftPos;
	espo.Overtype=ei.Overtype;
	espo.CurLine=ei.CurLine;
	espo.CurPos=CurPos=ei.CurPos;
	egs.StringNumber=-1;
	PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);

	if (Info->OpenFrom==OPEN_FROMMACRO)
	{
		OpenMacroInfo* mi=(OpenMacroInfo*)Info->Data;
		if (mi->Count)
		{
			int value=-1;
			if (FMVT_INTEGER==mi->Values[0].Type)
				value=(int)mi->Values[0].Integer;
			else if (FMVT_DOUBLE==mi->Values[0].Type)
				value=(int)mi->Values[0].Double;
			else if (FMVT_STRING==mi->Values[0].Type)
			{
				static struct {
					const wchar_t *Name;
					int Value;
				} CmdName[]={
					{L"SEARCHFWD",     0},
					{L"SEARCHBACK",    1},
					{L"SELECTFWD",     2},
					{L"SELECTBACK",    3},
					{L"CONFIG",        4},

					{nullptr,          0},
				};

				for (int I=0; CmdName[I].Name; ++I)
					if (!lstrcmpi(CmdName[I].Name,mi->Values[0].String))
					{
						value=CmdName[I].Value;
						break;
					}
			}

			switch (value)
			{
				case 0: // search fwd
					find_sel = FindSel_FIND;
					DirectQuotes = Direct_FORWARD;
					break;
				case 1: // search back
					find_sel = FindSel_FIND;
					DirectQuotes = Direct_BACKWARD;
					break;
				case 2: // select fwd
					find_sel = FindSel_SELECT;
					DirectQuotes = Direct_FORWARD;
					break;
				case 3: // select back
					find_sel = FindSel_SELECT;
					DirectQuotes = Direct_BACKWARD;
					break;
				case 4:
					Config();
					return nullptr;
				default:
					return nullptr;
			}
		}
	}

	if(find_sel == FindSel_UNKNOWN)
		if((find_sel = ShowMenu(MenuType_FindSelect)) < 0)
			return nullptr;

	if(CurPos > egs.StringLength)
		return nullptr;

	egs.StringNumber=espo.CurLine;
	Bracket_1=(CurPos-1 >= 0?egs.StringText[CurPos-1]:L'\0');
	Bracket1=(CurPos+1 < egs.StringLength?egs.StringText[CurPos+1]:L'\0');

	if(!Opt.QuotesType[0])
	{
		Opt.IgnoreQuotes=1;
	}
	else
	{
		// размер Opt.QuotesType должен быть кратный двум (иначе усекаем)
		i=lstrlen(Opt.QuotesType);

		if((i&1) == 1)
		{
			if(--i > 0)
				Opt.QuotesType[i]=0;
			else
				Opt.IgnoreQuotes=1;
		}

		nQuotes=i;
	}

	Opt.BracketPrior&=1;
	Opt.IgnoreAfter&=1;

	if(Opt.IgnoreQuotes == 0)
	{
		for(i=0; i < nQuotes; i+=2)
			if(Bracket_1 == Opt.QuotesType[i] && Bracket1 == Opt.QuotesType[i+1])
				return nullptr;
	}

	Bracket=(CurPos == egs.StringLength)?L'\0':egs.StringText[CurPos];

	// размер Opt.Brackets1 должен быть кратный двум (иначе усекаем)
	if(((lenBrackets1=lstrlen(Opt.Brackets1)) & 1) != 0)
	{
		lenBrackets1-=(lenBrackets1&1);
		Opt.Brackets1[lenBrackets1]=0;
	}

	lenBrackets1>>=1;

	// размер Opt.Brackets1 должен быть кратный четырем (иначе усекаем)
	if(((lenBrackets2=lstrlen(Opt.Brackets2)) & 3) != 0)
	{
		lenBrackets2-=(lenBrackets2&3);
		Opt.Brackets2[lenBrackets2]=0;
	}

	lenBrackets2>>=2;
	// анализ того, что под курсором
	i=3;
	short BracketPrior=Opt.BracketPrior;

	while(--i)
	{
		if(BracketPrior == 1)
		{
			BracketPrior--;

			if(types == BrZERO && lenBrackets2)
			{
				for(idxBrackets2=0; lenBrackets2 > 0; --lenBrackets2,idxBrackets2+=4)
				{
					B21=Opt.Brackets2[idxBrackets2+0];
					B22=Opt.Brackets2[idxBrackets2+1];
					B23=Opt.Brackets2[idxBrackets2+2];
					B24=Opt.Brackets2[idxBrackets2+3];

					if(Bracket == B21 || Bracket == B22 || Bracket == B23 || Bracket == B24)
					{
						if((Bracket==B21 && Bracket1==B22) || (Bracket==B22 && Bracket_1==B21))
						{
							types=BrTwo;
							Direction = +1;
						}
						else if((Bracket_1==B23 && Bracket==B24) || (Bracket==B23 && Bracket1==B24))
						{
							types=BrTwo;
							Direction = -1;
						}

						break;
					}
				}
			}
		}
		else
		{
			BracketPrior++;

			if(types == BrZERO && lenBrackets1)
			{
				int LB=lenBrackets1;

				for(idxBrackets1=0; lenBrackets1 > 0; --lenBrackets1,idxBrackets1+=2)
				{
					B21=Opt.Brackets1[idxBrackets1+0];
					B22=Opt.Brackets1[idxBrackets1+1];

					if(Bracket==B21)
					{
						types=BrOne;
						Direction = +1;
						break;
					}
					else if(Bracket==B22)
					{
						types=BrOne;
						Direction = -1;
						break;
					}
				}

				if(types == BrZERO && !Opt.IgnoreAfter)
				{
					for(idxBrackets1=0; LB > 0; --LB,idxBrackets1+=2)
					{
						B21=Opt.Brackets1[idxBrackets1+0];
						B22=Opt.Brackets1[idxBrackets1+1];

						if(Bracket_1==B21)
						{
							types=BrRight;
							Direction = +1;
							break;
						}
						else if(Bracket_1==B22)
						{
							types=BrRight;
							Direction = -1;
							break;
						}
					}
				}
			}
		}
	}

	if(Opt.IgnoreAfter && types == BrRight)
		return nullptr;

	if(types == BrZERO)
		return nullptr;

	if(B21 == B22)
	{
		if (DirectQuotes == Direct_UNKNOWN)
			if((DirectQuotes = ShowMenu(MenuType_Direction)) < 0)
				return nullptr;

		Direction = DirectQuotes != Direct_BACKWARD ? +1 : -1;
		types = BrOneMath;
	}

	esp.CurPos=esp.CurTabPos=esp.TopScreenLine=esp.LeftPos=esp.Overtype=-1;
	esp.CurLine=egs.StringNumber;
	egs.StringNumber=-1;

	// поиск пары
	while(!found)
	{
		CurPos+=Direction;
		bool cond_gt=CurPos >= egs.StringLength?true:false;

		if(cond_gt || CurPos < 0)
		{
			if(cond_gt)
				esp.CurLine++;
			else
				esp.CurLine--;

			if(esp.CurLine >= ei.TotalLines || esp.CurLine < 0)
				break;

			PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
			PsInfo.EditorControl(-1,ECTL_GETSTRING,0,&egs);

			if(cond_gt)
				CurPos=0;
			else
				CurPos=egs.StringLength-1;
		}

		if(CurPos > egs.StringLength || CurPos < 0)
			continue;

		Ch_1=(CurPos-1 >= 0?egs.StringText[CurPos-1]:L'\0');
		Ch=((CurPos == egs.StringLength)?L'\0':egs.StringText[CurPos]);
		Ch1=(CurPos+1 < egs.StringLength?egs.StringText[CurPos+1]:L'\0');

		// BUGBUGBUG!!!
		if(Opt.IgnoreQuotes == 1)
		{
			for(k=j=0; j < nQuotes; j+=2)
			{
				if(Ch_1 == Opt.QuotesType[j] && Ch1 == Opt.QuotesType[j+1])
				{
					k++;
					break;
				}
			}

			if(k)
				continue;
		}

		switch(types)
		{
				/***************************************************************/
			case BrOneMath:
			{
				if(Ch == Bracket)
				{
					if((Bracket_1==L'\\' && Ch_1==L'\\') || (Bracket_1!=L'\\' && Ch_1!=L'\\'))
						found=TRUE;
				}

				break;
			}
			/***************************************************************/
			case BrRight:
			{
				if(Ch == Bracket_1)
				{
					MatchCount++;
				}
				else if((Ch==B21 && Bracket_1==B22) || (Ch==B22 && Bracket_1==B21))
				{
					--MatchCount;

					if((Direction == +1 && MatchCount == 0) || (Direction == -1 && MatchCount == 1))
						found=TRUE;
				}

				break;
			}
			/***************************************************************/
			case BrOne:
			{
				if(Ch == Bracket)
				{
					MatchCount++;
				}
				else if((Ch==B21 && Bracket==B22) || (Ch==B22 && Bracket==B21))
				{
					if(--MatchCount==0)
						found=TRUE;
				}

				break;
			}
			/***************************************************************/
			case BrTwo:
			{
				if((Direction == +1 &&
				        ((Bracket==B21 && Ch==B21 && Ch1  == B22) ||
				         (Bracket==B22 && Ch==B22 && Ch_1 == B21))
				   ) ||
				        (Direction == -1 &&
				         ((Bracket==B23 && Ch==B23 && Ch1  == B24) ||
				          (Bracket==B24 && Ch==B24 && Ch_1 == B23))
				        )
				  )
				{
					MatchCount++;
				}
				else if(
				    (Bracket==B21 && Ch==B23 && Bracket1 ==B22 && Ch1 ==B24) ||
				    (Bracket==B22 && Ch==B24 && Bracket_1==B21 && Ch_1==B23) ||
				    (Bracket==B23 && Ch==B21 && Bracket1 ==B24 && Ch1 ==B22) ||
				    (Bracket==B24 && Ch==B22 && Bracket_1==B23 && Ch_1==B21)
				)
				{
					if(--MatchCount==0)
						found=TRUE;
				}

				break;
			}
		}
	}

	if(found)
	{
		egs.StringNumber=esp.CurLine;

		if(types == BrTwo)
		{
			if(Bracket == B21 || Bracket == B24)
				CurPos+=Direction;
			else if(Bracket == B22 || Bracket == B23)
				CurPos-=Direction;
		}

		esp.CurPos=CurPos;
		esp.CurTabPos=esp.LeftPos=esp.Overtype=-1;

		if(egs.StringNumber<ei.TopScreenLine || egs.StringNumber>=ei.TopScreenLine+ei.WindowSizeY)
		{
			esp.TopScreenLine=esp.CurLine-ei.WindowSizeY/2;

			if(esp.TopScreenLine < 0)
				esp.TopScreenLine=0;
		}
		else
		{
			esp.TopScreenLine=-1;
		}

		if(find_sel != FindSel_SELECT || Opt.JumpToPair)
			PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&esp);

		if(Opt.Beep)
			MessageBeep(0);

		if(find_sel == FindSel_SELECT)
		{
			es.BlockType=BTYPE_STREAM;
			es.BlockStartLine = std::min(esp.CurLine,espo.CurLine);
			es.BlockStartPos=(Direction > 0?espo.CurPos:esp.CurPos);
			es.BlockHeight = std::max(esp.CurLine,espo.CurLine)- std::min(esp.CurLine,espo.CurLine)+1;

			if(Direction > 0)
				es.BlockWidth=esp.CurPos-espo.CurPos+1;
			else
				es.BlockWidth=espo.CurPos-esp.CurPos+1;

			if(types == BrRight)
			{
				if(Direction > 0)
				{
					es.BlockStartPos--;
					es.BlockWidth++;
				}
				else if(Direction < 0)
				{
					es.BlockWidth--;
				}
			}

			PsInfo.EditorControl(-1,ECTL_SELECT,0,&es);
		}
	}
	else
	{
		PsInfo.EditorControl(-1,ECTL_SETPOSITION,0,&espo);
	}

	return (Info->OpenFrom==OPEN_FROMMACRO)?INVALID_HANDLE_VALUE:nullptr;
}
