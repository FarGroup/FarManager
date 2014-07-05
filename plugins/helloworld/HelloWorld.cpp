#include <CRT/crt.hpp>
#include <plugin.hpp>
#include "HelloWorldLng.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"
#include "HelloWorld.hpp"


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

static struct PluginStartupInfo Info;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

/*
 Функция GetMsg возвращает строку сообщения из языкового файла.
 А это надстройка над Info.GetMsg для сокращения кода :-)
*/
const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

/*
Функция SetStartupInfoW вызывается один раз, перед всеми
другими функциями. Она передается плагину информацию,
необходимую для дальнейшей работы.
*/
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
	Info=*psi;
}

/*
Функция GetPluginInfoW вызывается для получения информации о плагине
*/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MTitle);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

/*
  Функция OpenPluginW вызывается при создании новой копии плагина.
*/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	const wchar_t *MsgItems[]=
	{
		GetMsg(MTitle),
		GetMsg(MMessage1),
		GetMsg(MMessage2),
		GetMsg(MMessage3),
		GetMsg(MMessage4),
		L"\x01",                      /* separator line */
		GetMsg(MButton),
	};

	Info.Message(&MainGuid,           /* GUID */
		nullptr,
		FMSG_WARNING|FMSG_LEFTALIGN,  /* Flags */
		L"Contents",                  /* HelpTopic */
		MsgItems,                     /* Items */
		ARRAYSIZE(MsgItems),          /* ItemsNumber */
		1);                           /* ButtonsNumber */

	return nullptr;
}

intptr_t WINAPI ProcessEditorInputW(const struct ProcessEditorInputInfo *Info)
{
   if
   (
      (Info->Rec.EventType != KEY_EVENT && Info->Rec.EventType != 0x8001) ||
      Info->Rec.Event.KeyEvent.bKeyDown == 0
   ) return 0;

   if ((Info->Rec.EventType == KEY_EVENT) && (Info->Rec.Event.KeyEvent.uChar.UnicodeChar == L'{'))
   {
      INPUT_RECORD tr;
      tr.EventType = KEY_EVENT;
      tr.Event.KeyEvent.bKeyDown = true;
      tr.Event.KeyEvent.wRepeatCount = 1;
      tr.Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
      tr.Event.KeyEvent.wVirtualScanCode = 0;
      tr.Event.KeyEvent.uChar.UnicodeChar = 0;
      tr.Event.KeyEvent.dwControlKeyState = 0;
      ::Info.EditorControl (-1, ECTL_PROCESSINPUT, 0, &tr); 
      return 1;
   }
   return 0;
}