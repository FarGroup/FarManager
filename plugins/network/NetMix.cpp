#include "NetCommon.hpp"

const wchar_t *GetMsg(int MsgId)
{
  return Info.GetMsg(&MainGuid,MsgId);
}


void InitDialogItems(struct InitDialogItem *Init,struct FarDialogItem *Item, int ItemsNumber)
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
    PItem->History=(const wchar_t *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    PItem->MaxLen=0;
    if ((DWORD_PTR)PInit->Data<2000)
      PItem->PtrData = GetMsg((unsigned int)(DWORD_PTR)PInit->Data);
    else
      PItem->PtrData = PInit->Data;
  }
}

TSaveScreen::TSaveScreen()
{
  hScreen=Info.SaveScreen(0,0,-1,-1);
  const wchar_t *MsgItems[]={GetMsg(MWaitForNetworkBrowse1),GetMsg(MWaitForNetworkBrowse2)};
  Info.Message(Info.ModuleNumber,0,NULL,MsgItems,ARRAYSIZE(MsgItems),0);
}

TSaveScreen::~TSaveScreen()
{
  if (hScreen)
  {
    Info.RestoreScreen(NULL);
    Info.RestoreScreen(hScreen);
  }
}
