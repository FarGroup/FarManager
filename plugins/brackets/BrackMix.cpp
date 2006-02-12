const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}


void InitDialogItems(const struct InitDialogItem *Init,
                     struct FarDialogItem *Item,
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
    if ((unsigned int)PInit->Data<2000)
      lstrcpy(PItem->Data.Data,GetMsg((unsigned int)PInit->Data));
    else
      lstrcpy(PItem->Data.Data,PInit->Data);
  }
}

int ShowMenu(int Type)
{
  struct FarMenuItem shMenu[4]={0};
  static const char *HelpTopic[2]={"Find","Direct"};
  lstrcpy(shMenu[0].Text,GetMsg((Type?MBForward:MBrackMath)));
  lstrcpy(shMenu[1].Text,GetMsg((Type?MBBackward:MBrackSelect)));
  shMenu[2].Separator=1;
  lstrcpy(shMenu[3].Text,GetMsg(MConfigure));

  int Ret;
  while(1)
  {
    if((Ret=Info.Menu(Info.ModuleNumber,
             -1,-1,0,FMENU_WRAPMODE,GetMsg(MTitle),NULL,
             HelpTopic[Type&1],NULL,NULL,shMenu,
             sizeof(shMenu)/sizeof(shMenu[0]))) == 3)
      Config();
    else
      break;
  }
  return Ret;
}
