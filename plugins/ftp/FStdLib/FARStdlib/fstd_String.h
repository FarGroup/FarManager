#ifndef __STRING
#define __STRING

#define DEF_STR_ALLOC 50

class String
{
		char *str;
		int   len,maxchar;

	private:
		void BeginSet(size_t start_sise = DEF_STR_ALLOC);

	public:
		virtual ~String();

		String(void);
		String(const String& s);
		String(LPCSTR s);
		String(size_t DefaultAllocateSize);

		char       *c_str(void)                           const { return str; }
		int         Length(void)                          const { return len; }

		LPCSTR    Alloc(LPCSTR s,int maxLen = -1);
		LPCSTR    Alloc(int NewAllocatedSize);
		void        SetLength(int ValueLessThenExistingLength);
		void        Null(void)                                  { SetLength(0); }

		LPCSTR    Set(LPCSTR s,
		              int StartToAdd_Inclusive,
		              int EndToAdd_Exclusive /*-1*/);
		LPCSTR    Set(LPCSTR s)                             { return Alloc(s); }
		LPCSTR    Set(const String& s)                        { return Alloc(s.c_str()); }

		int         printf(LPCSTR fmt,...);
		int         vprintf(LPCSTR fmt,va_list list);

		void        cat(LPCSTR s,...);
		void        vcat(LPCSTR s,va_list a);

		String&     Add(const String& s);
		String&     Add(LPCSTR s);
		String&     Add(char s);
		String&     Add(LPCSTR s, int StartToAdd_Inclusive, int EndToAdd_Exclusive /*-1*/);

		void        Del(int StartChar, int Count);
		void        DelChars(char AllCharsToDel = ' ');
		void        InsCharPos(int InsertPos,char ch);

		// Rets -1 if not found.
		int         Chr(char ch,int StartToSearchFrom = 0)     const;   //strchr
		int         Chr(LPCSTR set,int StartToSearchFrom = 0)const;   //search first char contains in set
		int         RChr(char ch,int EndToSearchAt = -1)       const;   //strrchr
		int         Str(LPCSTR ch,int pos = 0)               const;   //strstr

		String&     operator=(const String& s);
		String&     operator=(LPCSTR s);

		//Exact compare
		BOOL        operator!=(const String& s)         const;
		BOOL        operator!=(LPCSTR s)              const;
		BOOL        operator==(const String& s)         const;
		BOOL        operator==(LPCSTR s)              const;
		BOOL        Cmp(LPCSTR s,int count = -1, BOOL isCase = FALSE) const;

		char        operator[](int num)                 const;
		char        SetChar(int num,char ch);

		void        Trim(char ch = ' ');
		void        RTrim(char ch = ' ');
		void        LTrim(char ch = ' ');
};

#endif
