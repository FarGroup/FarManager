#include "utils.hpp"

bool substr_match(const wstring& str, wstring::size_type pos, wstring::const_pointer mstr) {
  size_t mstr_len = wcslen(mstr);
  if ((pos > str.length()) || (pos + mstr_len > str.length())) {
    return false;
  }
  return wmemcmp(str.data() + pos, mstr, mstr_len) == 0;
}

wstring word_wrap(const wstring& str, wstring::size_type wrap_bound) {
  wstring result;
  wstring::size_type begin_pos = 0;
  while (begin_pos < str.size()) {
    wstring::size_type end_pos = begin_pos + wrap_bound;
    if (end_pos < str.size()) {
      for (wstring::size_type i = end_pos; i > begin_pos; i--) {
        if (str[i - 1] == L' ') {
          end_pos = i;
          break;
        }
      }
    }
    else {
      end_pos = str.size();
    }
    wstring::size_type trim_pos = end_pos;
    while (trim_pos > begin_pos && str[trim_pos - 1] == L' ') trim_pos--;
    if (trim_pos > begin_pos) {
      if (!result.empty())
        result.append(1, L'\n');
      result.append(str.data() + begin_pos, trim_pos - begin_pos);
    }
    begin_pos = end_pos;
  }
  return result;
}

wstring fit_str(const wstring& str, wstring::size_type size) {
  if (str.size() <= size)
    return str;
  if (size <= 3)
    return str.substr(0, size);
  size -= 3; // place for ...
  return wstring(str).replace(size / 2, str.size() - size, L"...");
}

wstring center(const wstring& str, unsigned width) {
  if (str.size() >= width)
    return str;
  size_t lpad = (width - str.size()) / 2;
  size_t rpad = width - str.size() - lpad;
  wstring result(lpad, L' ');
  result.append(str);
  result.append(rpad, L' ');
  return result;
}

template<class CharType> basic_string<CharType> strip(const basic_string<CharType>& str) {
  basic_string<CharType>::size_type hp = 0;
  basic_string<CharType>::size_type tp = str.size();
  while ((hp < str.size()) && ((static_cast<unsigned>(str[hp]) <= 0x20) || (str[hp] == 0x7F)))
    hp++;
  if (hp < str.size())
    while ((static_cast<unsigned>(str[tp - 1]) <= 0x20) || (str[tp - 1] == 0x7F))
      tp--;
  return str.substr(hp, tp - hp);
}

string strip(const string& str) {
  return strip<string::value_type>(str);
}

wstring strip(const wstring& str) {
  return strip<wstring::value_type>(str);
}

int str_to_int(const string& str) {
  return atoi(str.c_str());
}

int str_to_int(const wstring& str) {
  return _wtoi(str.c_str());
}

wstring int_to_str(int val) {
  wchar_t str[64];
  return _itow(val, str, 10);
}

unsigned __int64 str_to_uint(const wstring& str) {
  unsigned __int64 val = 0;
  for (unsigned i = 0; i < str.size() && str[i] >= L'0' && str[i] <= L'9'; i++) {
    val = val * 10 + (str[i] - L'0');
  }
  return val;
}

wstring uint_to_str(unsigned __int64 val) {
  if (val == 0)
    return L"0";
  wchar_t str[32];
  unsigned pos = ARRAYSIZE(str);
  while (val) {
    pos--;
    str[pos] = val % 10 + L'0';
    val /= 10;
  }
  return wstring(str + pos, ARRAYSIZE(str) - pos);
}

wstring widen(const string& str) {
  return wstring(str.begin(), str.end());
}

list<wstring> split(const wstring& str, wchar_t sep) {
  list<wstring> result;
  size_t begin = 0;
  while (begin < str.size()) {
    size_t end = str.find(sep, begin);
    if (end == wstring::npos)
      end = str.size();
    wstring sub_str = str.substr(begin, end - begin);
    result.push_back(sub_str);
    begin = end + 1;
  }
  return result;
}

wstring combine(const list<wstring>& lst, wchar_t sep) {
  size_t size = 0;
  for (list<wstring>::const_iterator str = lst.begin(); str != lst.end(); str++) {
    if (size)
      size++;
    size += str->size();
  }
  wstring result;
  result.reserve(size);
  for (list<wstring>::const_iterator str = lst.begin(); str != lst.end(); str++) {
    if (!result.empty())
      result.append(1, sep);
    result.append(*str);
  }
  return result;
}

wstring format_data_size(unsigned __int64 value, const wchar_t* suffixes[5]) {
  unsigned f = 0;
  unsigned __int64 div = 1;
  while ((value / div >= 1000) && (f < 4)) {
    f++;
    div *= 1024;
  }
  unsigned __int64 v1 = value / div;

  unsigned __int64 mul;
  if (v1 < 10) mul = 100;
  else if (v1 < 100) mul = 10;
  else mul = 1;

  unsigned __int64 v2 = value % div;
  unsigned __int64 d = v2 * mul * 10 / div % 10;
  v2 = v2 * mul / div;
  if (d >= 5) {
    if (v2 + 1 == mul) {
      v2 = 0;
      if ((v1 == 999) && (f < 4)) {
        v1 = 0;
        v2 = 98;
        f += 1;
      }
      else v1 += 1;
    }
    else v2 += 1;
  }

  wstring result;
  wchar_t buf[30];
  _ui64tow_s(v1, buf, ARRAYSIZE(buf), 10);
  result += buf;
  if (v2 != 0) {
    result += L'.';
    if ((v1 < 10) && (v2 < 10)) result += L'0';
    _ui64tow_s(v2, buf, ARRAYSIZE(buf), 10);
    result += buf;
  }
  if (*suffixes[f]) {
    result += L' ';
    result += suffixes[f];
  }
  return result;
}

bool is_slash(wchar_t c) {
  return c == L'\\' || c == L'/';
}

wstring unquote(const wstring& str) {
  if (str.size() >= 2 && str[0] == L'"' && str[str.size() - 1] == L'"')
    return str.substr(1, str.size() - 2);
  else
    return str;
}

wstring search_and_replace(const wstring& str, const wstring& search_str, const wstring& replace_str) {
  assert(!search_str.empty());
  if (search_str.empty())
    return str;
  wstring result;
  result.reserve(str.size());
  size_t pos1 = 0;
  while (true) {
    size_t pos2 = str.find(search_str, pos1);
    if (pos2 != wstring::npos) {
      result.append(str, pos1, pos2 - pos1);
      result.append(replace_str);
      pos1 = pos2 + search_str.size();
    }
    else {
      result.append(str, pos1, str.size() - pos1);
      break;
    }
  }
  result.shrink_to_fit();
  return result;
}
