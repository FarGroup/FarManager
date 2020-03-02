#include <Rtl.Base.h>
#include <FarPluginBase.hpp>
#include "unzip.h"
#include "../../module.hpp"

typedef unzFile (__stdcall *UNZOPEN) (const char*);
typedef unzFile (__stdcall *UNZOPEN2) (const char *, zlib_filefunc_def*);


typedef int (__stdcall *UNZCLOSE) (unzFile);

typedef int (__stdcall *UNZGOTOFIRSTFILE) (unzFile);
typedef int (__stdcall *UNZGOTONEXTFILE) (unzFile);

typedef int (__stdcall *UNZGETCURRENTFILEINFO) (unzFile, unz_file_info *, char *, uLong, void *, uLong, char *, uLong);
typedef int (__stdcall *UNZGETGLOBALINFO) (unzFile, unz_global_info *);


typedef int (__stdcall *UNZOPENCURRENTFILE) (unzFile file);
typedef int (__stdcall *UNZOPENCURRENTFILEPASSWORD) (unzFile file, const char* password);
typedef int (__stdcall *UNZCLOSECURRENTFILE) (unzFile file);
typedef int (__stdcall *UNZREADCURRENTFILE) (unzFile file, voidp buf, unsigned len);



class ZipModule {

public:

	HMODULE m_hModule;

	UNZOPEN m_pfnUnzOpen;
	UNZOPEN2 m_pfnUnzOpen2;
	UNZCLOSE m_pfnUnzClose;

	UNZGOTOFIRSTFILE m_pfnUnzGoToFirstFile;
	UNZGOTONEXTFILE m_pfnUnzGoToNextFile;

	UNZGETCURRENTFILEINFO m_pfnUnzGetCurrentFileInfo;
	UNZGETGLOBALINFO m_pfnUnzGetGlobalInfo;

	UNZOPENCURRENTFILE m_pfnUnzOpenCurrentFile;
	UNZOPENCURRENTFILEPASSWORD m_pfnUnzOpenCurrentFilePassword;
	UNZCLOSECURRENTFILE m_pfnUnzCloseCurrentFile;
	UNZREADCURRENTFILE m_pfnUnzReadCurrentFile;

public:

	ZipModule (const char *lpFileName);
	~ZipModule ();
};

class ZipArchive {

public:

	ZipModule *m_pModule;

	unzFile m_hFile;

	ARCHIVECALLBACK m_pfnCallback;

	PBYTE m_pfnRarCallback;
	PBYTE m_pfnRarProcessDataProc;

	bool m_bAborted;

	char *m_lpFileName;

	bool m_bFirst;

public:

	ZipArchive (ZipModule *pModule, const char *lpFileName);
	virtual ~ZipArchive ();

	LONG_PTR Callback (int nMsg, int nParam1, LONG_PTR nParam2);

	virtual bool __stdcall pOpenArchive (int nOpMode, ARCHIVECALLBACK pfnCallback);
	virtual void __stdcall pCloseArchive ();

	virtual int __stdcall pGetArchiveItem (ArchiveItemInfo *pItem);
	virtual bool __stdcall pExtract (PluginPanelItem *pItems, int nItemsNumber, const char *lpDestPath, const char *lpCurrentFolder);

	virtual int __stdcall pGetArchiveType () {	return 0; }
};
