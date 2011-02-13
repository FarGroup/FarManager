#pragma once

#include "error.hpp"

enum TriState {
  triTrue,
  triFalse,
  triUndef,
};

typedef vector<unsigned char> ByteVector;

bool substr_match(const wstring& str, wstring::size_type pos, wstring::const_pointer mstr);
wstring word_wrap(const wstring& str, wstring::size_type wrap_bound);
wstring fit_str(const wstring& str, wstring::size_type size);
wstring center(const wstring& str, unsigned width);
string strip(const string& str);
wstring strip(const wstring& str);
int str_to_int(const string& str);
int str_to_int(const wstring& str);
wstring int_to_str(int val);
unsigned __int64 str_to_uint(const wstring& str);
wstring uint_to_str(unsigned __int64 val);
wstring widen(const string& str);
list<wstring> split(const wstring& str, wchar_t sep);
wstring combine(const list<wstring>& lst, wchar_t sep);
wstring format_data_size(unsigned __int64 value, const wchar_t* suffixes[5]);
bool is_slash(wchar_t c);
wstring unquote(const wstring& str);

wstring long_path(const wstring& path);

wstring add_trailing_slash(const wstring& path);
wstring del_trailing_slash(const wstring& path);

wstring extract_path_root(const wstring& path);
wstring extract_file_name(const wstring& path);
wstring extract_file_path(const wstring& path);
wstring extract_file_ext(const wstring& path);
bool is_root_path(const wstring& path);
bool is_unc_path(const wstring& path);
bool is_absolute_path(const wstring& path);
wstring remove_path_root(const wstring& path);

int round(double d);

class NonCopyable {
protected:
  NonCopyable() {}
  ~NonCopyable() {}
private:
  NonCopyable(const NonCopyable&);
  NonCopyable& operator=(const NonCopyable&);
};

template<typename Type> class Buffer: private NonCopyable {
private:
  Type* buffer;
  size_t buf_size;
public:
  Buffer(): buffer(nullptr), buf_size(0) {
  }
  Buffer(size_t size) {
    buffer = new Type[size];
    buf_size = size;
  }
  ~Buffer() {
    delete[] buffer;
  }
  void resize(size_t size) {
    delete[] buffer;
    buffer = new Type[size];
    buf_size = size;
  }
  Type* data() {
    return buffer;
  }
  size_t size() const {
    return buf_size;
  }
  void clear() {
    memset(buffer, 0, buf_size * sizeof(Type));
  }
};

template<typename T> void memzero(T& v) {
  memset(&v, 0, sizeof(T));
}
