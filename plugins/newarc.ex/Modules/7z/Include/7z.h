#pragma once

#define _M(id) (TCHAR*)Info.GetMsg(Info.ModuleNumber, id)

//far
#include "FarPluginBase.hpp"
#include "SystemApi.hpp"
#include "strmix.hpp"
#include "makeguid.h"
#include "debug.h"

//newarc
#include "../../../API/module.hpp"

//7z
#include "../7z.headers/7zip/Archive/IArchive.h"
#include "../7z.headers/7zip/Common/FileStreams.h"
#include "../7z.headers/7zip/IPassword.h"

class SevenZipModule;
class SevenZipPlugin;
class SevenZipArchive;

extern "C" const GUID CLSID_CZipHandler;
extern "C" const GUID CLSID_CBZip2Handler;
extern "C" const GUID CLSID_CRarHandler;
extern "C" const GUID CLSID_CArjHandler;
extern "C" const GUID CLSID_CZHandler;
extern "C" const GUID CLSID_CLzhHandler;
extern "C" const GUID CLSID_CFormat7z;
extern "C" const GUID CLSID_CCabHandler;
extern "C" const GUID CLSID_CNsisHandler;
extern "C" const GUID CLSID_CLzmaHandler;
extern "C" const GUID CLSID_CLzma86Handler;
extern "C" const GUID CLSID_CXzHandler;
extern "C" const GUID CLSID_CTarHandler;
extern "C" const GUID CLSID_CGZipHandler;

#include "7z.Messages.h"
#include "7z.Commands.h"
#include "7z.Module.h"
#include "7z.DetectArchive.h"
#include "7z.File.h"

class CArchiveExtractCallback;
class CArchiveOpenCallback;
class CArchiveOpenVolumeCallback;
class CArchiveUpdateCallback;
class CCryptoGetTextPassword;

#include "Helpers/7z.ArchiveExtractCallback.h"
#include "Helpers/7z.ArchiveOpenCallback.h"
#include "Helpers/7z.ArchiveOpenVolumeCallback.h"
#include "Helpers/7z.ArchiveUpdateCallback.h"
#include "Helpers/7z.ArchiveDeleteCallback.h"
#include "Helpers/7z.CryptoGetTextPassword.h"


#include "7z.Plugin.h"
#include "7z.Archive.h"
#include "PropVariant.h"
#include "7z.Config.h"
#include "7z.Properties.h"
