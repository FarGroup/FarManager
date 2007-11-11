#include "proclist.hpp"
#include "proclng.hpp"
#include <stdlib.h>

InitDialogItem ViewItems[NVIEWITEMS]={
    /* 0 */{DI_TEXT,5,0,0,0,0,0,0,0,(TCHAR *)MIncludeAdditionalInfo},
    /* 1 */{DI_CHECKBOX,5,1,0,0,0,0,0,0,(TCHAR *)MInclEnvironment},
    /* 2 */{DI_CHECKBOX,5,2,0,0,0,0,0,0,(TCHAR *)MInclModuleInfo},
    /* 3 */{DI_CHECKBOX,5,3,0,0,0,0,0,0,(TCHAR *)MInclModuleVersion},
    /* 4 */{DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)MInclPerformance},
    /* 5 */{DI_CHECKBOX,5,5,0,0,0,0,0,0,(TCHAR *)MInclHandles},
    /* 6 */{DI_CHECKBOX,35,5,0,0,0,0,DIF_HIDDEN,0,(TCHAR *)MInclHandlesUnnamed},
};

void MakeViewOptions(FarDialogItem* Items, _Opt& Opt, int offset)
{
    InitDialogItems(ViewItems,Items,NVIEWITEMS);
    for(int i=0; i<NVIEWITEMS; i++)
        Items[i].Y1 += offset;
    if(!NT) {
        Items[1].Flags |= DIF_DISABLE;
        Items[5].Flags |= DIF_DISABLE;
        Items[6].Flags |= DIF_DISABLE;
    }
    Items[1].Selected = Opt.ExportEnvironment;
    Items[2].Selected = Opt.ExportModuleInfo;
    Items[3].Selected = Opt.ExportModuleVersion;
    Items[4].Selected = Opt.ExportPerformance;
    Items[5].Selected = Opt.ExportHandles&1;
    Items[6].Selected = (Opt.ExportHandles&2)!=0;
    if(!NT)
        Items[1].Selected = 0;
}

void GetViewOptions(FarDialogItem* Items, _Opt& Opt)
{
    Opt.ExportEnvironment = Items[1].Selected;
    Opt.ExportModuleInfo = Items[2].Selected;
    Opt.ExportModuleVersion = Items[3].Selected;
    Opt.ExportPerformance = Items[4].Selected;
    Opt.ExportHandles = Items[5].Selected | (Items[6].Selected<<1);
}

int Config()
{
  InitDialogItem InitItems[]={
  /*  0 */{DI_DOUBLEBOX,3,1,72,14,0,0,0,0,(TCHAR *)MConfigPlistPanel},
  /*  1 */{DI_CHECKBOX,5,2,0,0,0,0,0,0,(TCHAR *)MConfigAddToDisksMenu},
  /*  2 */{DI_FIXEDIT,7,3,7,3,1,0,0,0,_T("")},
  /*  3 */{DI_TEXT,9,3,0,0,0,0,0,0,(TCHAR *)MConfigDisksMenuDigit},
  /*  4 */{DI_CHECKBOX,5,4,0,0,0,0,0,0,(TCHAR *)MConfigAddToPluginMenu},
  /*  5 */{DI_TEXT,5,5,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          {DI_TEXT,5,12,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,_T("")},
          {DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,1,(TCHAR *)MOk},
          {DI_BUTTON,0,13,0,0,0,0,DIF_CENTERGROUP,0,(TCHAR *)MCancel},
  };
/*  if(!NT) {
        InitItems[ 7].Flags |= DIF_DISABLE;
        InitItems[11].Flags |= DIF_DISABLE;
  }
*/
  if(!Plist::PanelModesInitialized())
      Plist::InitializePanelModes();

#define NITEMS (ArraySize(InitItems) + NVIEWITEMS)

  FarDialogItem DialogItems[NITEMS];
  InitDialogItems(InitItems,DialogItems,ArraySize(InitItems));
  memcpy(DialogItems+NITEMS-2, DialogItems+NITEMS-NVIEWITEMS-2, sizeof(*DialogItems) * 2);

  MakeViewOptions(DialogItems+NITEMS-NVIEWITEMS-2, ::Opt, NITEMS-NVIEWITEMS-3);

  DialogItems[1].Selected = Opt.AddToDisksMenu;
  DialogItems[4].Selected = Opt.AddToPluginsMenu;
#ifdef UNICODE
  wchar_t tmpstr[64];
  DialogItems[2].DataIn = DialogItems[2].DataOut = tmpstr;
  DialogItems[2].MaxLen = ArraySize(tmpstr);
#define Data DataOut
#endif
  if (Opt.DisksMenuDigit)
    FSF.itoa(Opt.DisksMenuDigit,DialogItems[2].Data,10);

  int ExitCode = Info.Dialog(Info.ModuleNumber,-1,-1,76,16,_T("Config"),
                             DialogItems,ArraySize(DialogItems)
#ifdef UNICODE
                             , NULL
#endif
                             );
  if (ExitCode != ArraySize(DialogItems) - 2)
    return FALSE;

  Opt.AddToDisksMenu = DialogItems[1].Selected;
  Opt.DisksMenuDigit = FSF.atoi(DialogItems[2].Data);
#undef Data
  Opt.AddToPluginsMenu = DialogItems[4].Selected;
  GetViewOptions(DialogItems+NITEMS-NVIEWITEMS-2, ::Opt);

  Opt.Write();

  Plist::SavePanelModes();
  Plist::bInit = false;

  return TRUE;
}

#define SETKEY(_opt) SetRegKey(0, _T( #_opt ), Opt._opt);
void _Opt::Write()
{
  SETKEY(AddToDisksMenu)
  SETKEY(DisksMenuDigit)
  SETKEY(AddToPluginsMenu)
  SETKEY(ExportEnvironment)
  SETKEY(ExportModuleInfo)
  SETKEY(ExportModuleVersion)
  SETKEY(ExportPerformance)
  SETKEY(ExportHandles)
//  SETKEY(EnableWMI)
}

#define GETKEY(_opt, _dflt) Opt._opt = GetRegKey(0, _T( #_opt ), _dflt);

void _Opt::Read()
{
  GETKEY(AddToDisksMenu, 1)
  GETKEY(AddToPluginsMenu, 1)
  GETKEY(DisksMenuDigit, 0)
  GETKEY(ExportEnvironment, 1)
  GETKEY(ExportModuleInfo, 1)
  GETKEY(ExportModuleVersion, 0)
  GETKEY(ExportPerformance, 1)
  GETKEY(ExportHandles, 0)
  GETKEY(EnableWMI, 1)
}
