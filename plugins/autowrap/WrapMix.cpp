const TCHAR *GetMsg(int MsgId)
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
    PItem->History=(const TCHAR *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
#ifdef UNICODE
    PItem->MaxLen=0;
#endif
    if ((DWORD_PTR)PInit->Data<2000)
#ifndef UNICODE
      lstrcpy(PItem->Data,GetMsg((unsigned int)(DWORD_PTR)PInit->Data));
#else
      PItem->PtrData = GetMsg((unsigned int)(DWORD_PTR)PInit->Data);
#endif
    else
#ifndef UNICODE
      lstrcpy(PItem->Data,PInit->Data);
#else
      PItem->PtrData = PInit->Data;
#endif
  }
}

TCHAR *GetCommaWord(const TCHAR *Src,TCHAR *Word)
{
  int WordPos,SkipBrackets;
  if (*Src==_T('\0'))
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=_T('\0');Src++,WordPos++)
  {
    if (*Src==_T('[') && _tcschr(Src+1,_T(']'))!=NULL)
      SkipBrackets=TRUE;
    if (*Src==_T(']'))
      SkipBrackets=FALSE;
    if (*Src==_T(',') && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (_istspace(*Src))
        Src++;
      return((TCHAR*)Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return((TCHAR*)Src);
}
