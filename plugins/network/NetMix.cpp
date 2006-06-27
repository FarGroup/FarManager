#include "NetCommon.hpp"

char *GetMsg(int MsgId)
{
  return(char *) (Info.GetMsg(Info.ModuleNumber,MsgId));
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
    PItem->Selected=PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
    if ((unsigned int)PInit->Data<2000)
      lstrcpy(PItem->Data,GetMsg((unsigned int)PInit->Data));
    else
      lstrcpy(PItem->Data,PInit->Data);
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
    const char *MsgItems[]={GetMsg(MWaitForNetworkBrowse1),GetMsg(MWaitForNetworkBrowse2)};
    Info.Message(Info.ModuleNumber,0,NULL,MsgItems,
                sizeof(MsgItems)/sizeof(MsgItems[0]),0);
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
