#include "NetCommon.hpp"

TCHAR *GetMsg(int MsgId)
{
  return(TCHAR *) (Info.GetMsg(Info.ModuleNumber,MsgId));
}


void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item,
                    int ItemsNumber)
{
  int I;
  struct FarDialogItem *PItem=Item;
  struct InitDialogItem *PInit=Init;
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

//TSaveScreen::TSaveScreen(struct PluginStartupInfo *Info)
TSaveScreen::TSaveScreen()
{
//  TSaveScreen::Info=Info;
  hScreen=NULL;
//  if(Info)
  {
    hScreen=Info.SaveScreen(0,0,-1,-1);
    const TCHAR *MsgItems[]={GetMsg(MWaitForNetworkBrowse1),GetMsg(MWaitForNetworkBrowse2)};
    Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
  }
}

TSaveScreen::~TSaveScreen()
{
  if (hScreen)
  {
//    if(Info)
    {
      Info.RestoreScreen(NULL);
      Info.RestoreScreen(hScreen);
    }
  }
}
