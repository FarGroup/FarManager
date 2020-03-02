#ifndef __FAR_PLUGINS_UTILITES
#define __FAR_PLUGINS_UTILITES

/** @mainpage FAR Standart Library
    @author Jouri Mamaev JouriM@uran.ru
    @Eng

    @section intro Introduction
      C++ library for easy create plugins for file and archive manager FAR. <br>
      Contains set of classes and functions designed to decrease time for write all types of FAR plugins.

      Initialy created by JouriM@uran.ru

    @section Structure Document structure
      This documentation structure:
       - FarStdLib    Full documentation tree;
       - Consts       Different constants and defines;
       - Types        Data types;
       - Functions    Global functions;
       - Variables    Global variables;

    @section Add Additional helpers.
      Additional helpers andd classes does not relatie to FAR API directly but used by other wrapers or
      incapsulate usefull functionality.

      - FP_Screen - helper for save and restore FAR screen buffer. <br>
                    Class for save and restore whole screen buffer.
                    Can be used in nested calls. In this case the first call save screen and
                    restored after last FP_Screen object destroyed.
      - FPOpMode  - helper for save panel plugin OpMode value.
      - SRect     - helper class to incapsulate Win API SMALL_RECT structure with additional public
                    methods to manipulate its contents.
*/

/** @defgroup FSTDLib Library Compilation/Porting

    FARStdLibrary already ported to next compillers:
      - Borland C compiller version 5.xx
      - Visual C compiller version 6.xx
      - Symantec  compiller version 7.2
      - GCC
*/

#define _FAR_USE_FARFINDDATA 1

#include <plugin.hpp>
#include <farcolor.hpp>
#include <farkeys.hpp>

#ifdef __GNUC__
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

// --------------------------------------------------------------
#include <FARStdlib/funi.h>

// ------------------------------------------------------------------------
#if defined(__FILELOG__)
#define Log(v)  FARINProc::Say v
#define PROC(v) FARINProc _inp v ;
#else
#define PROC(v)
#define Log(v)
#endif


// ------------------------------------------------------------------------
/** [fstd_menu.cpp]
    @brief
*/
#if !defined(__FP_NOT_FUNCTIONS__)
struct FMenuItem: public FarMenuItem
{
	FMenuItem(LPCSTR txt,bool sel = false,char ch = 0)      { Assign(txt,sel,ch); }
	FMenuItem(const FMenuItem& p)                             { Assign(p); }
	FMenuItem(void)                                           { Assign(); }

	void Assign(LPCSTR txt,bool sel = false,char ch = 0);
	void Assign(const FMenuItem& p);
	void Assign(void);

	BOOL isChecked(void)     { return Checked; }
	void isChecked(BOOL v)   { Checked = v; }
	BOOL isSeparator(void)   { return Separator; }
	void isSeparator(BOOL v) { Separator = v; }
	BOOL isSelected(void)    { return Selected; }
	void isSelected(BOOL v)  { Selected = v; }
};

struct FMenuItemEx: public FarMenuItemEx
{
	FMenuItemEx(LPCSTR txt,bool sel = false,char ch = 0)      { Assign(txt,sel,ch); }
	FMenuItemEx(const FMenuItemEx& p)                           { Assign(p); }
	FMenuItemEx(void)                                           { Assign(); }

	void Assign(LPCSTR txt,bool sel = false,char ch = 0);
	void Assign(const FMenuItemEx& p);
	void Assign(void);

	BOOL isChecked(void)     { return IS_FLAG(Flags,MIF_CHECKED); }
	void isChecked(BOOL v)   { if(v) SET_FLAG(Flags,MIF_CHECKED); else CLR_FLAG(Flags,MIF_CHECKED); }
	BOOL isSeparator(void)   { return IS_FLAG(Flags,MIF_SEPARATOR); }
	void isSeparator(BOOL v) { if(v) SET_FLAG(Flags,MIF_SEPARATOR); else CLR_FLAG(Flags,MIF_SEPARATOR); }
	BOOL isSelected(void)    { return IS_FLAG(Flags,MIF_SELECTED); }
	void isSelected(BOOL v)  { if(v) SET_FLAG(Flags,MIF_SELECTED); else CLR_FLAG(Flags,MIF_SELECTED); }
	BOOL isGrayed(void)      { return IS_FLAG(Flags,MIF_DISABLE); }
	void isGrayed(BOOL v)    { if(v) SET_FLAG(Flags,MIF_DISABLE); else CLR_FLAG(Flags,MIF_DISABLE); }
};

// ------------------------------------------------------------------------
/** @brief  FP_Menu
    [fstd_menu.cpp]
*/
template <class T> class FP_MenuTyped
{
		T   *List;
		int  ItemsCount;
		int  MaxCount;

	private:
		BOOL Realloc(int DeltaSize);

	public:
		FP_MenuTyped(void);
		FP_MenuTyped(LPCSTR strings[],int SelNum = -1,int CheckNum = -1,char CheckChar = 0);
		~FP_MenuTyped() { Free(); }

		T    *Add(const T *src,int cn);                         //Add a `cn` items to list
		T    *Add(const T& p)           { return Add(&p,1); }   //Add 1 item to list
		void  Free(void);                                       //Clear list
		void  DeleteAll(void)           { Free(); }             //Clear list

		T    *Items(void)               { return List; }
		int   Count(void)               { return ItemsCount; }
		T    *Item(int num)             { return (num >= 0 && num < ItemsCount) ? (List+num) : NULL; }
		T    *operator[](int num)       { return Item(num); }

		int Execute(LPCSTR Title,DWORD Type = 0,LPCSTR Footer = NULL,
		            LPCSTR Help = NULL,
		            const int *BreakKeys = NULL, int *BreakCode = NULL);
};

/**/template <class T> FP_MenuTyped<T>::FP_MenuTyped(void)
/**/
{
	/**/    List       = NULL;
	/**/    ItemsCount = 0;
	/**/    MaxCount   = 0;
	/**/
}
/**/
/**/template <class T> FP_MenuTyped<T>::FP_MenuTyped(LPCSTR strings[],int SelNum,int CheckNum,char CheckChar)
/**/
{
	int n,cn;
	/**/     T  *it;
	/**/
	/**/    List       = NULL;
	/**/    ItemsCount = 0;
	/**/    MaxCount   = 0;

	/**/

	/**/    if(!strings) return;

	/**/

	/**/    for(cn = n = 0; strings[n]; cn++,n++);

	/**/

	/**/    if(!cn) return;

	/**/
	/**/    Realloc(cn);

	/**/    for(n = 0; n < cn; n++)
	{
		/**/      it = Add(T());
		/**/      it->Assign(strings[n],SelNum == n,CheckNum == n ? CheckChar : 0);
		/**/
	}

	/**/

}
/**/
/**/template <class T> BOOL FP_MenuTyped<T>::Realloc(int NewSize)
/**/
{
	/**/    if(!NewSize)
		/**/      return FALSE;

	/**/

	/**/    if(NewSize < MaxCount)
		/**/      return TRUE;

	/**/    MaxCount = NewSize;

	/**/

	/**/    if(!List)
		/**/      List = (T*)malloc(sizeof(T)*MaxCount);
	/**/     else
		/**/      List = (T*)realloc(List,sizeof(T)*MaxCount);

	/**/

	/**/    if(!List)
		/**/      return FALSE;

	/**/
	/**/ return TRUE;
	/**/
}
/**/
/**/template <class T> T *FP_MenuTyped<T>::Add(const T *pi,int icn)
/**/
{
	/**/    if(!Realloc(ItemsCount+icn))
		/**/      return NULL;

	/**/
	/**/    T *p = List + ItemsCount;
	/**/    memmove(p,pi,sizeof(*p)*icn);
	/**/    ItemsCount += icn;
	/**/
	/**/ return p;
	/**/
}
/**/
/**/template <class T> void FP_MenuTyped<T>::Free(void)
/**/
{
	/**/    if(!ItemsCount)
		/**/      return;

	/**/
	/**/    free(List);
	/**/    List       = NULL;
	/**/    ItemsCount = 0;
	/**/    MaxCount   = 0;
	/**/
}
/**/
/**/template <class T> int FP_MenuTyped<T>::Execute(LPCSTR Title,DWORD Type,
        /**/                                            LPCSTR Footer,LPCSTR Help,
        /**/                                            const int *BreakKeys,int *BreakCode)
/**/
{
	/**/ return FP_Info->Menu(FP_Info->ModuleNumber, -1, -1, 0,
	                          /**/                       Type | (sizeof(T) != sizeof(FarMenuItem) ? FMENU_USEEXT : 0),
	                          /**/ (const char *)FP_GetMsg(Title),
	                          /**/ (const char *)FP_GetMsg(Footer),
	                          /**/ (const char *)Help,
	                          /**/                       BreakKeys, BreakCode,
	                          /**/ (const struct FarMenuItem*)Items(), Count());
	/**/
}

typedef FP_MenuTyped<FMenuItem>    FP_Menu;
typedef FP_MenuTyped<FMenuItem>   *PFP_Menu;

typedef FP_MenuTyped<FMenuItemEx>  FP_MenuEx;
typedef FP_MenuTyped<FMenuItemEx> *PFP_MenuEx;

// ------------------------------------------------------------------------
/** @brief FP_Dialog
    [fstd_Dialog.cpp]
*/
struct FP_Dialog
{
		HANDLE Handle;
		int    LockCount;
	public:
		FP_Dialog(HANDLE h);

		void Assign(HANDLE h) { Handle = h; }

		int      Focused(void)                      const { return (int)FP_Info->SendDlgMessage(Handle,DM_GETFOCUS,0,0); }
		void     Focused(int num)                   const { FP_Info->SendDlgMessage(Handle,DM_SETFOCUS,num,0); }
		int      Enabled(int num, int v = -1)       const { return (int)FP_Info->SendDlgMessage(Handle,DM_ENABLE,num,v); }
		BOOL     Visible(int num, int v = -1)       const { return (BOOL)FP_Info->SendDlgMessage(Handle,DM_SHOWITEM,num,v); }

		bool     Checked(int num)                   const { return FP_Info->SendDlgMessage(Handle,DM_GETCHECK,num,0) == BSTATE_CHECKED; }
		void     Checked(int num, bool v)           const { FP_Info->SendDlgMessage(Handle,DM_SETCHECK,num,v ? BSTATE_CHECKED : BSTATE_UNCHECKED); }
		void     CheckTogle(int num)                const { Checked(num,!Checked(num)); }

		bool     GetItem(int num, FarDialogItem* p) const { return FP_Info->SendDlgMessage(Handle,DM_GETDLGITEM,num,(LONG_PTR)p) == TRUE; }
		bool     SetItem(int num, FarDialogItem* p) const { return FP_Info->SendDlgMessage(Handle,DM_SETDLGITEM,num,(LONG_PTR)p) == TRUE; }

		bool     DlgRect(SRect* p)                  const { return FP_Info->SendDlgMessage(Handle,DM_GETDLGRECT,0,(LONG_PTR)p) == TRUE; }
		bool     ItemRect(int num,SRect* p)         const { return FP_Info->SendDlgMessage(Handle,DM_GETITEMPOSITION,num,(LONG_PTR)p) == TRUE; }
		int      CursorPos(int num)                 const { COORD cp= {0,0}; return FP_Info->SendDlgMessage(Handle,DM_GETCURSORPOS,num,(LONG_PTR)&cp) == TRUE ? cp.X : 0; }
		bool     CursorPos(int num,int pos) const;

		void     Lock(bool v = true);
		void     Unlock(void)                             { Lock(false); }
		bool     Locked(void)                       const { return LockCount != 0; }
		void     FullUnlock(void);

		void     Close(int id = 0)                  const { FP_Info->SendDlgMessage(Handle,DM_CLOSE,id,0); }
		void     Invalidate(void)                   const { FP_Info->SendDlgMessage(Handle,DM_REDRAW,0,0); }
		void     Visible(bool v)                    const { FP_Info->SendDlgMessage(Handle,DM_SHOWDIALOG,(BOOL)v,0); }
		LONG_PTR User(int msg,int p=0,LONG_PTR p1=0)  const { return FP_Info->SendDlgMessage(Handle,msg,p,p1); }

		int      SetText(int num, LPCSTR str,int sz = 0) const;
		LPCSTR GetText(int num)                      const;
		int      GetText(int num,char *buff,int bSz)   const;
};
#endif // !defined(__FP_NOT_FUNCTIONS__)

// ------------------------------------------------------------------------
/** @brief Dynamic array of panel item elements.
    [fstd_ilist.cpp]

    PluginPanelItem Reserved and can be used to store additional data.
    This data will be correctly copyed and deleted in FP_ItemList.
    You MUST use _Alloc or strdup to allocate data space.
    Data CAT NOT BE zero sized.
*/
inline BOOL   FPIL_ADDEXIST(const PluginPanelItem *p) { return ((p)->Reserved[0] && (p)->Reserved[1]); }
inline DWORD  FPIL_ADDSIZE(const PluginPanelItem *p)  { return FPIL_ADDEXIST(p) ? (DWORD)(p)->Reserved[0] : 0; }
inline LPVOID FPIL_ADDDATA(const PluginPanelItem *p)  { return FPIL_ADDEXIST(p) ? ((void*)(p)->Reserved[1]) : NULL; }
inline void   FPIL_ADDSET(PluginPanelItem *p, DWORD sz, LPVOID dt) { (p)->Reserved[0] = sz; (p)->Reserved[1] = (DWORD_PTR)dt; }

struct FP_ItemList
{
		PluginPanelItem *List;         ///< Panel items array
		int              ItemsCount;   ///< Number of items in array
		BOOL             needToDelete; ///< Indicate if local copy of items array need to be deleted at destructor
		int              MaxCount;     ///< Number of elements may be inserted into list without expand list
	private:
		BOOL Realloc(int DeltaSize);
	public:
		FP_ItemList(BOOL NeedToDelete = TRUE);
		~FP_ItemList() { Free(); }

		PluginPanelItem *Add(const PluginPanelItem *src,int cn);                           ///<Add a `cn` items to list
		PluginPanelItem *Add(const PluginPanelItem *src)  { return Add(src,1); }
		void             Free(void);                                                       //Clear list

		PluginPanelItem *Items(void)                      { return List; }
		int              Count(void)                      { return ItemsCount; }
		PluginPanelItem *Item(int num)                    { return (num >= 0 && num < ItemsCount) ? (List+num) : NULL; }
		PluginPanelItem *operator[](int num)              { return Item(num); }

		static void      Free(PluginPanelItem *List,int count);                            //Free items, allocated by FP_ItemList
		static void      Copy(PluginPanelItem *dest,const PluginPanelItem *src,int cn);    //Copy all data from `src` to `dest`
};

struct FP_SizeItemList: public FP_ItemList
{
		__int64 TotalFullSize;
		__int64 TotalFiles;
	public:
		FP_SizeItemList(BOOL NeedToDelete = TRUE) : FP_ItemList(NeedToDelete)
		{
			TotalFullSize = 0;
			TotalFiles    = 0;
		}
};

#ifdef __GNUC__
#pragma pack()
#else
#pragma pack(pop)
#endif

#endif
