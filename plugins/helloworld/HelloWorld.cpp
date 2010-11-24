#include <windows.h>
#include "plugin.hpp"
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

enum {
  MTitle,
  MMessage1,
  MMessage2,
  MMessage3,
  MMessage4,
  MButton,
};

static struct PluginStartupInfo Info;

int WINAPI EXP_NAME(GetMinFarVersion)()
{
  return FARMANAGERVERSION;
}

/*
 Функция GetMsg возвращает строку сообщения из языкового файла.
 А это надстройка над Info.GetMsg для сокращения кода :-)
*/
const TCHAR *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

/*
Функция SetStartupInfo вызывается один раз, перед всеми
другими функциями. Она передается плагину информацию,
необходимую для дальнейшей работы.
*/
void WINAPI EXP_NAME(SetStartupInfo)(const struct PluginStartupInfo *psi)
{
  Info=*psi;
}

/*
Функция GetPluginInfo вызывается для получения основной
  (general) информации о плагине
*/
void WINAPI EXP_NAME(GetPluginInfo)(struct PluginInfo *pi)
{
  static const TCHAR *PluginMenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR;

  PluginMenuStrings[0]=GetMsg(MTitle);
  pi->PluginMenuStrings=PluginMenuStrings;
  pi->PluginMenuStringsNumber=ARRAYSIZE(PluginMenuStrings);
}

/*
  Функция OpenPlugin вызывается при создании новой копии плагина.
*/
HANDLE WINAPI EXP_NAME(OpenPlugin)(int OpenFrom,INT_PTR item)
{
  const TCHAR *Msg[]=
  {
    GetMsg(MTitle),
    GetMsg(MMessage1),
    GetMsg(MMessage2),
    GetMsg(MMessage3),
    GetMsg(MMessage4),
    _T("\x01"),                              /* separator line */
    GetMsg(MButton),
  };

  Info.Message(Info.ModuleNumber,            /* PluginNumber */
               FMSG_WARNING|FMSG_LEFTALIGN,  /* Flags */
               _T("Contents"),               /* HelpTopic */
               Msg,                          /* Items */
               ARRAYSIZE(Msg),               /* ItemsNumber */
               1);                           /* ButtonsNumber */

  return  INVALID_HANDLE_VALUE;
}
