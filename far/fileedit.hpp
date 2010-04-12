#ifndef __FILEEDITOR_HPP__
#define __FILEEDITOR_HPP__
/*
fileedit.hpp

Редактирование файла - надстройка над editor.cpp

*/

#include "frame.hpp"
#include "editor.hpp"
#include "keybar.hpp"

class NamesList;

// коды возврата Editor::SaveFile()
enum
{
	SAVEFILE_ERROR   = 0,         // пытались сохранять, не получилось
	SAVEFILE_SUCCESS = 1,         // либо успешно сохранили, либо сохранять было не надо
	SAVEFILE_CANCEL  = 2          // сохранение отменено, редактор не закрывать
};

// как открывать
enum FEOPMODEEXISTFILE
{
	FEOPMODE_QUERY        =0x00000000,
	FEOPMODE_NEWIFOPEN    =0x10000000,
	FEOPMODE_USEEXISTING  =0x20000000,
	FEOPMODE_BREAKIFOPEN  =0x30000000,
	FEOPMODE_RELOAD       =0x40000000,
};

enum FFILEEDIT_FLAGS
{
	FFILEEDIT_NEW                   = 0x00010000,  // Этот файл СОВЕРШЕННО! новый или его успели стереть! Нету такого и все тут.
	FFILEEDIT_REDRAWTITLE           = 0x00020000,  // Нужно редравить заголовок?
	FFILEEDIT_FULLSCREEN            = 0x00040000,  // Полноэкранный режим?
	FFILEEDIT_DISABLEHISTORY        = 0x00080000,  // Запретить запись в историю?
	FFILEEDIT_ENABLEF6              = 0x00100000,  // Переключаться во вьювер можно?
	FFILEEDIT_SAVETOSAVEAS          = 0x00200000,  // $ 17.08.2001 KM  Добавлено для поиска по AltF7.
	//   При редактировании найденного файла из архива для
	//   клавиши F2 сделать вызов ShiftF2.
	FFILEEDIT_SAVEWQUESTIONS        = 0x00400000,  // сохранить без вопросов
	FFILEEDIT_LOCKED                = 0x00800000,  // заблокировать?

	FFILEEDIT_DELETEONCLOSE         = 0x01000000,  // 10.10.2001 IS: Если TRUE, то удалить в деструкторе файл вместе с каталогом (если тот пуст)
	FFILEEDIT_DELETEONLYFILEONCLOSE = 0x02000000,  // Если флаг взведен и нет FFILEEDIT_DELETEONCLOSE, то удалить только файл

	FFILEEDIT_CANNEWFILE            = 0x10000000,  // допускается новый файл?
	FFILEEDIT_SERVICEREGION         = 0x20000000,  // используется сервисная область
	FFILEEDIT_OPENFAILED            = 0x40000000,  // ошибки при открытии файла
};


class FileEditor:public Frame
{
	private:
		typedef class Frame inherited;

		Editor *FEdit;
		KeyBar EditKeyBar;

		NamesList *EditNamesList;

		char FileName[NM*2];
		char FullFileName[NM*2];
		char StartDir[NM];
		char NewTitle[NM];

		char Title[512];
		char PluginTitle[512];
		char PluginData[NM*2];

		WIN32_FIND_DATA FileInfo;

		char AttrStr[4];               // 13.02.2001 IS - Сюда запомним буквы атрибутов, чтобы не вычислять их много раз
		DWORD FileAttributes;          // 12.02.2001 IS - сюда запомним атрибуты файла при открытии, пригодятся где-нибудь...
		BOOL  FileAttributesModified;  // 04.11.2003 SKV - надо ли восстанавливать аттрибуты при save

		DWORD SysErrorCode;

		bool bClosing;                 // 28.04.2005 AY: true когда редактор закрываеться (т.е. в деструкторе)

		bool bEE_READ_Sent;

	public:
		FileEditor(const char *Name,DWORD InitFlags,int StartLine=-1,int StartChar=-1,char *PluginData=NULL,int OpenModeExstFile=FEOPMODE_QUERY);
		/* $ 14.06.2002 IS
		   DeleteOnClose стал int:
		     0 - не удалять ничего
		     1 - удалять файл и каталог
		     2 - удалять только файл
		*/
		FileEditor(const char *Name,DWORD InitFlags,int StartLine,int StartChar,const char *Title,int X1,int Y1,int X2,int Y2,int DeleteOnClose=0,int OpenModeExstFile=FEOPMODE_QUERY);

		virtual ~FileEditor();

	private:
		virtual void DisplayObject();
		int  ProcessQuitKey(int FirstSave,BOOL NeedQuestion=TRUE);
		BOOL UpdateFileList();
		/* $ 10.10.2001 IS установка DeleteOnClose */
		/* $ 14.06.2002 IS
		    DeleteOnClose стал int:
		      0 - не удалять ничего
		      1 - удалять файл и каталог
		      2 - удалять только файл
		*/
		void SetDeleteOnClose(int NewMode);
		/* IS 14.06.2002 */
		/* IS 10.10.2001 */
		int ReProcessKey(int Key,int CalledFromControl=TRUE);

	public:
		/* $ 14.06.2002 IS
		   DeleteOnClose стал int:
		     0 - не удалять ничего
		     1 - удалять файл и каталог
		     2 - удалять только файл
		*/
		void Init(const char *Name,const char *Title,DWORD InitFlags,int StartLine,int StartChar,char *PluginData,int DeleteOnClose,int OpenModeExstFile);
		/* IS $ */

		virtual void InitKeyBar(void);                    // $ 07.08.2000 SVS - Функция инициализации KeyBar Labels
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);
		virtual void ShowConsoleTitle();
		int IsFileChanged() {return(FEdit->IsFileChanged());};
		virtual int IsFileModified() {return(FEdit->IsFileModified());};
		virtual void OnChangeFocus(int focus); // вызывается при смене фокуса

		virtual void SetScreenPosition();                 // $ 28.06.2000 tran - NT Console resize - resize editor

		virtual int GetTypeAndName(char *Type,char *Name);
		virtual const char *GetTypeName() {return "[FileEdit]";};
		virtual int GetType() { return MODALTYPE_EDITOR; }


		virtual void OnDestroy();                         // $ 10.05.2001 DJ
		void SetNamesList(NamesList *Names);              // $ 07.05.2001 DJ

		virtual int GetCanLoseFocus(int DynamicMode=FALSE);

		virtual int FastHide();                           // $ OT - Введена для нужд CtrlAltShift

		void SetEnableF6(int AEnableF6) { Flags.Change(FFILEEDIT_ENABLEF6,AEnableF6); InitKeyBar(); }   // $ 10.05.2001 DJ
		/* $ 17.08.2001 KM
		  Добавлено для поиска по AltF7. При редактировании найденного файла из
		  архива для клавиши F2 сделать вызов ShiftF2.
		*/
		void SetSaveToSaveAs(int ToSaveAs) { Flags.Change(FFILEEDIT_SAVETOSAVEAS,ToSaveAs); InitKeyBar(); }
		/* KM $ */

		/* $ 08.12.2001 OT
		  возвращает признак того, является ли файл временным
		  используется для принятия решения переходить в каталог по CtrlF10*/
		BOOL isTemporary();
		virtual void ResizeConsole();
		virtual void Show();

		int LoadFile(const char *Name,int &UserBreak);
		int SaveFile(const char *Name,int Ask,int TextFormat,int SaveAs);
		int EditorControl(int Command,void *Param);
		void SetPluginTitle(const char *PluginTitle);
		void SetTitle(const char *Title);
		virtual const char *GetTitle(char *Title,int LenTitle,int TruncSize=0);
		BOOL SetFileName(const char *NewFileName);
		int ProcessEditorInput(INPUT_RECORD *Rec);
		void SetLockEditor(BOOL LockMode);
		BOOL IsFullScreen() {return Flags.Check(FFILEEDIT_FULLSCREEN);}
		void ChangeEditKeyBar();
		void ShowStatus();

		DWORD GetFileAttributes(LPCTSTR);                 // $ 13.02.2001 IS - Обертка вокруг одноименной функции из win32 api

		void SetPluginData(char *PluginData);
		char *GetPluginData(void) {return PluginData;};

		bool LoadFromCache(EditorCacheParams *pp);
		void SaveToCache();

		void GetEditorOptions(struct EditorOptions& EdOpt);
		void SetEditorOptions(struct EditorOptions& EdOpt);
};

#endif  // __FILEEDITOR_HPP__
