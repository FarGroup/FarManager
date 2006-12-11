#define _FAR_USE_FARFINDDATA
#include "plugin.hpp"
#include "drawlng.hpp"
#include "drawline.hpp"
#include "CRT/crt.hpp"

#if defined(__GNUC__)

#ifdef __cplusplus
extern "C"{
#endif
  BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved);
#ifdef __cplusplus
};
#endif

BOOL WINAPI DllMainCRTStartup(HANDLE hDll,DWORD dwReason,LPVOID lpReserved)
{
  (void) lpReserved;
  (void) dwReason;
  (void) hDll;
  return TRUE;
}
#endif

static char  BoxChar[]  = {'³','´','µ','¶','·','¸','¹','º','»','¼','½','¾','¿','À','Á','Â','Ã','Ä','Å','Æ','Ç','È','É','Ê','Ë','Ì','Í','Î','Ï','Ð','Ñ','Ò','Ó','Ô','Õ','Ö','×','Ø','Ù','Ú'};
static short BoxLeft[]  = { 0 , 1 , 2 , 1 , 1 , 2 , 2 , 0 , 2 , 2 , 1 , 2 , 1 , 0 , 1 , 1 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 2 , 2 , 0 , 2 , 2 , 2 , 1 , 2 , 1 , 0 , 0 , 0 , 0 , 1 , 2 , 1 , 0 };
static short BoxUp[]    = { 1 , 1 , 1 , 2 , 0 , 0 , 2 , 2 , 0 , 2 , 2 , 1 , 0 , 1 , 1 , 0 , 1 , 0 , 1 , 1 , 2 , 2 , 0 , 2 , 0 , 2 , 0 , 2 , 1 , 2 , 0 , 0 , 2 , 1 , 0 , 0 , 2 , 1 , 1 , 0 };
static short BoxRight[] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 1 , 1 , 1 , 1 , 2 , 1 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 2 , 1 , 2 , 1 , 1 , 2 , 2 , 1 , 1 , 2 , 0 , 1 };
static short BoxDown[]  = { 1 , 1 , 1 , 2 , 2 , 1 , 2 , 2 , 2 , 0 , 0 , 0 , 1 , 0 , 0 , 1 , 1 , 0 , 1 , 1 , 2 , 0 , 2 , 0 , 2 , 2 , 0 , 2 , 0 , 0 , 1 , 2 , 0 , 0 , 1 , 2 , 2 , 1 , 0 , 1 };

void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *Info)
{
  ::Info=*Info;
}


HANDLE WINAPI _export OpenPlugin(int OpenFrom,INT_PTR Item)
{
  static int Reenter=FALSE;

  if (Reenter)
    return(INVALID_HANDLE_VALUE);
  Reenter=TRUE;

  int LineWidth=1, KeyCode /*I*/;
  BOOL Done=FALSE;
  INPUT_RECORD rec;
  /*COORD MousePos={-1,-1};*/

  SetTitle(LineWidth,(LineWidth==1)?MTitleSingle:MTitleDouble);

  while (!Done)
  {
    Info.EditorControl(ECTL_READINPUT,&rec);

    if ((rec.EventType&(~0x8000))!=KEY_EVENT || !rec.Event.KeyEvent.bKeyDown)
    {
#if 0
      if(rec.EventType==MOUSE_EVENT)
      {
        if((rec.Event.MouseEvent.dwControlKeyState&SHIFT_PRESSED) &&
         (rec.Event.MouseEvent.dwButtonState&(FROM_LEFT_1ST_BUTTON_PRESSED|RIGHTMOST_BUTTON_PRESSED)))
        {
          int DX=MousePos.X - rec.Event.MouseEvent.dwMousePosition.X;
          int DY=MousePos.Y - rec.Event.MouseEvent.dwMousePosition.Y;
          int KeyCodeX=!DX?VK_NUMPAD0:(DX<0?VK_NUMPAD6:VK_NUMPAD4);
          int KeyCodeY=!DY?VK_NUMPAD0:(DY<0?VK_NUMPAD2:VK_NUMPAD8);

          if(DX < 0) DX=-DX;
          if(DY < 0) DY=-DY;

          // Ctrl
          if(rec.Event.MouseEvent.dwControlKeyState&(LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
          {
            if(DX < DY)
              DX=0;
            else if(DX > DY)
              DY=0;
          }

          if(DX && !DY)
          {
            for(I=0; I < DX; ++I)
              ProcessShiftKey(KeyCodeX,
                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
          }
          else if(!DX && DY)
          {
            for(I=0; I < DY; ++I)
              ProcessShiftKey(KeyCodeY,
                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
          }
          else if(DX && DY)
          {
            for(I=0;; ++I)
            {
              ProcessShiftKey(KeyCodeX,
                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
              ProcessShiftKey(KeyCodeY,
                (rec.Event.MouseEvent.dwButtonState&FROM_LEFT_1ST_BUTTON_PRESSED)?1:2);
              DX--;
              DY--;
              if(DX < 0 && DY < 0) //??
                break;
            }
          }
        }
        MousePos=rec.Event.MouseEvent.dwMousePosition;
        Info.EditorControl(ECTL_PROCESSINPUT,&rec);
        continue;
      }
      else
#endif
      {
        Info.EditorControl(ECTL_PROCESSINPUT,&rec);
        continue;
      }
    }
    else
      KeyCode=rec.Event.KeyEvent.wVirtualKeyCode;

    switch(KeyCode)
    {
      case VK_ESCAPE:
      case VK_F10:
        if ((rec.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED|LEFT_CTRL_PRESSED|
            RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
          Done=TRUE;
        break;

      case VK_F1:
        Info.ShowHelp(Info.ModuleName,NULL,0);
        break;

      case VK_F2:
        if ((rec.Event.KeyEvent.dwControlKeyState & (SHIFT_PRESSED|LEFT_CTRL_PRESSED|
            RIGHT_CTRL_PRESSED|LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))==0)
        {
          LineWidth=3-LineWidth;
          SetTitle(LineWidth,((LineWidth==1)?MTitleSingle:MTitleDouble));
        }
        break;

      default:
        if (KeyCode>=VK_PRIOR && KeyCode<=VK_DOWN ||
            KeyCode>=VK_NUMPAD0 && KeyCode<=VK_NUMPAD9)
        {
          if(rec.Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED)
            ProcessShiftKey(KeyCode,LineWidth);
          else
            Info.EditorControl(ECTL_PROCESSINPUT,&rec);
        }
        else
        {
          if(KeyCode < VK_F3 || KeyCode > VK_F12)
            Info.EditorControl(ECTL_PROCESSINPUT,&rec);
          continue;
        }
        break;
    }
  }
  Info.EditorControl(ECTL_SETKEYBAR,NULL);
  Reenter=FALSE;
  return(INVALID_HANDLE_VALUE);
}


void SetTitle(int LineWidth,int IDTitle)
{
  int I;
  struct KeyBarTitles Kbt;
  for(I=0; I < 12; ++I)
  {
    Kbt.Titles[I]="";
    Kbt.CtrlTitles[I]="";
    Kbt.AltTitles[I]="";
    Kbt.ShiftTitles[I]="";
    Kbt.CtrlShiftTitles[I]="";
    Kbt.AltShiftTitles[I]="";
    Kbt.CtrlAltTitles[I]="";
  }
  Kbt.Titles[1-1]=(char *)GetMsg(MHelp);
  Kbt.Titles[2-1]=(char *)GetMsg((LineWidth==1)?MDouble:MSingle);
  Kbt.Titles[10-1]=(char *)GetMsg(MQuit);
  Info.EditorControl(ECTL_SETKEYBAR,&Kbt);
  Info.EditorControl(ECTL_SETTITLE,(char *)GetMsg(IDTitle));
}

void ProcessShiftKey(int KeyCode,int LineWidth)
{
  struct EditorInfo ei;
  Info.EditorControl(ECTL_GETINFO,&ei);

  struct EditorSetPosition esp;
  esp.CurLine=ei.CurLine;
  esp.CurPos=ei.CurTabPos;
  esp.CurTabPos=-1;
  esp.TopScreenLine=-1;
  esp.LeftPos=-1;
  esp.Overtype=-1;

  if (ei.CurLine>0)
  {
    int StringNumber=ei.CurLine-1;
    Info.EditorControl(ECTL_EXPANDTABS,&StringNumber);
  }
  Info.EditorControl(ECTL_EXPANDTABS,&ei.CurLine);
  if (ei.CurLine>=ei.TotalLines-1)
  {
    struct EditorGetString egs;
    egs.StringNumber=ei.CurLine;
    Info.EditorControl(ECTL_GETSTRING,&egs);
    struct EditorSetPosition esp;
    esp.CurLine=ei.CurLine;
    esp.CurPos=egs.StringLength;
    esp.CurTabPos=-1;
    esp.TopScreenLine=-1;
    esp.LeftPos=-1;
    esp.Overtype=-1;
    Info.EditorControl(ECTL_SETPOSITION,&esp);
    Info.EditorControl(ECTL_INSERTSTRING,NULL);
    esp.CurLine=ei.CurLine;
    esp.CurPos=ei.CurTabPos;
    Info.EditorControl(ECTL_SETPOSITION,&esp);
  }

  if (ei.CurLine<ei.TotalLines-1)
  {
    int StringNumber=ei.CurLine+1;
    Info.EditorControl(ECTL_EXPANDTABS,&StringNumber);
  }
  Info.EditorControl(ECTL_GETINFO,&ei);

  struct EditorGetString egs;
  egs.StringNumber=ei.CurLine;
  Info.EditorControl(ECTL_GETSTRING,&egs);

  int StringLength=egs.StringLength>ei.CurPos ? egs.StringLength:ei.CurPos+1;
  char *NewString=(char *)malloc(StringLength);
  if (StringLength>egs.StringLength)
    memset(NewString+egs.StringLength,' ',StringLength-egs.StringLength);
  memcpy(NewString,egs.StringText,egs.StringLength);

  int LeftLine,UpLine,RightLine,DownLine;
  GetEnvType(NewString,StringLength,&ei,LeftLine,UpLine,RightLine,DownLine);

  switch(KeyCode)
  {
    case VK_UP:
    case VK_NUMPAD8:
      UpLine=LineWidth;
      if (LeftLine==0 && RightLine==0)
        DownLine=UpLine;
      if (esp.CurLine>0)
        esp.CurLine--;
      break;
    case VK_DOWN:
    case VK_NUMPAD2:
      DownLine=LineWidth;
      if (LeftLine==0 && RightLine==0)
        UpLine=DownLine;
      esp.CurLine++;
      break;
    case VK_LEFT:
    case VK_NUMPAD4:
      LeftLine=LineWidth;
      if (UpLine==0 && DownLine==0)
        RightLine=LeftLine;
      if (esp.CurPos>0)
        esp.CurPos--;
      break;
    case VK_RIGHT:
    case VK_NUMPAD6:
      RightLine=LineWidth;
      if (UpLine==0 && DownLine==0)
        LeftLine=RightLine;
      esp.CurPos++;
      break;
  }

  if (LeftLine!=0 && RightLine!=0 && LeftLine!=RightLine)
    LeftLine=RightLine=LineWidth;

  if (UpLine!=0 && DownLine!=0 && UpLine!=DownLine)
    UpLine=DownLine=LineWidth;

  for (size_t I=0;I<sizeof(BoxChar);I++)
    if (LeftLine==BoxLeft[I] && UpLine==BoxUp[I] &&
        RightLine==BoxRight[I] && DownLine==BoxDown[I])
    {
      NewString[ei.CurPos]=BoxChar[I];

      struct EditorConvertText ect;
      ect.Text=NewString+ei.CurPos;
      ect.TextLength=1;
      Info.EditorControl(ECTL_OEMTOEDITOR,&ect);

      struct EditorSetString ess;
      ess.StringNumber=egs.StringNumber;
      ess.StringText=NewString;
      ess.StringEOL=(char*)egs.StringEOL;
      ess.StringLength=StringLength;

      Info.EditorControl(ECTL_SETSTRING,&ess);
      Info.EditorControl(ECTL_SETPOSITION,&esp);
      Info.EditorControl(ECTL_REDRAW,NULL);
      break;
    }
  free(NewString);
}


void GetEnvType(char *NewString,int StringLength,struct EditorInfo *ei,
                int &LeftLine,int &UpLine,int &RightLine,int &DownLine)
{
  char OldChar[3];

  OldChar[0]=ei->CurPos>0 ? NewString[ei->CurPos-1]:' ';
  OldChar[1]=NewString[ei->CurPos];
  OldChar[2]=ei->CurPos<StringLength-1 ? NewString[ei->CurPos+1]:' ';

  struct EditorConvertText ect;
  ect.Text=OldChar;
  ect.TextLength=sizeof(OldChar);
  Info.EditorControl(ECTL_EDITORTOOEM,&ect);

  char LeftChar=OldChar[0];
  char RightChar=OldChar[2];
  char UpChar=' ';
  char DownChar=' ';

  if (ei->CurLine>0)
  {
    struct EditorGetString UpStr;
    UpStr.StringNumber=ei->CurLine-1;
    Info.EditorControl(ECTL_GETSTRING,&UpStr);
    if (ei->CurPos<UpStr.StringLength)
      UpChar=UpStr.StringText[ei->CurPos];
    struct EditorConvertText ect;
    ect.Text=&UpChar;
    ect.TextLength=1;
    Info.EditorControl(ECTL_EDITORTOOEM,&ect);
  }
  if (ei->CurLine<ei->TotalLines-1)
  {
    struct EditorGetString DownStr;
    DownStr.StringNumber=ei->CurLine+1;
    Info.EditorControl(ECTL_GETSTRING,&DownStr);
    if (ei->CurPos<DownStr.StringLength)
      DownChar=DownStr.StringText[ei->CurPos];
    struct EditorConvertText ect;
    ect.Text=&DownChar;
    ect.TextLength=1;
    Info.EditorControl(ECTL_EDITORTOOEM,&ect);
  }
  LeftLine=UpLine=RightLine=DownLine=0;
  for (size_t I=0;I<sizeof(BoxChar);I++)
  {
    if (LeftChar==BoxChar[I])
      LeftLine=BoxRight[I];
    if (UpChar==BoxChar[I])
      UpLine=BoxDown[I];
    if (RightChar==BoxChar[I])
      RightLine=BoxLeft[I];
    if (DownChar==BoxChar[I])
      DownLine=BoxUp[I];
  }
}


void WINAPI _export GetPluginInfo(struct PluginInfo *Info)
{
  Info->StructSize=sizeof(*Info);
  Info->Flags=PF_EDITOR|PF_DISABLEPANELS;
  Info->DiskMenuStringsNumber=0;
  static const char *PluginMenuStrings[1];
  PluginMenuStrings[0]=GetMsg(MDrawLines);
  Info->PluginMenuStrings=PluginMenuStrings;
  Info->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
  Info->PluginConfigStringsNumber=0;
}

const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}
