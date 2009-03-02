const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,int ItemsNumber)
{
  int I;
  struct FarDialogItem *PItem=Item;
  const struct InitDialogItem *PInit=Init;
  for (I=0;I<ItemsNumber;I++,PItem++,PInit++)
  {
    PItem->Type=PInit->Type;
    PItem->X1=PInit->X1;
    PItem->Y1=PInit->Y1;
    PItem->X2=PInit->X2;
    PItem->Y2=PInit->Y2;
    PItem->Focus=PInit->Focus;
    PItem->Param.History=(const TCHAR *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
#ifdef UNICODE
    PItem->MaxLen=0;
#endif
    if ((unsigned int)(DWORD_PTR)PInit->Data<2000)
#ifndef UNICODE
      lstrcpy(PItem->Data.Data,GetMsg((unsigned int)(DWORD_PTR)PInit->Data));
#else
      PItem->PtrData = GetMsg((unsigned int)(DWORD_PTR)PInit->Data);
#endif
    else
#ifndef UNICODE
      lstrcpy(PItem->Data.Data,PInit->Data);
#else
      PItem->PtrData = PInit->Data;
#endif
  }
}

#ifndef UNICODE
typedef unsigned char UTCHAR;
#else
typedef wchar_t UTCHAR;
#endif
int IsCaseMixed(const TCHAR *Str)
{
  while (*Str && !FSF.LIsAlpha((UTCHAR)*Str))
    Str++;
  int Case=FSF.LIsLower((UTCHAR)*Str);
  while (*(Str++))
    if (FSF.LIsAlpha((UTCHAR)*Str) && FSF.LIsLower((UTCHAR)*Str)!=Case)
      return(TRUE);
  return(FALSE);
}

TCHAR *GetOnlyName(TCHAR *FullName)
{
  TCHAR *Name=_tcsrchr(FullName,_T('\\'));
  if(Name) ++Name;
  else Name=FullName;
  return Name;
}

TCHAR *GetFullName(TCHAR *Dest,const TCHAR *Dir,TCHAR *Name)
{
  lstrcpy(Dest,Dir);
  int len=lstrlen(Dest);
  if(len)
  {
    if(Dest[len-1]==_T('\\')) --len;
    else Dest[len]=_T('\\');
    lstrcpy(Dest+len+1,GetOnlyName(Name));
  }
  else lstrcpy(Dest, Name);

  return Dest;
}

BOOL IsWordDiv(int c)
{
  return (_tmemchr(Opt.WordDiv, c, Opt.WordDivLen)!=NULL);
}

//  CaseWord - convert case of string by given type
void CaseWord( TCHAR *nm, int Type )
{
  int I;

  switch( Type )
  {
    case MODE_N_WORD:
      *nm = (TCHAR)FSF.LUpper((UTCHAR)*nm);
      FSF.LStrlwr(nm+1);
      break;
    case MODE_LN_WORD:
      for (I=0; nm[I]; I++)
        if(!I || IsWordDiv(nm[I-1]))
           nm[I]=(TCHAR)FSF.LUpper((UTCHAR)nm[I]);
    else
      nm[I]=(TCHAR)FSF.LLower((UTCHAR)nm[I]);
      break;
    case MODE_LOWER:
      FSF.LStrlwr(nm);
      break;
    case MODE_UPPER:
      FSF.LStrupr(nm);
      break;
  }
}
