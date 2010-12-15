#include "utils.hpp"

wstring long_path(const wstring& path) {
  if (substr_match(path, 0, L"\\\\")) {
    if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
      return path;
    }
    else {
      return wstring(path).replace(0, 1, L"\\\\?\\UNC");
    }
  }
  else {
    return wstring(path).insert(0, L"\\\\?\\");
  }
}

wstring add_trailing_slash(const wstring& path) {
  if ((path.size() == 0) || (path[path.size() - 1] == L'\\')) {
    return path;
  }
  else {
    return path + L'\\';
  }
}

wstring del_trailing_slash(const wstring& path) {
  if ((path.size() < 2) || (path[path.size() - 1] != L'\\')) {
    return path;
  }
  else {
    return path.substr(0, path.size() - 1);
  }
}

void locate_path_root(const wstring& path, size_t& path_root_len, bool& is_unc_path) {
  unsigned prefix_len = 0;
  is_unc_path = false;
  if (substr_match(path, 0, L"\\\\")) {
    if (substr_match(path, 2, L"?\\UNC\\")) {
      prefix_len = 8;
      is_unc_path = true;
    }
    else if (substr_match(path, 2, L"?\\") || substr_match(path, 2, L".\\")) {
      prefix_len = 4;
    }
    else {
      prefix_len = 2;
      is_unc_path = true;
    }
  }
  if ((prefix_len == 0) && !substr_match(path, 1, L":\\")) {
    path_root_len = 0;
  }
  else {
    wstring::size_type p = path.find(L'\\', prefix_len);
    if (p == wstring::npos) {
      p = path.size();
    }
    if (is_unc_path) {
      p = path.find(L'\\', p + 1);
      if (p == wstring::npos) {
        p = path.size();
      }
    }
    path_root_len = p;
  }
}

wstring extract_path_root(const wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if (path_root_len)
    return path.substr(0, path_root_len).append(1, L'\\');
  else
    return wstring();
}

wstring extract_file_name(const wstring& path) {
  size_t pos = path.rfind(L'\\');
  if (pos == wstring::npos) {
    pos = 0;
  }
  else {
    pos++;
  }
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((pos <= path_root_len) && (path_root_len != 0))
    return wstring();
  else
    return path.substr(pos);
}

wstring extract_file_path(const wstring& path) {
  size_t pos = path.rfind(L'\\');
  if (pos == wstring::npos) {
    pos = 0;
  }
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((pos <= path_root_len) && (path_root_len != 0))
    return path.substr(0, path_root_len).append(1, L'\\');
  else
    return path.substr(0, pos);
}

wstring extract_file_ext(const wstring& path) {
  size_t ext_pos = path.rfind(L'.');
  if (ext_pos == wstring::npos) {
    return wstring();
  }
  size_t name_pos = path.rfind(L'\\');
  if (name_pos == wstring::npos) {
    name_pos = 0;
  }
  else {
    name_pos++;
  }
  if (ext_pos <= name_pos)
    return wstring();
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((ext_pos <= path_root_len) && (path_root_len != 0))
    return wstring();
  else
    return path.substr(ext_pos);
}

bool is_root_path(const wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  return (path.size() == path_root_len) || ((path.size() == path_root_len + 1) && (path[path.size() - 1] == L'\\'));
}

bool is_unc_path(const wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  return is_unc_path;
}

bool is_absolute_path(const wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if (path_root_len == 0)
    return false;
  wstring::size_type p1 = path_root_len;
  while (p1 != path.size()) {
    p1 += 1;
    wstring::size_type p2 = path.find(L'\\', p1);
    if (p2 == wstring::npos)
      p2 = path.size();
    wstring::size_type sz = p2 - p1;
    if (sz == 1 && path[p1] == L'.')
      return false;
    if (sz == 2 && path[p1] == L'.' && path[p1 + 1] == L'.')
      return false;
    p1 = p2;
  }
  return true;
}

wstring remove_path_root(const wstring& path) {
  size_t path_root_len;
  bool is_unc_path;
  locate_path_root(path, path_root_len, is_unc_path);
  if ((path_root_len < path.size()) && (path[path_root_len] == L'\\'))
    path_root_len++;
  return path.substr(path_root_len);
}
