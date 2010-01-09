const TCHAR *GetMsg(int MsgId)
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
    PItem->Param.History=(const TCHAR *)PInit->Selected;
    PItem->Flags=PInit->Flags;
    PItem->DefaultButton=PInit->DefaultButton;
#ifdef UNICODE
    PItem->MaxLen=0;
#endif
    if ((DWORD_PTR)PInit->Data<2000)
#ifndef UNICODE
      lstrcpy(PItem->Data.Data,GetMsg((unsigned int)(DWORD_PTR)PInit->Data));
#else
      PItem->PtrData = GetMsg((unsigned int)(DWORD_PTR)PInit->Data);
#endif
    else
#ifndef UNICODE
      lstrcpy(PItem->Data.Data,PInit->Data);
#else
      PItem->PtrData = PInit->Data;
#endif
  }
}

int ShowMenu(int Type)
{
  struct FarMenuItem shMenu[4]={0};
  static const TCHAR *HelpTopic[2]={_T("Find"),_T("Direct")};
#ifndef UNICODE
#define SET_MENUITEM(n,v)  lstrcpy(shMenu[n].Text, v)
#else
#define SET_MENUITEM(n,v)  shMenu[n].Text = v
#endif
  SET_MENUITEM(0,GetMsg((Type?MBForward:MBrackMath)));
  SET_MENUITEM(1,GetMsg((Type?MBBackward:MBrackSelect)));
  shMenu[2].Separator=1;
  SET_MENUITEM(3,GetMsg(MConfigure));

  int Ret;
  while(1)
  {
    if((Ret=Info.Menu(Info.ModuleNumber,
             -1,-1,0,FMENU_WRAPMODE,GetMsg(MTitle),NULL,
             HelpTopic[Type&1],NULL,NULL,shMenu,
             ArraySize(shMenu))) == 3)
      Config();
    else
      break;
  }
  return Ret;
}
