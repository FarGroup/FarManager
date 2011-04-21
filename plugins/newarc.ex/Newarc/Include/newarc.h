#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <map>

#include "strmix.hpp"
#include "SystemApi.hpp"
#include "StringBase.hpp"
#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <FarRegistry.hpp>
#include <FarLNG.hpp>
#include <array.hpp>
#include <debug.h>


//newarc
#include "../../API/module.hpp"

class Archive;
class ArchiveModule;
class ArchiveModuleManager;
class ArchiveItemArray;
class ArchivePItemArray;
class ArchivePanel;
class ArchiveFilter;
class ArchivePlugin;
class ArchiveFormat;
class ArchiveTemplate;
struct ArchiveFilterEntry;

typedef Array<const ArchiveFormatInfo*> ArchiveFormatInfoArray;
typedef Array<const ArchivePluginInfo*> PluginFormatInfoArray;
typedef Array<const ArchiveModuleInfo*> ModuleFormatInfoArray;

typedef Array<ArchiveModule*> ArchiveModuleArray;
typedef Array<ArchiveFilterEntry*> ArchiveFilterArray;

//self
#include "newarc.ArchiveTree.h"
#include "newarc.Archive.h"
#include "newarc.ArchiveModule.h"
#include "newarc.ArchiveFilter.h"
#include "newarc.ArchivePlugin.h"
#include "newarc.ArchiveFormat.h"
#include "newarc.ArchiveModuleManager.h"
#include "newarc.ArchiveTemplate.h"
#include "itemarray.hpp"
#include "newarc.ArchivePanel.h"
#include "newarc.Messages.h"



enum ArchiveOutputSettings {
	ARCHIVER_OUTPUT_SHOW_ALWAYS,
	ARCHIVER_OUTPUT_SHOW_EDIT_VIEW,
	ARCHIVER_OUTPUT_SHOW_NEVER
};

struct ArchiveFormatCommands {
	ArchiveFormat* pFormat;
	string Commands[MAX_COMMANDS];
};

struct Configuration {
	ArchiveOutputSettings uArchiverOutput;
	std::map<const ArchiveFormat*, ArchiveFormatCommands*> pArchiveCommands;
};

extern Configuration cfg;

#define _M(id) (TCHAR*)Info.GetMsg (Info.ModuleNumber, id)

extern const TCHAR *GUID2STR(const GUID &uid);
extern const GUID& STR2GUID(const TCHAR *lpStr);

extern void FindDataToArchiveItem(const FAR_FIND_DATA *src, ArchiveItem *dest);


extern const TCHAR *pCommandNames[11];

#define FILE_ENCRYPTED 1

extern bool msgError(const TCHAR* lpErrorString);
