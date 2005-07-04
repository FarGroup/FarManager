#pragma once

#include <Rtl.Base.h>
#include <FarPluginBase.h>
#include <FarHelp.h>
#include <FarDialogs.h>
#include <FarLNG.h>
#include <Registry.h>
#include "newarc.Messages.h"

#define _M(id) (char*)Info.GetMsg (Info.ModuleNumber, id)

class ArchivePlugin;
class Archive;

extern Collection<ArchivePlugin*> Plugins;

/*char *pCommandNames[11] = {
		"Extract",
        "ExtractWithoutPath",
        "Test",
        "Delete",
        "ArchiveComment",
        "FileComment",
        "ConvertToSFX",
        "Lock",
        "AddRecoveryRecord",
        "Recover",
        "Add"
        };*/

extern char *pCommandNames[11];


#ifdef _DEBUG
#include <debug.h>
#endif
#include <Collections.h>
#include "module.hpp"
#include "newarc.archiveplugin.h"
#include "newarc.archive.h"
#include "newarc.panel.h"

#define FILE_ENCRYPTED 1

struct ArchiveFile {
	dword dwFlags;
	dword dwDictionarySize;

	dword dwPackedFileSizeLow;
	dword dwPackedFileSizeHigh;

	dword dwFileCRC;
};

/*struct Archive {
	HANDLE hFile;

	dword dwNextPositionLow;
	dword dwNextPositionHigh;
};*/

#ifdef __cplusplus
extern "C"{
#endif

ArchivePanel *__stdcall _export OpenFilePlugin (
		const char *lpFileName,
		const unsigned char *pData,
		int nDataSize
		);

int __stdcall _export GetFindData (
		ArchivePanel *pPanel,
		PluginPanelItem **pPanelItem,
		int *pItemsNumber,
		int OpMode
		);

void __stdcall _export FreeFindData (
		ArchivePanel *pPanel,
		PluginPanelItem *pPanelItem,
		int nItemsNumber
		);


void __stdcall _export ClosePlugin (
		ArchivePanel *pPanel
		);

void __stdcall _export GetOpenPluginInfo (
		ArchivePanel *pPanel,
		OpenPluginInfo *pInfo
		);

int __stdcall _export GetFiles (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		char *DestPath,
		int OpMode
		);

int __stdcall _export PutFiles (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int Move,
		int OpMode
		);

int __stdcall _export DeleteFiles (
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		);

void __stdcall _export SetStartupInfo (
		PluginStartupInfo *pInfo
		);

int __stdcall _export ProcessHostFile(
		ArchivePanel *pPanel,
		PluginPanelItem *PanelItem,
		int ItemsNumber,
		int OpMode
		);

int __stdcall _export SetDirectory (
		ArchivePanel *pPanel,
		const char *Dir,
		int nOpMode
		);

int __stdcall _export ProcessKey (
		ArchivePanel *pPanel,
		int nKey,
		dword dwControlState
		);

void __stdcall _export ExitFAR ();

void __stdcall _export GetPluginInfo (
		PluginInfo *pi
		);

int __stdcall _export Configure (
		int nItem
		);

int __stdcall _export ProcessEditorEvent(
		int nEvent,
		void *Param
		);

#ifdef __cplusplus
};
#endif
