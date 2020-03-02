#pragma once

#include "error.hpp"

enum TriState {
  triTrue,
  triFalse,
  triUndef,
};

typedef std::vector<unsigned char> ByteVector;

bool substr_match(const std::wstring& str, std::wstring::size_type pos, std::wstring::const_pointer mstr);
std::wstring word_wrap(const std::wstring& str, std::wstring::size_type wrap_bound);
std::wstring fit_str(const std::wstring& str, std::wstring::size_type size);
std::wstring center(const std::wstring& str, unsigned width);
std::string strip(const std::string& str);
std::wstring strip(const std::wstring& str);
int str_to_int(const std::string& str);
int str_to_int(const std::wstring& str);
std::wstring int_to_str(int val);
uint64_t str_to_uint(const std::wstring& str);
std::wstring uint_to_str(uint64_t val);
std::wstring widen(const std::string& str);
std::list<std::wstring> split(const std::wstring& str, wchar_t sep);
std::wstring combine(const std::list<std::wstring>& lst, wchar_t sep);
std::wstring format_data_size(uint64_t value, const wchar_t* suffixes[5]);
bool is_slash(wchar_t c);
std::wstring unquote(const std::wstring& str);
std::wstring search_and_replace(const std::wstring& str, const std::wstring& search_str, const std::wstring& replace_str);

std::wstring long_path(const std::wstring& path);
std::wstring long_path_norm(const std::wstring& path);

std::wstring add_trailing_slash(const std::wstring& path);
std::wstring del_trailing_slash(const std::wstring& path);

std::wstring extract_path_root(const std::wstring& path);
std::wstring extract_file_name(const std::wstring& path);
std::wstring extract_file_path(const std::wstring& path);
std::wstring extract_file_ext(const std::wstring& path);
bool is_root_path(const std::wstring& path);
bool is_unc_path(const std::wstring& path);
bool is_absolute_path(const std::wstring& path);
std::wstring remove_path_root(const std::wstring& path);
std::wstring correct_filename(const std::wstring& name, int mode, bool alt_stream);

template<class T>
inline const T* null_to_empty(const T* Str) { static const T empty = T(); return Str? Str : &empty; }

int al_round(double d);

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
