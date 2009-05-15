#ifndef __FAR_EXCEPTION_HEADER
#define __FAR_EXCEPTION_HEADER

#if defined(__BORLANDC__)
  #pragma option -a2
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack(2)
#else
  #pragma pack(push,2)
#endif

#define FAR_LOG_VERSION  1

#define ArraySize(a)  (sizeof(a)/sizeof(a[0]))

#ifndef UNICODE
struct PLUGINRECORD{      // информация о плагине
  DWORD Reserved0;
  DWORD SizeRec;          // Размер
  DWORD Reserved1;

  DWORD WorkFlags;        // рабочие флаги текущего плагина
  DWORD FuncFlags;        // битовые маски эксп. функций плагина (бит есть - есть и функция)
  DWORD CallFlags;        // битовые маски вызова эксп. функций плагина

  short CachePos;         // позиция в кеше
  DWORD SysID;

  struct {
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    char     cFileName[MAX_PATH];
    char     cAlternateFileName[14];
  } FindData;

  DWORD Reserved2[3];    // разерв :-)
};
#else
struct PLUGINRECORD{      // информация о плагине
  DWORD TypeRec;          // Тип записи = RTYPE_PLUGIN
  DWORD SizeRec;          // Размер
  DWORD Reserved1;

  DWORD WorkFlags;        // рабочие флаги текущего плагина
  DWORD FuncFlags;        // битовые маски эксп.функций плагина (бит есть - ест и функция)
  DWORD CallFlags;        // битовые маски вызова эксп.функций плагина

  DWORD SysID;

  const wchar_t *ModuleName;

  DWORD Reserved2[2];    // разерв :-)

  DWORD SizeModuleName;
};
#endif

#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

#endif
