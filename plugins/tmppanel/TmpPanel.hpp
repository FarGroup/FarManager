/*
TMPPANEL.HPP

Temporary panel header file

*/

#ifndef __TMPPANEL_HPP__
#define __TMPPANEL_HPP__

#include <CRT/crt.hpp>
#include "plugin.hpp"
#include <shellapi.h>

#include "TmpLng.hpp"
#include "TmpClass.hpp"
#include "TmpCfg.hpp"
#include "version.hpp"

#define COMMONPANELSNUMBER 10

typedef struct _MyInitDialogItem
{
	unsigned char Type;
	unsigned char X1,Y1,X2,Y2;
	DWORD Flags;
	signed char Data;
} MyInitDialogItem;

typedef struct _PluginPanels
{
	PluginPanelItem *Items;
	unsigned int ItemsNumber;
	unsigned int OpenFrom;
} PluginPanels;

extern PluginPanels CommonPanels[COMMONPANELSNUMBER];

extern unsigned int CurrentCommonPanel;

extern struct PluginStartupInfo Info;
extern struct FarStandardFunctions FSF;

extern int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;
extern wchar_t *PluginRootKey;

const wchar_t *GetMsg(int MsgId);

int Config();
void GoToFile(const wchar_t *Target, BOOL AnotherPanel);
void FreePanelItems(PluginPanelItem *Items, size_t Total);

wchar_t *ParseParam(wchar_t *& str);
void GetOptions(void);
void WFD2FFD(WIN32_FIND_DATA &wfd, PluginPanelItem &ffd);

#define BOM_UCS2     0xFEFF
#define BOM_UCS2_BE  0xFFFE
#define BOM_UTF8     0xBFBBEF

#define NT_MAX_PATH 32768

class StrBuf
{
	private:
		wchar_t *ptr;
		size_t len;

	private:
		StrBuf(const StrBuf &);
		StrBuf & operator=(const StrBuf &);

	public:
		StrBuf() { ptr = NULL; len = 0; }
		StrBuf(size_t len) { ptr = NULL; Reset(len); }
		~StrBuf() { free(ptr); }

	public:
		void Reset(size_t len) { if (ptr) free(ptr); ptr = (wchar_t *) malloc(len * sizeof(wchar_t)); *ptr = 0; this->len = len; }
		void Grow(size_t len) { if (len > this->len) Reset(len); }
		operator wchar_t*() { return ptr; }
		wchar_t *Ptr() { return ptr; }
		size_t Size() const { return len; }
};

class PtrGuard
{
	private:
		wchar_t *ptr;

	private:
		PtrGuard(const PtrGuard &);
		PtrGuard & operator=(const PtrGuard &);

	public:
		PtrGuard() { ptr = NULL; }
		PtrGuard(wchar_t *ptr) { this->ptr = ptr; }
		PtrGuard & operator=(wchar_t *ptr) { free(this->ptr); this->ptr = ptr; return *this; }
		~PtrGuard() { free(ptr); }

	public:
		operator wchar_t*() { return ptr; }
		wchar_t *Ptr() { return ptr; }
		wchar_t **PtrPtr() { return &ptr; }
};

wchar_t* FormNtPath(const wchar_t* path, StrBuf& buf);
wchar_t* GetFullPath(const wchar_t* input, StrBuf& output);
wchar_t* ExpandEnvStrs(const wchar_t* input, StrBuf& output);
bool FindListFile(const wchar_t *FileName, StrBuf &output);

#endif /* __TMPPANEL_HPP__ */
