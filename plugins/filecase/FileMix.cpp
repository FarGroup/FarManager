#include <cwchar>
#include "FileCase.hpp"
#include "guid.hpp"

const wchar_t *GetMsg(int MsgId)
{
	return PsInfo.GetMsg(&MainGuid,MsgId);
}


int IsCaseMixed(const wchar_t *Str)
{
	while (*Str && !FSF.LIsAlpha(*Str))
		Str++;

	int Case=FSF.LIsLower(*Str);

	while (*(Str++))
		if (FSF.LIsAlpha(*Str) && FSF.LIsLower(*Str)!=Case)
			return TRUE;

	return FALSE;
}

static BOOL IsWordDiv(int c)
{
	return (wmemchr(Opt.WordDiv, c, Opt.WordDivLen)!=nullptr);
}

//  CaseWord - convert case of string by given type
void CaseWord(wchar_t *nm, int Type)
{
	if (!nm || !*nm)
		return;

	int I;

	switch (Type)
	{
		case MODE_N_WORD:
			*nm = FSF.LUpper(*nm);
			FSF.LStrlwr(nm+1);
			break;
		case MODE_LN_WORD:

			for (I=0; nm[I]; I++)
			{
				if (!I || IsWordDiv(nm[I-1]))
					nm[I]=(wchar_t)FSF.LUpper(nm[I]);
				else
					nm[I]=(wchar_t)FSF.LLower(nm[I]);
			}

			break;
		case MODE_LOWER:
			FSF.LStrlwr(nm);
			break;
		case MODE_UPPER:
			FSF.LStrupr(nm);
			break;
	}
}
