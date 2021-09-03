#include <plugin.hpp>
#include "HelloWorldLng.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"
#include "HelloWorld.hpp"

static struct PluginStartupInfo Info;

void WINAPI GetGlobalInfoW(struct GlobalInfo *GInfo)
{
	GInfo->StructSize=sizeof(struct GlobalInfo);
	GInfo->MinFarVersion=FARMANAGERVERSION;
	GInfo->Version=PLUGIN_VERSION;
	GInfo->Guid=MainGuid;
	GInfo->Title=PLUGIN_NAME;
	GInfo->Description=PLUGIN_DESC;
	GInfo->Author=PLUGIN_AUTHOR;
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
void WINAPI GetPluginInfoW(struct PluginInfo *PInfo)
{
	PInfo->StructSize=sizeof(*PInfo);
	PInfo->Flags=PF_EDITOR;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MTitle);
	PInfo->PluginMenu.Guids=&MenuGuid;
	PInfo->PluginMenu.Strings=PluginMenuStrings;
	PInfo->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
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
		NULL,
		FMSG_WARNING|FMSG_LEFTALIGN,  /* Flags */
		L"Contents",                  /* HelpTopic */
		MsgItems,                     /* Items */
		ARRAYSIZE(MsgItems),          /* ItemsNumber */
		1);                           /* ButtonsNumber */

	return NULL;
}
