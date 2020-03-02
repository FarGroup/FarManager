#pragma once
#include <cwchar>

#define __SIMPLE_STRING_USED

#define __DEF_DELTA 20

typedef class SimpleString
{
	private:
		wchar_t *m_str;
		size_t m_len;
		size_t m_size;

		void Alloc(size_t size)
		{
			if (size <= __DEF_DELTA)
				m_size = __DEF_DELTA;
			else if (size%__DEF_DELTA > 0)
				m_size = (size/__DEF_DELTA + 1) * __DEF_DELTA;
			else
				m_size = size;
			m_str = (wchar_t *)realloc(m_str,m_size*sizeof(wchar_t));
		}

	public:

		SimpleString() : m_str(NULL), m_len(0), m_size(0) { Alloc(__DEF_DELTA); }
		SimpleString(const SimpleString &strCopy) : m_str(NULL), m_len(0), m_size(0) { Alloc(strCopy.Len()+1); Copy(strCopy); }
		SimpleString(const wchar_t *data) : m_str(NULL), m_len(0), m_size(0) { size_t l = lstrlen(data?data:L""); Alloc(l+1); Copy(data, l); }
		SimpleString(const wchar_t *data, size_t len) : m_str(NULL), m_len(0), m_size(0) { Alloc(len+1); Copy(data, len); }
		explicit SimpleString(size_t size) : m_str(NULL), m_len(0), m_size(0) { Alloc(size); }

		~SimpleString() { free(m_str); }

		void Inflate(size_t size)
		{
			if (size <= m_size)
				return;

			if (size >= __DEF_DELTA << 3)
				size = size << 1;
			else
				size = (size/__DEF_DELTA + 1) * __DEF_DELTA;

			Alloc(size);
		}

		wchar_t *GetBuf(size_t size = (size_t)-1) { Inflate(size == (size_t)-1 ? m_size : size); return m_str; }
		void ReleaseBuf(size_t len = (size_t)-1) { (len == (size_t)-1) ? SetLen(lstrlen(m_str)) : (len >= m_size ? SetLen(m_size-1) : SetLen(len)); }

		size_t Len() const { return m_len; }
		size_t SetLen(size_t len) { if (len < m_size) { m_len = len; m_str[m_len] = 0; } return m_len; }

		size_t Size() const { return m_size; }

		wchar_t At(size_t index) const { return m_str[index]; }

		bool IsEmpty() const { return !m_len; }

		int __cdecl Format(const wchar_t * format, ...)
		{
			wchar_t *buffer = nullptr;
			size_t Size = MAX_PATH;
			int retValue = -1;
			va_list argptr;
			va_start(argptr, format);

			do
			{
				Size <<= 1;
				wchar_t *tmpbuffer = (wchar_t*)realloc(buffer, Size*sizeof(wchar_t));

				if (!tmpbuffer)
				{
					va_end(argptr);
					free(buffer);
					return retValue;
				}

				buffer = tmpbuffer;
				//_vsnwprintf не всегда ставит '\0' вконце.
				//Поэтому надо обнулить и передать в _vsnwprintf размер-1.
				buffer[Size-1] = 0;
				retValue = _vsnwprintf(buffer, Size-1, format, argptr);
			}
			while (retValue == -1);

			va_end(argptr);
			Copy(buffer);
			free(buffer);
			return retValue;
		}

		SimpleString& Replace(size_t Pos, size_t Len, const wchar_t* Data, size_t DataLen)
		{
			// Pos & Len must be valid
			// Data and *this must not intersect (but Data can be located entirely within *this)

			if (!Len && !DataLen)
				return *this;

			size_t NewLength = m_len + DataLen - Len;

			if (NewLength)
			{
				if (Data >= m_str && Data + DataLen <= m_str + m_len)
				{
					// copy data from self
					SimpleString TmpStr(Data, DataLen);
					Inflate(NewLength + 1);
					wmemmove(m_str + Pos + DataLen, m_str + Pos + Len, m_len - Pos - Len);
					wmemcpy(m_str + Pos, TmpStr.CPtr(), TmpStr.Len());
				}
				else
				{
					Inflate(NewLength + 1);
					wmemmove(m_str + Pos + DataLen, m_str + Pos + Len, m_len - Pos - Len);
					wmemcpy(m_str + Pos, Data, DataLen);
				}
			}

			SetLen(NewLength);

			return *this;
		}

		SimpleString& Replace(size_t Pos, size_t Len, const SimpleString& Str) { return Replace(Pos, Len, Str.CPtr(), Str.Len()); }
		SimpleString& Replace(size_t Pos, size_t Len, const wchar_t* Str) { return Replace(Pos, Len, Str, lstrlen(Str?Str:L"")); }
		SimpleString& Replace(size_t Pos, size_t Len, wchar_t Ch) { return Replace(Pos, Len, &Ch, 1); }

		SimpleString& Append(const wchar_t* Str, size_t StrLen) { return Replace(Len(), 0, Str, StrLen); }
		SimpleString& Append(const SimpleString& Str) { return Append(Str.CPtr(), Str.Len()); }
		SimpleString& Append(const wchar_t* Str) { return Append(Str, lstrlen(Str?Str:L"")); }
		SimpleString& Append(wchar_t Ch) { return Append(&Ch, 1); }

		SimpleString& Insert(size_t Pos, const wchar_t* Str, size_t StrLen) { return Replace(Pos, 0, Str, StrLen); }
		SimpleString& Insert(size_t Pos, const SimpleString& Str) { return Insert(Pos, Str.CPtr(), Str.Len()); }
		SimpleString& Insert(size_t Pos, const wchar_t* Str) { return Insert(Pos, Str, lstrlen(Str?Str:L"")); }
		SimpleString& Insert(size_t Pos, wchar_t Ch) { return Insert(Pos, &Ch, 1); }

		SimpleString& Copy(const wchar_t *Str, size_t StrLen) { return Replace(0, Len(), Str, StrLen); }
		SimpleString& Copy(const wchar_t *Str) { return Copy(Str, lstrlen(Str?Str:L"")); }
		SimpleString& Copy(wchar_t Ch) { return Copy(&Ch, 1); }
		SimpleString& Copy(const SimpleString &Str) { return Copy(Str.CPtr(), Str.Len()); }

		SimpleString& Remove(size_t Pos, size_t Len = 1) { return Replace(Pos, Len, nullptr, 0); }
		SimpleString& LShift(size_t nShiftCount, size_t nStartPos=0) { return Remove(nStartPos, nShiftCount); }

		SimpleString& Clear() { m_len = 0; *m_str = 0; return *this; }

		const wchar_t *CPtr() const { return m_str; }
		operator const wchar_t *() const { return m_str; }

		const SimpleString& operator=(const SimpleString &strCopy) { return Copy(strCopy); }
		const SimpleString& operator=(const wchar_t *lpwszData) { return Copy(lpwszData); }
		const SimpleString& operator=(wchar_t chData) { return Copy(chData); }

		const SimpleString& operator+=(const SimpleString &strAdd) { return Append(strAdd); }
		const SimpleString& operator+=(const wchar_t *lpwszAdd) { return Append(lpwszAdd); }
		const SimpleString& operator+=(wchar_t chAdd) { return Append(chAdd); }

		friend const SimpleString operator+(const SimpleString &strSrc1, const SimpleString &strSrc2);
		friend const SimpleString operator+(const SimpleString &strSrc1, const wchar_t *lpwszSrc2);

} string;

const SimpleString operator+(const SimpleString &strSrc1, const SimpleString &strSrc2)
{
	return SimpleString(strSrc1).Append(strSrc2);
}

const SimpleString operator+(const SimpleString &strSrc1, const wchar_t *lpwszSrc2)
{
	return SimpleString(strSrc1).Append(lpwszSrc2);
}
