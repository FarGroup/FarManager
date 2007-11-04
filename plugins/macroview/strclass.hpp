#ifndef STR_CLASS_INCLUDED
#define STR_CLASS_INCLUDED

class TStrList;
/*class TStr
{
  friend class TStrList;

  TCHAR *string;
  TStr():string(NULL){}
};*/

class TStrList
{
  private:
//    TStr *List;
    TCHAR **List;
    int Count;
    void DeleteList();

  public:
    TStrList();
    ~TStrList();
    void __fastcall Clear();
    BOOL __fastcall Add(TCHAR *String);
    BOOL __fastcall Insert(TCHAR *String,int Index);
    BOOL __fastcall Delete(int Index);
    TCHAR *__fastcall GetText(TCHAR *String,int Index);
    TCHAR *__fastcall GetText(int Index);
    BOOL __fastcall SetText(TCHAR *String,int Index);
    int GetCount() {return Count;};
    void __fastcall Sort(int Low,int Up);
    TStrList &operator=(TStrList &lst);
//    TStrList &operator=(TCHAR *String);
};

#endif //STR_CLASS_INCLUDED
