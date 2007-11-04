#ifndef STR_CLASS_INCLUDED
#define STR_CLASS_INCLUDED

class TStrList;
/*class TStr
{
  friend class TStrList;

  char *string;
  TStr():string(NULL){}
};*/

class TStrList
{
  private:
//    TStr *List;
    char **List;
    int Count;
    void DeleteList();

  public:
    TStrList();
    ~TStrList();
    void __fastcall Clear();
    BOOL __fastcall Add(char *String);
    BOOL __fastcall Insert(char *String,int Index);
    BOOL __fastcall Delete(int Index);
    char *__fastcall GetText(char *String,int Index);
    char *__fastcall GetText(int Index);
    BOOL __fastcall SetText(char *String,int Index);
    int GetCount() {return Count;};
    void __fastcall Sort(int Low,int Up);
	char *__fastcall MultiStrToCrlfStr(char *DestStr,unsigned char *SrcStr,int SrcLen);
    TStrList &operator=(TStrList &lst);
//    TStrList &operator=(char *String);
};

#endif //STR_CLASS_INCLUDED
