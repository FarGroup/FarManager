#ifndef STR_CLASS_INCLUDED
#define STR_CLASS_INCLUDED

class TStrList
{
	private:
		TCHAR **List;
		int Count;

	private:
		void DeleteList();

	public:
		TStrList();
		~TStrList();

	public:
		void __fastcall Clear();
		BOOL __fastcall Add(const TCHAR *String);
		BOOL __fastcall Insert(const TCHAR *String,int Index);
		BOOL __fastcall Delete(int Index);
		TCHAR *__fastcall GetText(TCHAR *String,int Index);
		TCHAR *__fastcall GetText(int Index);
		BOOL __fastcall SetText(const TCHAR *String,int Index);
		int GetCount() {return Count;};
		void __fastcall Sort(int Low,int Up);
		TStrList &operator=(TStrList &lst);
};

#endif //STR_CLASS_INCLUDED
