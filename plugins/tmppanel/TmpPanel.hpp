/*
TMPPANEL.HPP

Temporary panel header file

*/

#ifndef __TMPPANEL_HPP__
#define __TMPPANEL_HPP__

#define COMMONPANELSNUMBER 10

struct MyInitDialogItem
{
	unsigned char Type;
	unsigned char X1,Y1,X2,Y2;
	DWORD Flags;
	signed char Data;
};

struct PluginPanels
{
	PluginPanelItem *Items;
	unsigned int ItemsNumber;
	unsigned int OpenFrom;
};

extern PluginPanels CommonPanels[COMMONPANELSNUMBER];

extern unsigned int CurrentCommonPanel;

extern PluginStartupInfo PsInfo;
extern FarStandardFunctions FSF;

extern int StartupOptFullScreenPanel,StartupOptCommonPanel,StartupOpenFrom;
extern wchar_t *PluginRootKey;

const wchar_t *GetMsg(int MsgId);

int Config();
void GoToFile(const wchar_t *Target, BOOL AnotherPanel);
void FreePanelItems(PluginPanelItem *Items, size_t Total);

wchar_t *ParseParam(wchar_t *& str);
void GetOptions();
void WFD2FFD(WIN32_FIND_DATA &wfd, PluginPanelItem &ffd);

bool IsTextUTF8(const char* Buffer,size_t Length);

#define NT_MAX_PATH 32768

#define SIGN_UNICODE    0xFEFF
#define SIGN_REVERSEBOM 0xFFFE
#define SIGN_UTF8_LO    0xBBEF
#define SIGN_UTF8_HI    0xBF


class StrBuf
{
	private:
		wchar_t *m_Ptr{};
		size_t m_Len{};

	private:
		StrBuf(const StrBuf &);
		StrBuf & operator=(const StrBuf &);

	public:
		StrBuf() = default;
		StrBuf(size_t len) { Reset(len); }
		~StrBuf() { free(m_Ptr); }

	public:
		void Reset(size_t len) { free(m_Ptr); m_Ptr = (wchar_t *) malloc(len * sizeof(wchar_t)); *m_Ptr = 0; m_Len = len; }
		void Grow(size_t len) { if (len > m_Len) Reset(len); }
		operator wchar_t*() { return m_Ptr; }
		wchar_t *Ptr() { return m_Ptr; }
		size_t Size() const { return m_Len; }
};

class PtrGuard
{
	private:
		wchar_t* m_Ptr{};

	private:
		PtrGuard(const PtrGuard &);
		PtrGuard & operator=(const PtrGuard &);

	public:
		PtrGuard() = default;
		PtrGuard(wchar_t *ptr) { m_Ptr = ptr; }
		PtrGuard & operator=(wchar_t *ptr) { free(m_Ptr); m_Ptr = ptr; return *this; }
		~PtrGuard() { free(m_Ptr); }

	public:
		operator wchar_t*() { return m_Ptr; }
		wchar_t *Ptr() { return m_Ptr; }
		wchar_t **PtrPtr() { return &m_Ptr; }
};

wchar_t* FormNtPath(const wchar_t* path, StrBuf& buf);
wchar_t* GetFullPath(const wchar_t* input, StrBuf& output);
wchar_t* ExpandEnvStrs(const wchar_t* input, StrBuf& output);
bool FindListFile(const wchar_t *FileName, StrBuf &output);

#endif /* __TMPPANEL_HPP__ */
