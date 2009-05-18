#ifndef __TASKBAR_HPP__
#define __TASKBAR_HPP__

/*
TaskBar.hpp

Windows 7 taskbar support
*/

#ifndef __ITaskbarList3_INTERFACE_DEFINED__
#define __ITaskbarList3_INTERFACE_DEFINED__

typedef enum TBPFLAG
{
	TBPF_NOPROGRESS    = 0,
	TBPF_INDETERMINATE = 0x1,
	TBPF_NORMAL        = 0x2,
	TBPF_ERROR         = 0x4,
	TBPF_PAUSED        = 0x8
}
TBPFLAG;

typedef enum TBATFLAG
{
	TBATF_USEMDITHUMBNAIL   = 0x1,
	TBATF_USEMDILIVEPREVIEW = 0x2
}
TBATFLAG;

EXTERN_C const IID IID_ITaskbarList3;

#if defined(__GNUC__) || defined(__BORLANDC__)
DECLARE_INTERFACE_(ITaskbarList3,IUnknown) //BUGBUG, ITaskbarList2
#else
MIDL_INTERFACE("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf") ITaskbarList3 : public ITaskbarList2
#endif
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetProgressValue(HWND hwnd,ULONGLONG ullCompleted,ULONGLONG ullTotal)=0;
	virtual HRESULT STDMETHODCALLTYPE SetProgressState(HWND hwnd,TBPFLAG tbpFlags)=0;
	virtual HRESULT STDMETHODCALLTYPE RegisterTab(HWND hwndTab,HWND hwndMDI)=0;
	virtual HRESULT STDMETHODCALLTYPE UnregisterTab(HWND hwndTab)=0;
	virtual HRESULT STDMETHODCALLTYPE SetTabOrder(HWND hwndTab,HWND hwndInsertBefore)=0;
	virtual HRESULT STDMETHODCALLTYPE SetTabActive(HWND hwndTab,HWND hwndMDI,TBATFLAG tbatFlags)=0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons(HWND hwnd,UINT cButtons,/*LPTHUMBBUTTON*/LPVOID pButton)=0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons(HWND hwnd,UINT cButtons,/*LPTHUMBBUTTON*/LPVOID pButton)=0;
	virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList(HWND hwnd,HIMAGELIST himl)=0;
	virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon(HWND hwnd,HICON hIcon,LPCWSTR pszDescription)=0;
	virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip(HWND hwnd,LPCWSTR pszTip)=0;
	virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip(HWND hwnd,RECT *prcClip)=0;
};

#endif // __ITaskbarList3_INTERFACE_DEFINED__

#if !defined(__BORLANDC__)
typedef BOOL (WINAPI *FLASHWINDOWEX)(PFLASHWINFO pfwi);
#endif

class TaskBarCore
{
	bool CoInited;
	TBPFLAG State;
	ITaskbarList3* pTaskbarList;
public:
	TaskBarCore();
	~TaskBarCore();
	TBPFLAG GetProgressState();
	void SetProgressState(TBPFLAG tbpFlags);
	void SetProgressValue(UINT64 Completed, UINT64 Total);
	void Flash();
};

extern TaskBarCore TBC;

class TaskBar
{
public:
	TaskBar();
	~TaskBar();
};

class TaskBarPause
{
	TBPFLAG PrevState;
public:
	TaskBarPause();
	~TaskBarPause();
};

class TaskBarError
{
	TBPFLAG PrevState;
public:
	TaskBarError();
	~TaskBarError();
};

#endif //__TASKBAR_HPP__
