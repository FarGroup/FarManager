/*
checkver.cpp

Проверка регистрации

*/

/* Revision: 1.00 25.06.2000 $ */

/*
Modify:
  25.06.2000 SVS
    ! Подготовка Master Copy
    ! Выделение в качестве самостоятельного модуля
*/

#include "headers.hpp"
#pragma hdrstop

/* $ 30.06.2000 IS
   Стандартные заголовки
*/
#include "internalheaders.hpp"
/* IS $ */


unsigned char MyName[]={'E'^0x50,'u'^0x51,'g'^0x52,'e'^0x53,'n'^0x54,'e'^0x55,' '^0x56,'R'^0x57,'o'^0x58,'s'^0x59,'h'^0x5a,'a'^0x5b,'l'^0x5c};
char *Days[]={"воскресенье","понедельник","вторник","среда","четверг",
              "пятница","суббота"};

#pragma warn -par
void _cdecl CheckVersion(void *Param)
{
  Sleep(1000);

  if (!RegVer)
  {
    Opt.TabSize=8;
    Opt.ViewerEditorClock=0;
  }
  _endthread();
}
#pragma warn +par


void Register()
{
  static struct DialogData RegDlgData[]=
  {
    DI_DOUBLEBOX,3,1,72,8,0,0,0,0,(char *)MRegTitle,
    DI_TEXT,5,2,0,0,0,0,0,0,(char *)MRegUser,
    DI_EDIT,5,3,70,3,1,0,0,0,"",
    DI_TEXT,5,4,0,0,0,0,0,0,(char *)MRegCode,
    DI_EDIT,5,5,70,5,0,0,0,0,"",
    DI_TEXT,3,6,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,1,(char *)MOk,
    DI_BUTTON,0,7,0,0,0,0,DIF_CENTERGROUP,0,(char *)MCancel
  };
  MakeDialogItems(RegDlgData,RegDlg);
  Dialog Dlg(RegDlg,sizeof(RegDlg)/sizeof(RegDlg[0]));
  Dlg.SetPosition(-1,-1,76,10);
  Dlg.SetHelp("Register");
  Dlg.Process();
  if (Dlg.GetExitCode()!=6)
    return;
  char RegName[256],RegCode[256],RegData[256];
  strcpy(RegName,RegDlg[2].Data);
  strcpy(RegCode,RegDlg[4].Data);
  int Length=strlen(RegName);
  if (*RegName==0 || *RegCode==0)
    return;
  unsigned char Xor=17;
  for (int I=0;RegName[I]!=0;I++)
    Xor^=RegName[I];
  int xUSSR=FALSE;
  if (strcmp(RegName,"xUSSR регистрация")==0)
  {
    SYSTEMTIME st;
    GetLocalTime(&st);
    if (strcmp(RegCode,Days[st.wDayOfWeek])==0)
      xUSSR=TRUE;
  }
  if (!xUSSR && (Length<4 || (Xor & 0xf)!=ToHex(RegCode[0]) || ((~(Xor>>4))&0xf)!=ToHex(RegCode[3])))
  {
    Message(MSG_WARNING,1,MSG(MError),MSG(MRegFailed),MSG(MOk));
    return;
  }
  Dlg.Hide();
  RegData[0]=clock();
  RegData[1]=strlen(RegName);
  RegData[2]=strlen(RegCode);
  strcpy(RegData+3,RegName);
  strcpy(RegData+RegData[1]+3,RegCode);
  int Size=RegData[1]+RegData[2]+3;
  for (int I=0;I<Size;I++)
    RegData[I]^=I+7;
  for (int I=1;I<Size;I++)
    RegData[I]^=RegData[0];
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  char SaveKey[512];
  strcpy(SaveKey,Opt.RegRoot);
  strcpy(Opt.RegRoot,"Software\\Far");
  SetRegKey("Registration","Data",(BYTE *)RegData,Size);
  strcpy(Opt.RegRoot,SaveKey);
  SetRegRootKey(HKEY_CURRENT_USER);
  Message(0,1,MSG(MRegTitle),MSG(MRegThanks),MSG(MOk));
}


void _cdecl CheckReg(void *Param)
{
  struct RegInfo *Reg=(struct RegInfo *)Param;
  char RegName[256],RegCode[256],RegData[256];
  DWORD Size=sizeof(RegData);
  SetRegRootKey(HKEY_LOCAL_MACHINE);
  char SaveKey[512];
  strcpy(SaveKey,Opt.RegRoot);
  strcpy(Opt.RegRoot,"Software\\Far");
  Size=GetRegKey("Registration","Data",(BYTE *)RegData,NULL,Size);
  strcpy(Opt.RegRoot,SaveKey);
  SetRegRootKey(HKEY_CURRENT_USER);
  memset(Reg,0,sizeof(*Reg));
  if (Size==0)
    RegVer=0;
  else
  {
    for (int I=1;I<Size;I++)
      RegData[I]^=RegData[0];
    for (int I=0;I<Size;I++)
      RegData[I]^=I+7;
    strncpy(RegName,RegData+3,RegData[1]);
    RegName[RegData[1]]=0;
    strncpy(RegCode,RegData+RegData[1]+3,RegData[2]);
    RegCode[RegData[2]]=0;
    RegVer=1;
    unsigned char Xor=17;
    for (int I=0;RegName[I]!=0;I++)
      Xor^=RegName[I];
    if ((Xor & 0xf)!=ToHex(RegCode[0]) || ((~(Xor>>4))&0xf)!=ToHex(RegCode[3]))
      RegVer=0;
    if (strcmp(RegName,"xUSSR регистрация")==0)
      RegVer=3;
    strcpy(Reg->RegCode,RegCode);
    strcpy(Reg->RegName,RegName);
    strcpy(::RegName,RegName);
  }
  Reg->Done=TRUE;
  _endthread();
}


char ToHex(char Ch)
{
  if (Ch>='0' && Ch<='9')
    return(Ch-'0');
  if (Ch>='a' && Ch<='z')
    return(Ch-'a'+10);
  if (Ch>='A' && Ch<='Z')
    return(Ch-'A'+10);
  return(0);
}


#pragma warn -par
void _cdecl ErrRegFn(void *Param)
{
  if (RegVer!=3)
  {
    Message(0,1,MSG(MRegTitle),MSG(MRegFailed),MSG(MOk));
    RegVer=0;
  }
  _endthread();
}
#pragma warn +par
