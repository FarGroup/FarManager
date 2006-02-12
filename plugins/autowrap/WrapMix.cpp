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
    PItem->Selected=PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    lstrcpy(PItem->Data,((unsigned int)PInit->Data<2000)?GetMsg((unsigned int)PInit->Data):PInit->Data);
  }
}

char *GetCommaWord(const char *Src,char *Word)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src=='[' && strchr(Src+1,']')!=NULL)
      SkipBrackets=TRUE;
    if (*Src==']')
      SkipBrackets=FALSE;
    if (*Src==',' && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (isspace(*Src))
        Src++;
      return((char*)Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return((char*)Src);
}
