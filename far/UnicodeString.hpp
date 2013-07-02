#pragma once

/*
UnicodeString.hpp

Unicode строка
*/
/*
Copyright © 1996 Eugene Roshal
Copyright © 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "local.hpp"

const size_t __US_DELTA = 20;

class UnicodeStringData
{
	private:
		size_t m_nLength;
		size_t m_nSize;
		size_t m_nDelta;
		wchar_t *m_pData;
		wchar_t StackBuffer[__US_DELTA];

		wchar_t *AllocData(size_t nSize, size_t& nNewSize)
		{
			if (nSize <= m_nDelta)
				nNewSize = m_nDelta;
			else if (nSize%m_nDelta > 0)
				nNewSize = (nSize/m_nDelta + 1) * m_nDelta;
			else
				nNewSize = nSize;

			return nNewSize > __US_DELTA? new wchar_t[nNewSize] : StackBuffer;
		}

		void FreeData(wchar_t *pData)
		{
			if (pData != StackBuffer)
			{
				delete [] pData;
			}
		}

	public:
		UnicodeStringData(size_t nSize=0, size_t nDelta=0)
		{
			m_nDelta = nDelta? nDelta : __US_DELTA;
			m_nLength = 0;
			m_pData = AllocData(nSize, m_nSize);
			//“ак как ни где выше в коде мы не готовы на случай что пам€ти не хватит
			//то уж лучше и здесь не провер€ть а сразу падать
			*m_pData = 0;
		}

		size_t SetLength(size_t nLength)
		{
			//if (nLength<m_nSize) //Ёту проверку делает верхний класс, так что скажем что это оптимизаци€
			{
				m_nLength = nLength;
				m_pData[m_nLength] = 0;
			}
			return m_nLength;
		}

		void Inflate(size_t nSize)
		{
			if (nSize <= m_nSize)
				return;

			if (nSize >= m_nDelta << 3)
				nSize = nSize << 1;
			else
				nSize = (nSize/m_nDelta + 1) * m_nDelta;

			wchar_t *pOldData = m_pData;
			m_pData = AllocData(nSize, m_nSize);
			//“ак как ни где выше в коде мы не готовы на случай что пам€ти не хватит
			//то уж лучше и здесь не провер€ть а сразу падать
			wmemcpy(m_pData,pOldData,m_nLength);
			m_pData[m_nLength] = 0;
			FreeData(pOldData);
		}

		wchar_t *GetData() const { return m_pData; }
		size_t GetLength() const { return m_nLength; }
		size_t GetSize() const { return m_nSize; }

		~UnicodeStringData() { FreeData(m_pData); }
};

typedef class UnicodeString
{
	private:
		class char_proxy
		{
		public:
			operator wchar_t() const {return Parent.at(Index);}
			char_proxy& operator=(const char_proxy& Value)
			{
				Parent.GetBuffer()[Index] = Value.Parent[Value.Index];
				Parent.ReleaseBuffer(Parent.size());
				return *this;
			};

			char_proxy& operator=(wchar_t Value)
			{
				Parent.GetBuffer()[Index] = Value;
				Parent.ReleaseBuffer(Parent.size());
				return *this;
			}

		private:
			char_proxy(UnicodeString& Parent, size_t Index):Parent(Parent), Index(Index){}

			UnicodeString& Parent;
			size_t Index;
			friend class UnicodeString;
		};

		class us_iterator:public std::iterator<std::random_access_iterator_tag, wchar_t>
		{
		public:
			//us_iterator(){}
			us_iterator(char_proxy value):value(value) {}
			us_iterator(const us_iterator& rhs):value(rhs.value) {}

			char_proxy operator ->() const {return value;}
			char_proxy operator *() const {return value;}
			bool operator <(const us_iterator& rhs) const {return value.Index < rhs.value.Index;}
			bool operator >(const us_iterator& rhs) const {return value.Index > rhs.value.Index;}
			bool operator <=(const us_iterator& rhs) const {return value.Index <= rhs.value.Index;}
			bool operator >=(const us_iterator& rhs) const {return value.Index >= rhs.value.Index;}
			bool operator !=(const us_iterator& rhs) const {return value.Index != rhs.value.Index;}
			bool operator ==(const us_iterator& rhs) const {return value.Index == rhs.value.Index;}
			us_iterator& operator =(const us_iterator& rhs) {value.Index = rhs.value.Index; return *this;}
			us_iterator& operator ++() {++value.Index; return *this;}
			us_iterator& operator --() {--value.Index; return *this;}
			us_iterator operator ++(int) {auto ret = *this; ++value.Index; return ret;}
			us_iterator operator --(int) {auto ret = *this; --value.Index; return ret;}
			us_iterator& operator +=(size_t Offset) {value.Index += Offset; return *this;}
			us_iterator& operator -=(size_t Offset) {value.Index -= Offset; return *this;}
			us_iterator operator +(size_t Offset) const {auto ret = *this; ret.value.Index += Offset; return ret;}
			us_iterator operator -(size_t Offset) const {auto ret = *this; ret.value.Index -= Offset; return ret;}
			ptrdiff_t operator -(const us_iterator& rhs) const {return value.Index  -rhs.value.Index;}

		private:
			char_proxy value;
		};

		std::shared_ptr<UnicodeStringData> m_pData;

		void SetEUS();
		void Inflate(size_t nSize);
		UnicodeString& assign(const char *lpszData, uintptr_t CodePage=CP_OEMCP);

	public:
		UnicodeString() { SetEUS(); }
		UnicodeString(const UnicodeString &strCopy) { SetEUS(); assign(strCopy); }
		UnicodeString(UnicodeString&& rvalString):m_pData(rvalString.m_pData) { rvalString.SetEUS(); }
		UnicodeString(const wchar_t *lpwszData) { SetEUS(); assign(lpwszData); }
		UnicodeString(const wchar_t *lpwszData, size_t nLength) { SetEUS(); assign(lpwszData, nLength); }
		UnicodeString(const char *lpszData, uintptr_t CodePage=CP_OEMCP) { SetEUS(); assign(lpszData, CodePage); }
		explicit UnicodeString(size_t nSize, size_t nDelta=0) { m_pData = std::make_shared<DECLTYPE(m_pData)::element_type>(nSize, nDelta); }

		~UnicodeString() {}

		static const size_t npos = -1;

		typedef char_proxy value_type;
		typedef us_iterator iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef const wchar_t* const_iterator;
		typedef const std::reverse_iterator<const_iterator> const_reverse_iterator;

		iterator begin() {return char_proxy(*this, 0);}
		iterator end() {return char_proxy(*this, size());}

		const_iterator begin() const {return data();}
		const_iterator end() const {return data() + size();}

		const_iterator cbegin() const {return begin();}
		const_iterator cend() const {return end();}

		reverse_iterator rbegin() {return reverse_iterator(end());}
		reverse_iterator rend() {return reverse_iterator(begin());}

		const_reverse_iterator rbegin() const {return const_reverse_iterator(end());}
		const_reverse_iterator rend() const {return const_reverse_iterator(begin());}

		const_reverse_iterator crbegin() const {return rbegin();}
		const_reverse_iterator crend() const {return rend();}

		char_proxy front() {return *begin();}
		char_proxy back() {return *(end() - 1);}

		const wchar_t& front() const {return *begin();}
		const wchar_t& back() const {return *(end() - 1);}

		size_t capacity() const { return m_pData->GetSize(); }
		size_t size() const { return m_pData->GetLength(); }
		size_t length() const { return m_pData->GetLength(); }
		void resize(size_t nLength);

		wchar_t at(size_t nIndex) const { return m_pData->GetData()[nIndex]; }

		bool empty() const { return !(m_pData->GetLength() && *m_pData->GetData()); }

		void clear();

		const wchar_t *c_str() const { return m_pData->GetData(); }
		const wchar_t *data() const { return m_pData->GetData(); }

		UnicodeString substr(size_t Pos, size_t Len = npos) const;

		UnicodeString& operator=(const UnicodeString &strCopy) { return assign(strCopy); }
		UnicodeString& operator=(UnicodeString&& rvalString) { if (this != &rvalString) { m_pData.swap(rvalString.m_pData); rvalString.SetEUS(); } return *this; }
		UnicodeString& operator=(const char *lpszData) { return assign(lpszData); }
		UnicodeString& operator=(const wchar_t *lpwszData) { return assign(lpwszData); }
		UnicodeString& operator=(wchar_t chData) { return assign(chData); }

		UnicodeString& operator+=(const UnicodeString &strAdd) { return append(strAdd); }
		UnicodeString& operator+=(const char *lpszAdd) { return append(lpszAdd); }
		UnicodeString& operator+=(const wchar_t *lpwszAdd) { return append(lpwszAdd); }
		UnicodeString& operator+=(wchar_t chAdd) { return append(chAdd); }

		friend const UnicodeString operator+(const UnicodeString &strSrc1, const UnicodeString &strSrc2);
		friend const UnicodeString operator+(const UnicodeString &strSrc1, const wchar_t *lpwszSrc2);
		friend const UnicodeString operator+(const wchar_t *strSrc1, const UnicodeString &lpwszSrc2);

		bool operator==(const UnicodeString& str) const { return compare(str) == 0; }
		bool operator==(const wchar_t* s) const { return compare(s) == 0; }

		bool operator!=(const UnicodeString& str) const { return !(*this == str); }
		bool operator!=(const wchar_t* s) const { return !(*this == s); }

		bool operator<(const UnicodeString& str) const { return compare(str) < 0; }
		bool operator<(const wchar_t* s) const { return compare(s) < 0; }

		char_proxy operator[](size_t Index) { return char_proxy(*this, Index);}
		const wchar_t& operator[](size_t Index) const { return m_pData->GetData()[Index];}

		// TODO: iterator versions

		UnicodeString& assign(const UnicodeString &Str);
		// TODO
		// UnicodeString& Copy(const UnicodeString& str, size_t subpos, size_t sublen);
		UnicodeString& assign(const wchar_t *Str) { return assign(Str, StrLength(NullToEmpty(Str))); }
		UnicodeString& assign(const wchar_t *Str, size_t StrLen) { return replace(0, size(), Str, StrLen); }
		// TODO: size_t n
		UnicodeString& assign(/*size_t n, */wchar_t Ch) { return assign(&Ch, 1); }


		UnicodeString& replace(size_t Pos, size_t Len, const UnicodeString& Str) { return replace(Pos, Len, Str.data(), Str.size()); }
		// TODO
		// UnicodeString& replace(size_t pos,  size_t len, const UnicodeString& str, size_t subpos, size_t sublen);
		UnicodeString& replace(size_t Pos, size_t Len, const wchar_t* Str) { return replace(Pos, Len, Str, StrLength(NullToEmpty(Str))); }
		UnicodeString& replace(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen);
		// TODO: size_t n
		UnicodeString& replace(size_t Pos, size_t Len, /*size_t n, */wchar_t Ch) { return replace(Pos, Len, &Ch, 1); }


		UnicodeString& append(const UnicodeString& Str) { return append(Str.data(), Str.size()); }
		// TODO
		// UnicodeString& append(const UnicodeString& str, size_t subpos, size_t sublen);
		UnicodeString& append(const wchar_t* Str) { return append(Str, StrLength(NullToEmpty(Str))); }
		UnicodeString& append(const wchar_t* Str, size_t StrLen) { return replace(size(), 0, Str, StrLen); }
		// TODO: size_t n
		UnicodeString& append(/*size_t n, */wchar_t Ch) { return append(&Ch, 1); }


		UnicodeString& insert(size_t Pos, const UnicodeString& Str) { return insert(Pos, Str.data(), Str.size()); }
		// TODO
		// UnicodeString& insert(size_t pos, const UnicodeString& str, size_t subpos, size_t sublen);
		UnicodeString& insert(size_t Pos, const wchar_t* Str) { return insert(Pos, Str, wcslen(Str)); }
		UnicodeString& insert(size_t Pos, const wchar_t* Str, size_t StrLen) { return replace(Pos, 0, Str, StrLen); }
		// TODO: size_t n
		UnicodeString& insert(size_t Pos, /*size_t n, */wchar_t Ch) { return insert(Pos, &Ch, 1); }

		int compare(const UnicodeString& str) const { return compare(0, npos, str.data(), str.size()); }
		int compare(size_t pos, size_t len, const UnicodeString& str) const { return compare(pos, len, str.data(), str.size()); }
		// TODO
		// int compare(size_t pos, size_t len, const string& str, size_t subpos, size_t sublen) const;
		int compare(const wchar_t* s) const { return compare(0, npos, s, wcslen(s)); }
		int compare(size_t pos, size_t len, const wchar_t* s) const { return compare(pos, len, s, wcslen(s)); }
		int compare(size_t pos, size_t len, const wchar_t* s, size_t n) const;

		size_t find(const UnicodeString& str, size_t pos = 0) const { return find(str.data(), pos, str.size()); }
		size_t find(const wchar_t* s, size_t pos = 0) const { return find(s, pos, StrLength(s)); }
		size_t find(const wchar_t* s, size_t pos, size_t n) const {auto Iterator = std::search(cbegin() + pos, cend(), s, s + n); return Iterator != cend()? Iterator - cbegin() : npos;}
		size_t find(wchar_t c, size_t pos = 0) const { return find(&c, pos, 1); }

		size_t rfind(const UnicodeString& str, size_t pos = npos) const { return rfind(str.data(), pos, str.size()); };
		size_t rfind(const wchar_t* s, size_t pos = npos) const { return rfind(s, pos, StrLength(s)); };
		size_t rfind(const wchar_t* s, size_t pos, size_t n) const { pos = std::min(pos, size()); auto Iterator = std::find_end(cbegin(), cbegin() + pos, s, s + n); return Iterator != cend()? Iterator - cbegin() : npos;}
		size_t rfind(wchar_t c, size_t pos = npos) const { return rfind(&c, pos, 1); }

		// TODO: iterator & range versions
		UnicodeString& erase(size_t pos = 0, size_t len = npos) { return replace(pos, len, nullptr, 0); }

		void pop_back() { erase(size() - 1, 1); }

		wchar_t *GetBuffer(size_t nSize = npos);
		void ReleaseBuffer(size_t nLength = npos);
		int CDECL Format(const wchar_t * format, ...);
		UnicodeString& Lower(size_t nStartPos=0, size_t nLength=npos);
		UnicodeString& Upper(size_t nStartPos=0, size_t nLength=npos);
		bool PosI(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos=0) const;
} string;

inline wchar_t* UNSAFE_CSTR(const string& s) {return const_cast<wchar_t*>(s.data());}
