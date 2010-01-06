#pragma once

#define _CRT_SECURE_NO_WARNINGS


#include "strmix.hpp"
#include "StringBase.hpp"
#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include <FarHelp.hpp>
#include <FarRegistry.hpp>
#include <FarDialogBase.hpp>
#include "FarPanelInfo.hpp"
#include <FarLNG.hpp>
#include <array.hpp>
#include <debug.h>
#include <farnew.hpp>


//newarc
#include "../../API/module.hpp"



class Archive;
class ArchiveModule;
class ArchiveModuleManager;
class ArchiveItemArray;
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

#define _M(id) (TCHAR*)Info.GetMsg (Info.ModuleNumber, id)

extern const TCHAR *GUID2STR (const GUID &uid);
extern const GUID& STR2GUID (const TCHAR *lpStr);

extern void FindDataToArchiveItem(const FAR_FIND_DATA *src, ArchiveItem *dest);


extern const TCHAR *pCommandNames[11];

#define FILE_ENCRYPTED 1



extern bool msgError(const TCHAR* lpErrorString);
