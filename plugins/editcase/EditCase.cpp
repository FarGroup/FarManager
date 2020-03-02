// EditCase is Far Manager plugin. It allows to change the case of selected text
// or current (nearest) word in the internal editor.
// This plugin can change case to: lower case, Title Case, UPPER CASE and tOGGLE cASE
// Besides, it has ability of cyclic case change like MS Word by ShiftF3
#include <algorithm>
#include <cwchar>
#include <plugin.hpp>
#include <PluginSettings.hpp>
#include "EditLng.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"

#if defined(__GNUC__) || defined (_MSC_VER)
#include <limits.h>
#undef MAXINT
#define MAXINT INT_MAX
#else
#include <values.h> //MAXINT
#endif

static struct PluginStartupInfo Info;
static FARSTANDARDFUNCTIONS FSF;

// Menu item numbers
enum ENUMCCTYPES
{
	CCLower,
	CCTitle,
	CCUpper,
	CCToggle,
	CCCyclic,
};

// This chars aren't letters
static wchar_t* WordDiv;
static int WordDivLen;

const wchar_t *GetMsg(int MsgId);
bool FindBounds(wchar_t *Str, intptr_t Len, intptr_t Pos, intptr_t &Start, intptr_t &End);
intptr_t FindEnd(wchar_t *Str, intptr_t Len, intptr_t Pos);
intptr_t FindStart(wchar_t *Str, intptr_t Len, intptr_t Pos);
bool MyIsAlpha(int c);
int GetNextCCType(wchar_t *Str, intptr_t StrLen, intptr_t Start, intptr_t End);
int ChangeCase(wchar_t *NewString, intptr_t Start, intptr_t End, int CCType);

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

void WINAPI SetStartupInfoW(const struct PluginStartupInfo *Info)
{
	::Info=*Info;
	::FSF=*Info->FSF;
	::Info.FSF=&::FSF;
	PluginSettings plugSettings(MainGuid, ::Info.SettingsControl);
	const wchar_t* AddWordDiv=plugSettings.Get(0,L"AddWordDiv",L"#");

	FarSettingsCreate settings={sizeof(FarSettingsCreate),FarGuid,INVALID_HANDLE_VALUE};
	HANDLE Settings=::Info.SettingsControl(INVALID_HANDLE_VALUE,SCTL_CREATE,0,&settings)?settings.Handle:0;
	if(Settings)
	{
		FarSettingsItem item={sizeof(FarSettingsItem),FSSF_EDITOR,L"WordDiv",FST_UNKNOWN,{0}};
		if(::Info.SettingsControl(Settings,SCTL_GET,0,&item)&&FST_STRING==item.Type)
		{
			WordDivLen=lstrlen(item.String)+lstrlen(AddWordDiv)+ARRAYSIZE(L" \n\r\t");
			WordDiv=(wchar_t*)malloc((WordDivLen+1)*sizeof(wchar_t));
			if(WordDiv)
			{
				lstrcpy(WordDiv,item.String);
				lstrcat(WordDiv, AddWordDiv);
				lstrcat(WordDiv, L" \n\r\t");
			}
		}
		::Info.SettingsControl(Settings,SCTL_FREE,0,0);
	}
}

HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	int MenuCode=-1;

	if (OInfo->OpenFrom==OPEN_FROMMACRO)
	{
		OpenMacroInfo* mi=(OpenMacroInfo*)OInfo->Data;
		if (mi->Count)
 		{
 			if (FMVT_INTEGER == mi->Values[0].Type)
 			{
				MenuCode=(int)mi->Values[0].Integer;
			}
			else if (FMVT_DOUBLE == mi->Values[0].Type)
			{
				MenuCode=(int)mi->Values[0].Double;
			}
			else if (FMVT_STRING == mi->Values[0].Type)
			{
				static struct {
					const wchar_t *Name;
					int Value;
				} CmdName[]={
					{L"LOWER",         0},
					{L"TITLE",         1},
					{L"UPPER",         2},
					{L"TOGGLE",        3},
					{L"CYCLIC",        4},

					{nullptr,          0},
				};

				for (int I=0; CmdName[I].Name; ++I)
					if (!lstrcmpi(CmdName[I].Name,mi->Values[0].String))
					{
						MenuCode=CmdName[I].Value;
						break;
					}
			}
			if (MenuCode < 0 || MenuCode > 4)
				return nullptr;
		}
	}

	if (MenuCode == -1)
	{
		size_t i;
		struct FarMenuItem MenuItems[5] = {}, *MenuItem;
		int Msgs[]={MCaseLower, MCaseTitle, MCaseUpper, MCaseToggle, MCaseCyclic};

		for (MenuItem=MenuItems,i=0; i < ARRAYSIZE(MenuItems); ++i, ++MenuItem)
		{
			MenuItem->Text = GetMsg(Msgs[i]); // Text in menu
		}

		// First item is selected
		MenuItems[0].Flags=MIF_SELECTED;
		// Show menu
		MenuCode=(int)Info.Menu(&MainGuid, nullptr,-1,-1,0,FMENU_AUTOHIGHLIGHT|FMENU_WRAPMODE,
		                       GetMsg(MCaseConversion),NULL,L"Contents",NULL,NULL,
		                       MenuItems,ARRAYSIZE(MenuItems));
	}

	switch (MenuCode)
	{
			// If menu Escaped
		case -1:
			break;
		default:
			EditorInfo ei={sizeof(EditorInfo)};
			Info.EditorControl(-1,ECTL_GETINFO,0,&ei);
			// Current line number
			intptr_t CurLine=ei.CurLine;
			// Is anything selected
			bool IsBlock=false;

			// Nothing selected?
			if (ei.BlockType!=BTYPE_NONE)
			{
				IsBlock=true;
				CurLine=ei.BlockStartLine;
			}

			// Type of Case Change
			int CCType=MenuCode;
			// Temporary string
			wchar_t *NewString=0;

			// Forever :-) (Line processing loop)
			for (;;)
			{
				if (IsBlock)
				{
					if (CurLine >= ei.TotalLines)
						break;

					struct EditorSetPosition esp = {sizeof(EditorSetPosition),CurLine++,-1,-1,-1,-1,-1};
					Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
				}

				struct EditorGetString egs={sizeof(EditorGetString)};

				egs.StringNumber=-1;

				// If can't get line
				if (!Info.EditorControl(-1,ECTL_GETSTRING,0,&egs))
					break; // Exit

				// If last selected line was processed or
				// nothing selected and line is empty
				if ((IsBlock && egs.SelStart==-1) || (!IsBlock && egs.StringLength<=0))
					break; // Exit

				// If something selected, but line is empty
				if (egs.StringLength<=0)
					continue; // Get next line

				// If whole line (with EOL) is selected
				if (egs.SelEnd==-1 || egs.SelEnd>egs.StringLength)
				{
					egs.SelEnd=egs.StringLength;

					if (egs.SelEnd<egs.SelStart)
						egs.SelEnd=egs.SelStart;
				}

				// Memory allocation
				NewString=(wchar_t *)malloc((egs.StringLength+1)*sizeof(wchar_t));

				// If memory couldn't be allocated
				if (!NewString)
					break;

				// If nothing selected - finding word bounds (what'll be converted)
				if (!IsBlock)
				{
					// Making NewString
					wmemcpy(NewString,egs.StringText,egs.StringLength);
					NewString[egs.StringLength]=0;
					// Like whole line is selected
					egs.SelStart=0;
					egs.SelEnd=egs.StringLength;
					// Finding word bounds (what'll be converted)
					FindBounds(NewString, egs.StringLength, ei.CurPos, egs.SelStart, egs.SelEnd);
				}

				// Making NewString
				wmemcpy(NewString,egs.StringText,egs.StringLength);
				NewString[egs.StringLength]=0;

				// If Conversion Type is unknown or Cyclic
				if (CCType==CCCyclic) // Define Conversion Type
					CCType=GetNextCCType(NewString, egs.StringLength, egs.SelStart, egs.SelEnd);

				// NewString contains no words
				if (CCType!=CCCyclic)
				{
					// Do the conversion
					ChangeCase(NewString, egs.SelStart, egs.SelEnd, CCType);
					// Put converted string to editor
					struct EditorSetString ess={sizeof(EditorSetString)};
					ess.StringNumber=-1;
					ess.StringText=NewString;
					ess.StringEOL=(wchar_t*)egs.StringEOL;
					ess.StringLength=egs.StringLength;
					Info.EditorControl(-1,ECTL_SETSTRING,0,&ess);
				}

#if 0

				if (!IsBlock)
				{
					struct EditorSelect esel={EditorSelect};
					esel.BlockType=BTYPE_STREAM;
					esel.BlockStartLine=-1;
					esel.BlockStartPos=egs.SelStart;
					esel.BlockWidth=egs.SelEnd-egs.SelStart;
					esel.BlockHeight=1;
					Info.EditorControl(-1,ECTL_SELECT,0,&esel);
				}

#endif
				// Free memory
				free(NewString);

				// Exit if nothing was selected (single word was converted)
				if (!IsBlock)
					break;
			}

			if (IsBlock)
			{
				struct EditorSetPosition esp = {sizeof(EditorSetPosition),ei.CurLine,ei.CurPos,-1,ei.TopScreenLine,ei.LeftPos,ei.Overtype};
				Info.EditorControl(-1,ECTL_SETPOSITION,0,&esp);
			}
	} // switch

	return nullptr;
}

void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MCaseConversion);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

void WINAPI ExitFARW(const struct ExitInfo *Info)
{
	free(WordDiv);
}

const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

// What we consider as letter
bool MyIsAlpha(int c)
{
	return (WordDiv&&wmemchr(WordDiv, c, WordDivLen)==NULL) ? true : false;
}

// Finding word bounds (what'll be converted) (Str is in OEM)
bool FindBounds(wchar_t *Str, intptr_t Len, intptr_t Pos, intptr_t &Start, intptr_t &End)
{
	int i=1;
	bool ret = false;

	// If line isn't empty
	if (Len>Start)
	{
		End = std::min(End,Len);
		// Pos between [Start, End] ?
		Pos = std::max(Pos,Start);
		Pos = std::min(End,Pos);

		// If current character is non letter
		if (!MyIsAlpha(Str[Pos]))
		{
			// Looking for letter on the left and counting radius
			while ((Start<=Pos-i) && (!MyIsAlpha(Str[Pos-i])))
				i++;

			// Radius
			int r=MAXINT;

			// Letter was found on the left
			if (Start<=Pos-i)
				r=i; // Storing radius

			i=1;

			// Looking for letter on the right and counting radius
			while ((Pos+i<=End) && (!MyIsAlpha(Str[Pos+i])))
				i++;

			// Letter was not found
			if (Pos+i>End)
				i=MAXINT;

			// Here r is left radius and i is right radius

			// If no letters was found
			if (std::min(r,i) != MAXINT)
			{
				// What radius is less? Left?
				if (r <= i)
				{
					End=Pos-r+1;
					Start=FindStart(Str, Start, End);
				}
				else // Right!
				{
					Start=Pos+i;
					End=FindEnd(Str, Start, End);
				}

				ret=true;
			}
		}
		else // Current character is letter!
		{
			Start=FindStart(Str, Start, Pos);
			End=FindEnd(Str, Pos, End);
			ret=true;
		}
	}

	if (!ret)
		Start=End=-1;

	return ret;
}

intptr_t FindStart(wchar_t *Str, intptr_t Start, intptr_t End)
{
	// Current pos in Str
	intptr_t CurPos=End-1;

	// While current character is letter
	while (CurPos>=Start && MyIsAlpha(Str[CurPos]))
		CurPos--; // Moving to left

	return CurPos+1;
}

intptr_t FindEnd(wchar_t *Str, intptr_t Start, intptr_t End)
{
	// Current pos in Str
	intptr_t CurPos=Start;

	// While current character is letter
	while (CurPos<End && MyIsAlpha(Str[CurPos]))
		CurPos++; // Moving to right

	return CurPos;
}

// Changes Case of NewString from position Start till End
// to CCType and returns amount of changes
int ChangeCase(wchar_t *NewString, intptr_t Start, intptr_t End, int CCType)
{
	// If previous symbol is letter, then IsPrevSymbAlpha!=0
	bool IsPrevSymbAlpha=false;
	// Amount of changes
	int ChangeCount=0;

	// Main loop (position inside line)
	for (intptr_t i=Start; i<End; i++)
	{
		if (MyIsAlpha(NewString[i]))// && ReverseOem==NewString[i])
		{
			switch (CCType)
			{
				case CCLower:
					NewString[i]=(wchar_t)FSF.LLower(NewString[i]);
					break;
				case CCTitle:

					if (IsPrevSymbAlpha)
						NewString[i]=(wchar_t)FSF.LLower(NewString[i]);
					else
						NewString[i]=(wchar_t)FSF.LUpper(NewString[i]);

					break;
				case CCUpper:
					NewString[i]=(wchar_t)FSF.LUpper(NewString[i]);
					break;
				case CCToggle:

					if (FSF.LIsLower(NewString[i]))
						NewString[i]=(wchar_t)FSF.LUpper(NewString[i]);
					else
						NewString[i]=(wchar_t)FSF.LLower(NewString[i]);

					break;
			}

			// Put converted letter back to string
			IsPrevSymbAlpha=true;
			ChangeCount++;
		}
		else
		{
			IsPrevSymbAlpha=false;
		}
	}

	return ChangeCount;
}

// Return CCType by rule: lower->Title->UPPER
// If Str contains no letters, then return CCCyclic
int GetNextCCType(wchar_t *Str, intptr_t StrLen, intptr_t Start, intptr_t End)
{
	intptr_t SignalWordStart=Start,
	                    SignalWordEnd=End;
	intptr_t SignalWordLen = std::max(Start,End);
	// Default conversion is to lower case
	int CCType=CCLower;
	Start = std::min(Start,End);
	End=SignalWordLen;

	if (StrLen<Start)
		return CCCyclic;

	// Looking for SignalWord (the 1-st word)
	if (!FindBounds(Str, StrLen, SignalWordStart, SignalWordStart, SignalWordEnd))
		return CCCyclic;

	SignalWordLen=SignalWordEnd-SignalWordStart;
	wchar_t *SignalWord=(wchar_t *)malloc((SignalWordLen+1)*sizeof(wchar_t));

	if (SignalWord != NULL)
	{
		wchar_t *WrappedWord=(wchar_t *)malloc((SignalWordLen+1)*sizeof(wchar_t));

		if (WrappedWord != NULL)
		{
			lstrcpyn(SignalWord, &Str[SignalWordStart], (int)(SignalWordLen+1) /* BUGBUG because of intptr_t */);
			lstrcpy(WrappedWord, SignalWord);
			// if UPPER then Title
			FSF.LUpperBuf(WrappedWord, SignalWordLen);

			if (SignalWordLen == 1 && lstrcmp(SignalWord, WrappedWord)==0)
			{
				CCType=CCLower;
			}
			else
			{
				if (SignalWordLen == 1)
				{
					CCType=CCUpper;
				}
				else
				{
					if (lstrcmp(SignalWord, WrappedWord)==0)
					{
						CCType=CCTitle;
					}
					else
					{
						// if lower then UPPER
						FSF.LLowerBuf(WrappedWord, SignalWordLen);

						if (lstrcmp(SignalWord,WrappedWord)==0)
						{
							CCType=CCUpper;
						}
						else
						{
							// if Title then lower
							WrappedWord[0]=FSF.LUpper(WrappedWord[0]);

							if (lstrcmp(SignalWord,WrappedWord)==0)
							{
								CCType=CCLower;
							}
							else
							{
								// if upper case letters amount more than lower case letters
								// then tOGGLE
								FSF.LUpperBuf(WrappedWord, SignalWordLen);
								intptr_t Counter=SignalWordLen/2+1;

								for (int i=0; i<SignalWordLen && Counter; i++)
									if (SignalWord[i]==WrappedWord[i])
										Counter--;

								if (!Counter)
									CCType=CCToggle;
							}
						}
					}
				}
			}

			free(WrappedWord);
		}

		free(SignalWord);
	}

	return CCType;
}
