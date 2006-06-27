#include <windows.h>
#include <string.h>
#include "plugin.hpp"

#if defined(__GNUC__)

#include "crt.hpp"

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

/*
 �㭪�� GetMsg �����頥� ��ப� ᮮ�饭�� �� �몮���� 䠩��.
 � �� �����ன�� ��� Info.GetMsg ��� ᮪�饭�� ���� :-)
*/
const char *GetMsg(int MsgId)
{
  return(Info.GetMsg(Info.ModuleNumber,MsgId));
}

/*
�㭪�� SetStartupInfo ��뢠���� ���� ࠧ, ��। �ᥬ�
��㣨�� �㭪�ﬨ. ��� ��।����� ������� ���ଠ��,
����室���� ��� ���쭥�襩 ࠡ���.
*/
void WINAPI _export SetStartupInfo(const struct PluginStartupInfo *psi)
{
  Info=*psi;
}

/*
�㭪�� GetPluginInfo ��뢠���� ��� ����祭�� �᭮����
  (general) ���ଠ樨 � �������
*/
void WINAPI _export GetPluginInfo(struct PluginInfo *pi)
{
  static const char *PluginMenuStrings[1];

  pi->StructSize=sizeof(struct PluginInfo);
  pi->Flags=PF_EDITOR;

  PluginMenuStrings[0]=GetMsg(MTitle);
  pi->PluginMenuStrings=PluginMenuStrings;
  pi->PluginMenuStringsNumber=sizeof(PluginMenuStrings)/sizeof(PluginMenuStrings[0]);
}

/*
  �㭪�� OpenPlugin ��뢠���� �� ᮧ����� ����� ����� �������.
*/
HANDLE WINAPI _export OpenPlugin(int OpenFrom,int item)
{
  const char *Msg[7];

  Msg[0]=GetMsg(MTitle);
  Msg[1]=GetMsg(MMessage1);
  Msg[2]=GetMsg(MMessage2);
  Msg[3]=GetMsg(MMessage3);
  Msg[4]=GetMsg(MMessage4);
  Msg[5]="\x01";                   /* separator line */
  Msg[6]=GetMsg(MButton);

  Info.Message(Info.ModuleNumber,  /* PluginNumber */
               FMSG_WARNING|FMSG_LEFTALIGN,  /* Flags */
               "Contents",         /* HelpTopic */
               Msg,                /* Items */
               7,                  /* ItemsNumber */
               1);                 /* ButtonsNumber */

  return  INVALID_HANDLE_VALUE;
}
