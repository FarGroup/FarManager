#ifndef __STDAFX_H__
#define __STDAFX_H__

// Insert your headers here
#define WIN32_LEAN_AND_MEAN   // Exclude rarely-used stuff from Windows headers

#define STRICT
#define __STD_STRING

#define _FAR_USE_FARFINDDATA

#pragma pack(push,2)
#include "../common/plugin.hpp"
#pragma pack(pop)

#include <shellapi.h>

#include "Memory.hpp"
#include "TmpLng.hpp"

#include "TmpClass.hpp"
#include "TmpPanel.hpp"
#include "TmpCfg.hpp"

#ifdef _MSC_VER
  #if _MSC_VER < 1400
    #pragma comment(linker, "/ignore:4078")
  #else
    #pragma warning(disable : 4078)
  #endif

  #pragma comment(linker, "/merge:.rdata=.text")
  #pragma comment(linker, "/section:.text,RWE")

  #if _MSC_VER < 1400
    #pragma comment(linker, "/ignore:4078")
  #else
    #pragma warning(disable : 4078)
  #endif

  #pragma function(memset)
  #define my_memset memset
#endif

//Include ODS headers
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STDAFX_H__ */
