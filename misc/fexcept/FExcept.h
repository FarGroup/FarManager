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

struct PLUGINRECORD       // информация о плагине
{
	DWORD TypeRec;          // Тип записи = RTYPE_PLUGIN
	DWORD SizeRec;          // Размер
	DWORD Reserved1[4];

	// DWORD SysID; GUID

	const wchar_t *ModuleName;

	DWORD Reserved2[2];    // разерв :-)

	DWORD SizeModuleName;
};

#if defined(__BORLANDC__)
  #pragma option -a.
#elif defined(__GNUC__) || (defined(__WATCOMC__) && (__WATCOMC__ < 1100)) || defined(__LCC__)
  #pragma pack()
#else
  #pragma pack(pop)
#endif

#endif
