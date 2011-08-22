const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
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

const wchar_t *GetOnlyName(const wchar_t *FullName)
{
	const wchar_t *Name=wcsrchr(FullName,L'\\');

	if (Name) ++Name;
	else Name=FullName;

	return Name;
}

wchar_t *GetFullName(wchar_t *Dest,const wchar_t *Dir,const wchar_t *Name)
{
	lstrcpy(Dest,Dir);
	int len=lstrlen(Dest);

	if (len)
	{
		if (Dest[len-1]==L'\\') --len;
		else Dest[len]=L'\\';

		lstrcpy(Dest+len+1,GetOnlyName(Name));
	}
	else lstrcpy(Dest, Name);

	return Dest;
}

BOOL IsWordDiv(int c)
{
	return (wmemchr(Opt.WordDiv, c, Opt.WordDivLen)!=NULL);
}

//  CaseWord - convert case of string by given type
void CaseWord(wchar_t *nm, int Type)
{
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
