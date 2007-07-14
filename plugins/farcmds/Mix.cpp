#ifndef __mix_cpp
#define __mix_cpp

/*
 возвращает число, вырезав его из строки, или -2 в случае ошибки
 Start, End - начало и конец строки
*/
int GetInt(char *Start, char *End)
{
  int Ret=-2;
  if(End >= Start)
  {
    char Tmp[11];
    int Size=(int)(End-Start);

    if(Size)
    {
      if(Size < 11)
      {
        memcpy(Tmp,Start,Size);
        Tmp[Size]=0;
        Ret=FarAtoi(Tmp);
      }
    }
    else
      Ret=0;
  }
  return Ret;
}

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

/*Возвращает TRUE, если файл name существует*/
BOOL FileExists(const char *Name)
{
  return GetFileAttributes(Name)!=0xFFFFFFFF;
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
    PItem->History=(const char *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    if ((unsigned int)(DWORD_PTR)PInit->Data<2000)
      lstrcpy(PItem->Data,GetMsg((unsigned int)(DWORD_PTR)PInit->Data));
    else
      lstrcpy(PItem->Data,PInit->Data);
  }
}

char *GetCommaWord(char *Src,char *Word,char Separator)
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
    if (*Src==Separator && !SkipBrackets)
    {
      Word[WordPos]=0;
      Src++;
      while (IsSpace(*Src))
        Src++;
      return(Src);
    }
    else
      Word[WordPos]=*Src;
  }
  Word[WordPos]=0;
  return(Src);
}

#endif
