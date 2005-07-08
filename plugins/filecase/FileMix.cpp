const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

void InitDialogItems(const struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
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
    PItem->Param.Selected=PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    lstrcpy(PItem->Data.Data,((unsigned int)PInit->Data<2000)?
           GetMsg((unsigned int)PInit->Data):PInit->Data);
  }
}

int IsCaseMixed(const char *Str)
{
  while (*Str && !FSF.LIsAlpha((BYTE)*Str))
    Str++;
  int Case=FSF.LIsLower((BYTE)*Str);
  while (*(Str++))
    if (FSF.LIsAlpha((BYTE)*Str) && FSF.LIsLower((BYTE)*Str)!=Case)
      return(TRUE);
  return(FALSE);
}

char *GetOnlyName(char *FullName)
{
  char *Name=strrchr(FullName,'\\');
  if(Name) ++Name;
  else Name=FullName;
  return Name;
}

char *GetFullName(char *Dest,const char *Dir,char *Name)
{
  lstrcpy(Dest,Dir);
  int len=lstrlen(Dest);
  if(len)
  {
    if(Dest[len-1]=='\\') --len;
    else Dest[len]='\\';
    lstrcpy(Dest+len+1,GetOnlyName(Name));
  }
  else lstrcpy(Dest, Name);

  return Dest;
}

BOOL IsWordDiv(int c)
{
  return (memchr(Opt.WordDiv, c, Opt.WordDivLen)!=NULL);
}

//  CaseWord - convert case of string by given type
void CaseWord( char *nm, int Type )
{
  int I;

  switch( Type )
  {
    case MODE_N_WORD:
      *nm = (char)FSF.LUpper((BYTE)*nm);
      FSF.LStrlwr(nm+1);
      break;
    case MODE_LN_WORD:
      for (I=0; nm[I]; I++)
        if(!I || IsWordDiv(nm[I-1]))
           nm[I]=(char)FSF.LUpper((BYTE)nm[I]);
    else
      nm[I]=(char)FSF.LLower((BYTE)nm[I]);
      break;
    case MODE_LOWER:
      FSF.LStrlwr(nm);
      break;
    case MODE_UPPER:
      FSF.LStrupr(nm);
      break;
  }
}
