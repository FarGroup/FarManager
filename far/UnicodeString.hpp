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



typedef class UnicodeString
{
	class UnicodeStringData
	{
	private:
		static const size_t __US_DELTA = 20;
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

	class char_proxy
	{
	public:
		char_proxy(const char_proxy& rhs):Parent(rhs.Parent), Index(rhs.Index){}
		operator const wchar_t&() const {return Parent->data()[Index];}
		const wchar_t* operator &() const {return & operator const wchar_t &();}
		char_proxy& operator=(const char_proxy& rhs) {return *this = rhs.operator const wchar_t&();}
		char_proxy& operator=(wchar_t Value)
		{
			Parent->GetBuffer()[Index] = Value;
			Parent->ReleaseBuffer(Parent->size());
			return *this;
		}

	private:
		char_proxy(UnicodeString* Parent, size_t Index):Parent(Parent), Index(Index){}

		UnicodeString* Parent;
		size_t Index;
		friend class UnicodeString;
	};

	class us_iterator:public std::iterator<std::random_access_iterator_tag, char_proxy, ptrdiff_t, char_proxy*, char_proxy>
	{
	public:
		us_iterator():value(value_type(nullptr, npos)) {}
		us_iterator(const value_type& value):value(value) {}
		reference operator *() const {return value;}
		bool operator <(const us_iterator& rhs) const {return value.Index < rhs.value.Index;}
		bool operator >(const us_iterator& rhs) const {return value.Index > rhs.value.Index;}
		bool operator <=(const us_iterator& rhs) const {return value.Index <= rhs.value.Index;}
		bool operator >=(const us_iterator& rhs) const {return value.Index >= rhs.value.Index;}
		bool operator !=(const us_iterator& rhs) const {return value.Index != rhs.value.Index;}
		bool operator ==(const us_iterator& rhs) const {return value.Index == rhs.value.Index;}
		us_iterator& operator ++() {++value.Index; return *this;}
		us_iterator& operator --() {--value.Index; return *this;}
		us_iterator operator ++(int) {auto ret = *this; ++value.Index; return ret;}
		us_iterator operator --(int) {auto ret = *this; --value.Index; return ret;}
		us_iterator& operator +=(size_t Offset) {value.Index += Offset; return *this;}
		us_iterator& operator -=(size_t Offset) {value.Index -= Offset; return *this;}
		us_iterator operator +(size_t Offset) const {auto ret = *this; ret.value.Index += Offset; return ret;}
		us_iterator operator -(size_t Offset) const {auto ret = *this; ret.value.Index -= Offset; return ret;}
		difference_type operator -(const us_iterator& rhs) const {return value.Index - rhs.value.Index;}

	private:
		value_type value;
	};

public:
	UnicodeString() { SetEUS(); }
	UnicodeString(const UnicodeString &str) { SetEUS(); assign(str); }
	UnicodeString(const wchar_t *s) { SetEUS(); assign(s); }
	UnicodeString(const wchar_t *s, size_t n) { SetEUS(); assign(s, n); }
	UnicodeString(UnicodeString&& str):m_pData(str.m_pData) { str.SetEUS(); }

	~UnicodeString() {}

	static const size_t npos = -1;

	typedef us_iterator iterator;
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef const wchar_t* const_iterator;
	typedef const std::reverse_iterator<const_iterator> const_reverse_iterator;
	typedef iterator::value_type value_type;

	iterator begin() {return iterator::value_type(this, 0);}
	iterator end() {return iterator::value_type(this, size());}

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

	iterator::value_type front() {return *begin();}
	iterator::value_type back() {return *rbegin();}

	const wchar_t& front() const {return *begin();}
	const wchar_t& back() const {return *rbegin();}

	iterator::value_type at(size_t pos) {return iterator::value_type(this, pos);}
	const wchar_t& at(size_t pos) const {return m_pData->GetData()[pos];}

	iterator::value_type operator[](size_t pos) {return at(pos);}
	const wchar_t& operator[](size_t pos) const {return at(pos);}

	size_t capacity() const { return m_pData->GetSize(); }
	size_t size() const { return m_pData->GetLength(); }
	size_t length() const { return size(); }
	void resize(size_t nLength);

	bool empty() const {return !size();}

	void clear();

	const wchar_t *data() const { return m_pData->GetData(); }

	UnicodeString substr(size_t pos, size_t ln = npos) const;

	UnicodeString& operator=(const UnicodeString& str) { return assign(str); }
	UnicodeString& operator=(UnicodeString&& str) { if (this != &str) { m_pData.swap(str.m_pData); str.SetEUS(); } return *this; }
	UnicodeString& operator=(const wchar_t* s) { return assign(s); }
	UnicodeString& operator=(wchar_t c) { return assign(1, c); }

	UnicodeString& operator+=(const UnicodeString& str) { return append(str); }
	UnicodeString& operator+=(const wchar_t* s) { return append(s); }
	UnicodeString& operator+=(wchar_t c) { return append(1, c); }

	friend const UnicodeString operator+(const UnicodeString &lhs, const UnicodeString &rhs);
	friend const UnicodeString operator+(const UnicodeString &lhs, const wchar_t *rhs);
	friend const UnicodeString operator+(const wchar_t *lhs, const UnicodeString &rhs);

	bool operator==(const UnicodeString& str) const { return compare(str) == 0; }
	bool operator==(const wchar_t* s) const { return compare(s) == 0; }

	bool operator!=(const UnicodeString& str) const { return !(*this == str); }
	bool operator!=(const wchar_t* s) const { return !(*this == s); }

	bool operator<(const UnicodeString& str) const { return compare(str) < 0; }
	bool operator<(const wchar_t* s) const { return compare(s) < 0; }

	// TODO: iterator versions

	UnicodeString& assign(const UnicodeString& str);
	// TODO
	// UnicodeString& Copy(const UnicodeString& str, size_t subpos, size_t sublen);
	UnicodeString& assign(const wchar_t* str) { return assign(str, StrLength(NullToEmpty(str))); }
	UnicodeString& assign(const wchar_t* str, size_t len) { return replace(0, size(), str, len); }
	UnicodeString& assign(size_t n, wchar_t c) { resize(n); std::fill(ALL_RANGE(*this), c); }


	UnicodeString& replace(size_t pos, size_t len, const UnicodeString& str) { return replace(pos, len, str.data(), str.size()); }
	// TODO
	// UnicodeString& replace(size_t pos,  size_t len, const UnicodeString& str, size_t subpos, size_t sublen);
	UnicodeString& replace(size_t pos, size_t len, const wchar_t* s) { return replace(pos, len, s, StrLength(NullToEmpty(s))); }
	UnicodeString& replace(size_t pos, size_t len, const wchar_t* s, size_t n);
	UnicodeString& replace(size_t pos, size_t len, size_t n, wchar_t c) {UnicodeString tmp; tmp.resize(n); std::fill(ALL_RANGE(tmp), c); return replace(pos, len, tmp);}  // TODO: optimize


	UnicodeString& append(const UnicodeString& str) { return append(str.data(), str.size()); }
	// TODO
	// UnicodeString& append(const UnicodeString& str, size_t subpos, size_t sublen);
	UnicodeString& append(const wchar_t* s) { return append(s, StrLength(NullToEmpty(s))); }
	UnicodeString& append(const wchar_t* s, size_t n) { return replace(size(), 0, s, n); }
	UnicodeString& append(size_t n, wchar_t c) { while (n--) append(&c, 1); return *this;} // TODO: optimize


	UnicodeString& insert(size_t pos, const UnicodeString& str) { return insert(pos, str.data(), str.size()); }
	// TODO
	// UnicodeString& insert(size_t pos, const UnicodeString& str, size_t subpos, size_t sublen);
	UnicodeString& insert(size_t pos, const wchar_t* s) { return insert(pos, s, StrLength(NullToEmpty(s))); }
	UnicodeString& insert(size_t pos, const wchar_t* s, size_t n) { return replace(pos, 0, s, n); }
	UnicodeString& insert(size_t pos, size_t n, wchar_t c) { while(n--) insert(pos, &c, 1); return *this;} // TODO: optimize


	int compare(const UnicodeString& str) const { return compare(0, npos, str.data(), str.size()); }
	int compare(size_t pos, size_t len, const UnicodeString& str) const { return compare(pos, len, str.data(), str.size()); }
	// TODO
	// int compare(size_t pos, size_t len, const string& str, size_t subpos, size_t sublen) const;
	int compare(const wchar_t* s) const { return compare(0, npos, s, StrLength(NullToEmpty(s))); }
	int compare(size_t pos, size_t len, const wchar_t* s) const { return compare(pos, len, s, StrLength(NullToEmpty(s))); }
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
	UnicodeString& Lower(size_t nStartPos=0, size_t nLength=npos);
	UnicodeString& Upper(size_t nStartPos=0, size_t nLength=npos);
	bool PosI(size_t &nPos, const wchar_t *lpwszFind, size_t nStartPos=0) const;

private:
	void SetEUS();
	void Inflate(size_t nSize);

	std::shared_ptr<UnicodeStringData> m_pData;
}
string;
