#pragma once
#include "FarPluginBase.hpp"
#include "SystemApi.hpp"
#include "strmix.hpp"
#include "debug.h"
#include "makeguid.h"

#if defined(_MSC_VER)
#pragma pack(push,8)
#else
#pragma pack(8)
#endif

//observer
#include "Observer/ModuleDef.h"

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#pragma pack()
#endif



struct ProgressContextEx {
	ProgressContext ctx;
	HANDLE filler;
	HANDLE hArchive;
};

//newarc
#include "../../API/module.hpp"

class ObserverArchive;
class ObserverModule;
class ObserverPlugin;

#include "observer.Archive.h"
#include "observer.Plugin.h"
#include "observer.Module.h"