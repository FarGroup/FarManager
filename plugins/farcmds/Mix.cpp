#ifndef __mix_cpp
#define __mix_cpp

/*
 возвращает число, вырезав его из строки, или -2 в случае ошибки
 Start, End - начало и конец строки
*/
int GetInt(TCHAR *Start, TCHAR *End)
{
  int Ret=-2;
  if(End >= Start)
  {
    TCHAR Tmp[11];
    int Size=(int)(End-Start);

    if(Size)
    {
      if(Size < 11)
      {
        _tmemcpy(Tmp,Start,Size);
        Tmp[Size]=0;
        Ret=FarAtoi(Tmp);
      }
    }
    else
      Ret=0;
  }
  return Ret;
}

const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

/*Возвращает TRUE, если файл name существует*/
BOOL FileExists(const TCHAR *Name)
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

TCHAR *GetCommaWord(TCHAR *Src,TCHAR *Word,TCHAR Separator)
{
  int WordPos,SkipBrackets;
  if (*Src==0)
    return(NULL);
  SkipBrackets=FALSE;
  for (WordPos=0;*Src!=0;Src++,WordPos++)
  {
    if (*Src==_T('[') && _tcschr(Src+1,_T(']'))!=NULL)
      SkipBrackets=TRUE;
    if (*Src==_T(']'))
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
