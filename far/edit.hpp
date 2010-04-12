#ifndef __EDIT_HPP__
#define __EDIT_HPP__
/*
edit.hpp

Реализация одиночной строки редактирования

*/

#include "scrobj.hpp"
#include "colors.hpp"
#include "bitflags.hpp"

//изменить флаги (подвинуть убрав FEDITLINE_CONVERTTABS)

// Младший байт (маска 0xFF) юзается классом ScreenObject!!!
enum FLAGS_CLASS_EDITLINE
{
	FEDITLINE_MARKINGBLOCK         = 0x00000100,
	FEDITLINE_DROPDOWNBOX          = 0x00000200,
	FEDITLINE_CLEARFLAG            = 0x00000400,
	FEDITLINE_PASSWORDMODE         = 0x00000800,
	FEDITLINE_EDITBEYONDEND        = 0x00001000,
	FEDITLINE_EDITORMODE           = 0x00002000,
	FEDITLINE_OVERTYPE             = 0x00004000,
	FEDITLINE_DELREMOVESBLOCKS     = 0x00008000,  // Del удаляет блоки (Opt.EditorDelRemovesBlocks)
	FEDITLINE_PERSISTENTBLOCKS     = 0x00010000,  // Постоянные блоки (Opt.EditorPersistentBlocks)
//  FEDITLINE_CONVERTTABS          = 0x00020000,
	FEDITLINE_READONLY             = 0x00040000,
	FEDITLINE_CURSORVISIBLE        = 0x00080000,
	// Если ни один из FEDITLINE_PARENT_ не указан (или указаны оба), то Edit
	// явно не в диалоге юзается.
	FEDITLINE_PARENT_SINGLELINE    = 0x00100000,  // обычная строка ввода в диалоге
	FEDITLINE_PARENT_MULTILINE     = 0x00200000,  // для будущего Memo-Edit (DI_EDITOR или DIF_MULTILINE)
	FEDITLINE_PARENT_EDITOR        = 0x00400000,  // "вверху" обычный редактор
};


class Dialog;
class Editor;

class Edit:public ScreenObject
{
		friend class DlgEdit;
		friend class Editor;

	public:
		Edit  *m_next;
		Edit  *m_prev;

	private:
		char  *Str;
		int    StrSize;
		int    MaxLength;

		char  *Mask;             // 12.08.2000 KM - Переменная для хранения маски ввода

		struct CharTableSet *TableSet;
		struct ColorItem *ColorList;
		int    ColorCount;

		int    Color;
		int    SelColor;
		int    ColorUnChanged;   // 28.07.2000 SVS - для диалога

		int    LeftPos;
		int    CurPos;
		int    PrevCurPos;       // 12.08.2000 KM - предыдущее положение курсора

		int    TabSize;          // 14.02.2001 IS - Размер табуляции - по умолчанию равен Opt.TabSize;

		int    TabExpandMode;

		int    MSelStart;
		int    SelStart;
		int    SelEnd;

		int    EndType;

		int    CursorSize;
		int    CursorPos;
		const char *WordDiv;

	private:
		virtual void   DisplayObject();
		int    InsertKey(int Key);
		int    RecurseProcessKey(int Key);
		void   DeleteBlock();
		void   ApplyColor();

		/* $ 12.08.2000 KM
		   Внутренняя функция которая парсит Mask и возвращает
		   следующее возможное положение курсора в строке ввода,
		   + функция обновляющая содержимое Str на основании Mask.
		*/
		int    GetNextCursorPos(int Position,int Where);
		void   RefreshStrByMask(int InitMode=FALSE);

		/* $ 15.11.2000 KM
		   Проверяет: попадает ли символ в разрешённый
		   диапазон символов, пропускаемых маской
		*/
		int KeyMatchedMask(int Key);

		int ProcessCtrlQ(void);
		int ProcessInsDate(const char *Fmt);
		int ProcessInsPlainText(const char *str);
		int CheckCharMask(char Chr);
		int ProcessInsPath(int Key,int PrevSelStart=-1,int PrevSelEnd=0);

	public:
		Edit(ScreenObject *pOwner = NULL);
		virtual ~Edit();

	public:
		void FastShow();
		virtual int ProcessKey(int Key);
		virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent);
		virtual __int64 VMProcess(int OpCode,void *vParam=NULL,__int64 iParam=0);

		void  SetObjectColor(int Color,int SelColor=0xf,int ColorUnChanged=COL_DIALOGEDITUNCHANGED);
		long  GetObjectColor() {return MAKELONG(Color,SelColor);}
		int   GetObjectColorUnChanged() {return ColorUnChanged;}

		void SetTabSize(int NewSize) { TabSize=NewSize; }
		int  GetTabSize(void) {return TabSize; }

		void SetDelRemovesBlocks(int Mode) {Flags.Change(FEDITLINE_DELREMOVESBLOCKS,Mode);}
		int  GetDelRemovesBlocks(void) {return Flags.Check(FEDITLINE_DELREMOVESBLOCKS); }

		void SetPersistentBlocks(int Mode) {Flags.Change(FEDITLINE_PERSISTENTBLOCKS,Mode);}
		int  GetPersistentBlocks(void) {return Flags.Check(FEDITLINE_PERSISTENTBLOCKS); }

		void  GetString(char *Str,int MaxSize);
		const char* GetStringAddr();
		void  SetString(const char *Str,int Length=-1);
		void  SetHiString(const char *Str);
		void  SetBinaryString(const char *Str,int Length);
		void  GetBinaryString(const char **Str,const char **EOL,int &Length);
		void  SetEOL(const char *EOL);
		const char *GetEOL(void);
		int   GetSelString(char *Str,int MaxSize);
		int   GetLength();
		void  InsertString(const char *Str);
		void  InsertBinaryString(const char *Str,int Length);
		int   Search(char *Str,int Position,int Case,int WholeWords,int Reverse);
		void  SetClearFlag(int Flag) {Flags.Change(FEDITLINE_CLEARFLAG,Flag);}
		int   GetClearFlag(void) {return Flags.Check(FEDITLINE_CLEARFLAG);}
		void  SetCurPos(int NewPos) {CurPos=NewPos; PrevCurPos=NewPos;}
		int   GetCurPos() {return(CurPos);}
		int   GetTabCurPos();
		void  SetTabCurPos(int NewPos);
		int   GetLeftPos() {return(LeftPos);}
		void  SetLeftPos(int NewPos) {LeftPos=NewPos;}
		void  SetPasswordMode(int Mode) {Flags.Change(FEDITLINE_PASSWORDMODE,Mode);};
		void  SetMaxLength(int Length) {MaxLength=Length;};
		int   GetMaxLength() {return MaxLength;};

		// $ 12.08.2000 KM - Функции установки и получения маски ввода
		void  SetInputMask(const char *InputMask);
		char* GetInputMask() {return Mask;}

		void  SetOvertypeMode(int Mode) {Flags.Change(FEDITLINE_OVERTYPE,Mode);};
		int   GetOvertypeMode() {return Flags.Check(FEDITLINE_OVERTYPE);};

		void  SetConvertTabs(int Mode) { TabExpandMode = Mode;};
		int   GetConvertTabs() {return TabExpandMode;};

		int   RealPosToTab(int Pos);
		int   TabPosToReal(int Pos);
		void  SetTables(struct CharTableSet *TableSet);
		void  Select(int Start,int End);
		void  AddSelect(int Start,int End);
		void  GetSelection(int &Start,int &End);
		BOOL  IsSelection() {return  SelStart==-1 && !SelEnd?FALSE:TRUE; };
		void  GetRealSelection(int &Start,int &End);
		void  SetEditBeyondEnd(int Mode) {Flags.Change(FEDITLINE_EDITBEYONDEND,Mode);};
		void  SetEditorMode(int Mode) {Flags.Change(FEDITLINE_EDITORMODE,Mode);};
		void  ReplaceTabs();

		void  InsertTab();

		void  AddColor(struct ColorItem *col);
		int   DeleteColor(int ColorPos);
		int   GetColor(struct ColorItem *col,int Item);

		void Xlat(BOOL All=FALSE);  // $ 24.09.2000 SVS - Функция Xlat - перекодировка по принципу QWERTY <-> ЙЦУКЕН
		static void DisableEncode(int Disable);
		static int GetEncodeState();

		void SetDialogParent(DWORD Sets);
		void SetCursorType(int Visible,int Size);
		void GetCursorType(int &Visible,int &Size);
		int  GetReadOnly() {return Flags.Check(FEDITLINE_READONLY);}
		void SetReadOnly(int NewReadOnly) {Flags.Change(FEDITLINE_READONLY,NewReadOnly);}
		int  GetDropDownBox() {return Flags.Check(FEDITLINE_DROPDOWNBOX);}
		void SetDropDownBox(int NewDropDownBox) {Flags.Change(FEDITLINE_DROPDOWNBOX,NewDropDownBox);}
		void SetWordDiv(const char *WordDiv) {Edit::WordDiv=WordDiv;}
};

#endif  // __EDIT_HPP__
