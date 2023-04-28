#include "utils.hpp"

bool substr_match(const std::wstring& str, std::wstring::size_type pos, std::wstring::const_pointer mstr) {
  size_t mstr_len = std::wcslen(mstr);
  if ((pos > str.length()) || (pos + mstr_len > str.length())) {
    return false;
  }
  return wmemcmp(str.c_str() + pos, mstr, mstr_len) == 0;
}

std::wstring word_wrap(const std::wstring& str, std::wstring::size_type wrap_bound) {
  std::wstring result;
  std::wstring::size_type begin_pos = 0;
  while (begin_pos < str.size()) {
    std::wstring::size_type end_pos = begin_pos + wrap_bound;
    if (end_pos < str.size()) {
      for (std::wstring::size_type i = end_pos; i > begin_pos; i--) {
        if (str[i - 1] == L' ') {
          end_pos = i;
          break;
        }
      }
    }
    else {
      end_pos = str.size();
    }
    std::wstring::size_type trim_pos = end_pos;
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

std::wstring fit_str(const std::wstring& str, std::wstring::size_type size) {
  if (str.size() <= size)
    return str;
  if (size <= 3)
    return str.substr(0, size);
  size -= 3; // place for ...
  return std::wstring(str).replace(size / 2, str.size() - size, L"...");
}

std::wstring center(const std::wstring& str, unsigned width) {
  if (str.size() >= width)
    return str;
  size_t lpad = (width - str.size()) / 2;
  size_t rpad = width - str.size() - lpad;
  std::wstring result(lpad, L' ');
  result.append(str);
  result.append(rpad, L' ');
  return result;
}

template<class CharType> std::basic_string<CharType> strip(const std::basic_string<CharType>& str) {
  size_t hp = 0;
  size_t tp = str.size();
  while ((hp < str.size()) && ((static_cast<unsigned>(str[hp]) <= 0x20) || (str[hp] == 0x7F)))
    hp++;
  if (hp < str.size())
    while ((static_cast<unsigned>(str[tp - 1]) <= 0x20) || (str[tp - 1] == 0x7F))
      tp--;
  return str.substr(hp, tp - hp);
}

std::string strip(const std::string& str) {
  return strip<std::string::value_type>(str);
}

std::wstring strip(const std::wstring& str) {
  return strip<std::wstring::value_type>(str);
}

int str_to_int(const std::string& str) {
  return atoi(str.c_str());
}

int str_to_int(const std::wstring& str) {
  return _wtoi(str.c_str());
}

std::wstring int_to_str(int val) {
  wchar_t str[64];
  return _itow(val, str, 10);
}

uint64_t str_to_uint(const std::wstring& str) {
  uint64_t val = 0;
  for (unsigned i = 0; i < str.size() && str[i] >= L'0' && str[i] <= L'9'; i++) {
    val = val * 10 + (str[i] - L'0');
  }
  return val;
}

std::wstring uint_to_str(uint64_t val) {
  if (val == 0)
    return L"0";
  wchar_t str[32];
  unsigned pos = ARRAYSIZE(str);
  while (val) {
    pos--;
    str[pos] = val % 10 + L'0';
    val /= 10;
  }
  return std::wstring(str + pos, ARRAYSIZE(str) - pos);
}

std::wstring widen(const std::string& str) {
  return std::wstring(str.begin(), str.end());
}

std::list<std::wstring> split(const std::wstring& str, wchar_t sep) {
  std::list<std::wstring> result;
  size_t begin = 0;
  while (begin < str.size()) {
    size_t end = str.find(sep, begin);
    if (end == std::wstring::npos)
      end = str.size();
    std::wstring sub_str = str.substr(begin, end - begin);
    result.push_back(sub_str);
    begin = end + 1;
  }
  return result;
}

std::wstring combine(const std::list<std::wstring>& lst, wchar_t sep) {
  size_t size = 0;
  for (const auto& str: lst) {
    if (size)
      size++;
    size += str.size();
  }
  std::wstring result;
  result.reserve(size);
  for (const auto& str: lst) {
    if (!result.empty())
      result.append(1, sep);
    result.append(str);
  }
  return result;
}

std::wstring format_data_size(uint64_t value, const wchar_t* suffixes[5]) {
  unsigned f = 0;
  uint64_t div = 1;
  while ((value / div >= 1000) && (f < 4)) {
    f++;
    div *= 1024;
  }
  uint64_t v1 = value / div;

  uint64_t mul;
  if (v1 < 10) mul = 100;
  else if (v1 < 100) mul = 10;
  else mul = 1;

  uint64_t v2 = value % div;
  uint64_t d = v2 * mul * 10 / div % 10;
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

  std::wstring result;
  wchar_t buf[65];
  _ui64tow(v1, buf, 10);
  result += buf;
  if (v2 != 0) {
    result += L'.';
    if ((v1 < 10) && (v2 < 10)) result += L'0';
    _ui64tow(v2, buf, 10);
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

std::wstring unquote(const std::wstring& str) {
  if (str.size() >= 2 && str[0] == L'"' && str[str.size() - 1] == L'"')
    return str.substr(1, str.size() - 2);
  else
    return str;
}

std::wstring search_and_replace(const std::wstring& str, const std::wstring& search_str, const std::wstring& replace_str) {
  assert(!search_str.empty());
  if (search_str.empty())
    return str;
  std::wstring result;
  result.reserve(str.size());
  size_t pos1 = 0;
  while (true) {
    size_t pos2 = str.find(search_str, pos1);
    if (pos2 != std::wstring::npos) {
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

bool str_start_with(const std::wstring& str, const wchar_t* prefix, const bool ignore_case) {
  auto pref_len = wcslen(prefix);
  if (pref_len > str.size())
    return false;
  return (ignore_case ? _wcsnicmp(str.c_str(), prefix, pref_len) : wcsncmp(str.c_str(), prefix, pref_len)) == 0;
}

bool str_end_with(const std::wstring& str, const wchar_t* suffix, const bool ignore_case)
{
  auto suff_len = wcslen(suffix);
  if (suff_len > str.size())
    return false;
  size_t off = str.size() - suff_len;
	return (ignore_case ? _wcsicmp(str.c_str()+off, suffix) : wcscmp(str.c_str()+off, suffix)) == 0;
}
