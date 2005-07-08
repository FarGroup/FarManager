#ifndef __mix_cpp
#define __mix_cpp

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

BOOL FileExists(const char* Name)
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
    PItem->Selected=PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    strcpy(PItem->Data,((unsigned int)PInit->Data<2000)?GetMsg((unsigned int)PInit->Data):PInit->Data);
  }
}

BOOL CheckExtension(const char *ptrName)
{
  return Info.CmpName("*.hlf", ptrName, TRUE);
}

void ShowHelp(const char *fullfilename,const char *topic, bool CmdLine)
{
  if(CmdLine || CheckExtension(fullfilename))
  {
    const char *Topic=topic;

    if(NULL == Topic)
       Topic=GetMsg(MDefaultTopic);

    Info.ShowHelp(fullfilename,Topic,FHELP_CUSTOMFILE);
  }
}

#endif
