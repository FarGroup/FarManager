#include "msg.h"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"

const wchar_t* c_method_copy = L"Copy";
const wchar_t* c_method_lzma = L"LZMA";
const wchar_t* c_method_lzma2 = L"LZMA2";
const wchar_t* c_method_ppmd = L"PPMD";

#define DEFINE_ARC_ID(name, value) \
  const unsigned char c_guid_##name[] = "\x69\x0F\x17\x23\xC1\x40\x8A\x27\x10\x00\x00\x01\x10" value "\x00\x00"; \
  const ArcType c_##name(c_guid_##name, c_guid_##name + 16);

DEFINE_ARC_ID(7z, "\x07")
DEFINE_ARC_ID(zip, "\x01")
DEFINE_ARC_ID(bzip2, "\x02")
DEFINE_ARC_ID(gzip, "\xEF")
DEFINE_ARC_ID(xz, "\x0C")
DEFINE_ARC_ID(iso, "\xE7")
DEFINE_ARC_ID(udf, "\xE0")
DEFINE_ARC_ID(rar, "\x03")
DEFINE_ARC_ID(split, "\xEA")
DEFINE_ARC_ID(wim, "\xE6")
DEFINE_ARC_ID(tar, "\xEE")

#undef DEFINE_ARC_ID

const unsigned __int64 c_min_volume_size = 16 * 1024;

const wchar_t* c_sfx_ext = L".exe";
const wchar_t* c_volume_ext = L".001";

unsigned Archive::max_check_size;

HRESULT ArcLib::get_prop(UInt32 index, PROPID prop_id, PROPVARIANT* prop) const {
  if (GetHandlerProperty2) {
    return GetHandlerProperty2(index, prop_id, prop);
  }
  else {
    assert(index == 0);
    return GetHandlerProperty(prop_id, prop);
  }
}

HRESULT ArcLib::get_bool_prop(UInt32 index, PROPID prop_id, bool& value) const {
  PropVariant prop;
  HRESULT res = get_prop(index, prop_id, prop.ref());
  if (res != S_OK)
    return res;
  if (!prop.is_bool())
    return E_FAIL;
  value = prop.get_bool();
  return S_OK;
}

HRESULT ArcLib::get_string_prop(UInt32 index, PROPID prop_id, wstring& value) const {
  PropVariant prop;
  HRESULT res = get_prop(index, prop_id, prop.ref());
  if (res != S_OK)
    return res;
  if (!prop.is_str())
    return E_FAIL;
  value = prop.get_str();
  return S_OK;
}

HRESULT ArcLib::get_bytes_prop(UInt32 index, PROPID prop_id, ByteVector& value) const {
  PropVariant prop;
  HRESULT res = get_prop(index, prop_id, prop.ref());
  if (res != S_OK)
    return res;
  if (prop.vt == VT_BSTR) {
    UINT len = SysStringByteLen(prop.bstrVal);
    unsigned char* data =  reinterpret_cast<unsigned char*>(prop.bstrVal);
    value.assign(data, data + len);
  }
  else
    return E_FAIL;
  return S_OK;
}


wstring ArcFormat::default_extension() const {
  if (extension_list.empty())
    return wstring();
  else
    return extension_list.front();
}


ArcTypes ArcFormats::get_arc_types() const {
  ArcTypes types;
  for (const_iterator fmt = begin(); fmt != end(); fmt++) {
    types.push_back(fmt->first);
  }
  return types;
}

ArcTypes ArcFormats::find_by_name(const wstring& name) const {
  ArcTypes types;
  wstring uc_name = upcase(name);
  for (const_iterator fmt = begin(); fmt != end(); fmt++) {
    if (upcase(fmt->second.name) == uc_name)
      types.push_back(fmt->first);
  }
  return types;
}

ArcTypes ArcFormats::find_by_ext(const wstring& ext) const {
  ArcTypes types;
  if (ext.empty())
    return types;
  wstring uc_ext = upcase(ext);
  for (const_iterator fmt = begin(); fmt != end(); fmt++) {
    for (auto ext_iter = fmt->second.extension_list.cbegin(); ext_iter != fmt->second.extension_list.cend(); ext_iter++) {
      if (upcase(*ext_iter) == uc_ext) {
        types.push_back(fmt->first);
        break;
      }
    }
  }
  return types;
}


unsigned SfxModules::find_by_name(const wstring& name) const {
  for (const_iterator sfx_module = begin(); sfx_module != end(); sfx_module++) {
    if (upcase(extract_file_name(sfx_module->path)) == upcase(name))
      return distance(begin(), sfx_module);
  }
  return size();
}


wstring ArcChain::to_string() const {
  wstring result;
  for_each(begin(), end(), [&] (const ArcEntry& arc) {
    if (!result.empty())
      result += L"\x2192";
    result += ArcAPI::formats().at(arc.type).name;
  });
  return result;
}


ArcAPI* ArcAPI::arc_api = nullptr;

ArcAPI::~ArcAPI() {
  for_each(arc_libs.begin(), arc_libs.end(), [&] (const ArcLib& arc_lib) {
    if (arc_lib.h_module)
      FreeLibrary(arc_lib.h_module);
  });
}

ArcAPI* ArcAPI::get() {
  if (arc_api == nullptr) {
    arc_api = new ArcAPI();
    arc_api->load();
  }
  return arc_api;
}

void ArcAPI::load_libs(const wstring& path) {
  FileEnum file_enum(path);
  wstring dir = extract_file_path(path);
  bool more;
  while (file_enum.next_nt(more) && more) {
    ArcLib arc_lib;
    arc_lib.module_path = add_trailing_slash(dir) + file_enum.data().cFileName;
    arc_lib.h_module = LoadLibraryW(arc_lib.module_path.c_str());
    if (arc_lib.h_module == nullptr)
      continue;
    arc_lib.CreateObject = reinterpret_cast<ArcLib::FCreateObject>(GetProcAddress(arc_lib.h_module, "CreateObject"));
    arc_lib.GetNumberOfMethods = reinterpret_cast<ArcLib::FGetNumberOfMethods>(GetProcAddress(arc_lib.h_module, "GetNumberOfMethods"));
    arc_lib.GetMethodProperty = reinterpret_cast<ArcLib::FGetMethodProperty>(GetProcAddress(arc_lib.h_module, "GetMethodProperty"));
    arc_lib.GetNumberOfFormats = reinterpret_cast<ArcLib::FGetNumberOfFormats>(GetProcAddress(arc_lib.h_module, "GetNumberOfFormats"));
    arc_lib.GetHandlerProperty = reinterpret_cast<ArcLib::FGetHandlerProperty>(GetProcAddress(arc_lib.h_module, "GetHandlerProperty"));
    arc_lib.GetHandlerProperty2 = reinterpret_cast<ArcLib::FGetHandlerProperty2>(GetProcAddress(arc_lib.h_module, "GetHandlerProperty2"));
    if (arc_lib.CreateObject && ((arc_lib.GetNumberOfFormats && arc_lib.GetHandlerProperty2) || arc_lib.GetHandlerProperty)) {
      arc_lib.version = get_module_version(arc_lib.module_path);
      arc_libs.push_back(arc_lib);
    }
    else
      FreeLibrary(arc_lib.h_module);
  }
}


struct SfxModuleInfo {
  const wchar_t* module_name;
  unsigned descr_id;
  bool all_codecs;
  bool install_config;
};

const SfxModuleInfo c_known_sfx_modules[] = {
  { L"7z.sfx", MSG_SFX_DESCR_7Z, true, false },
  { L"7zCon.sfx", MSG_SFX_DESCR_7ZCON, true, false },
  { L"7zS.sfx", MSG_SFX_DESCR_7ZS, false, true },
  { L"7zSD.sfx", MSG_SFX_DESCR_7ZSD, false, true },
  { L"7zS2.sfx", MSG_SFX_DESCR_7ZS2, false, false },
  { L"7zS2con.sfx", MSG_SFX_DESCR_7ZS2CON, false, false },
};

const SfxModuleInfo* find(const wstring& path) {
  unsigned i = 0;
  for (; i < ARRAYSIZE(c_known_sfx_modules) && upcase(extract_file_name(path)) != upcase(c_known_sfx_modules[i].module_name); i++);
  if (i < ARRAYSIZE(c_known_sfx_modules))
    return c_known_sfx_modules + i;
  else
    return nullptr;
}

wstring SfxModule::description() const {
  const SfxModuleInfo* info = find(path);
  return info ? Far::get_msg(info->descr_id) : Far::get_msg(MSG_SFX_DESCR_UNKNOWN) + L" [" + extract_file_name(path) + L"]";
}

bool SfxModule::all_codecs() const {
  const SfxModuleInfo* info = find(path);
  return info ? info->all_codecs : true;
}

bool SfxModule::install_config() const {
  const SfxModuleInfo* info = find(path);
  return info ? info->install_config : true;
}

void ArcAPI::find_sfx_modules(const wstring& path) {
  FileEnum file_enum(path);
  wstring dir = extract_file_path(path);
  bool more;
  while (file_enum.next_nt(more) && more) {
    SfxModule sfx_module;
    sfx_module.path = add_trailing_slash(dir) + file_enum.data().cFileName;
    File file;
    if (!file.open_nt(sfx_module.path, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING, 0))
      continue;
    Buffer<char> buffer(2);
    unsigned sz;
    if (!file.read_nt(buffer.data(), static_cast<unsigned>(buffer.size()), sz))
      continue;
    string sig(buffer.data(), sz);
    if (sig != "MZ")
      continue;
    sfx_modules.push_back(sfx_module);
  }
}

void ArcAPI::load() {
  load_libs(add_trailing_slash(Far::get_plugin_module_path()) + L"*.dll");
  find_sfx_modules(add_trailing_slash(Far::get_plugin_module_path()) + L"*.sfx");
  if (arc_libs.empty() || sfx_modules.empty()) {
    wstring _7zip_path;
    Key _7zip_key;
    _7zip_key.open_nt(HKEY_CURRENT_USER, L"Software\\7-Zip", KEY_QUERY_VALUE, false) && _7zip_key.query_str_nt(_7zip_path, L"Path");
    if (_7zip_path.empty())
      _7zip_key.open_nt(HKEY_LOCAL_MACHINE, L"Software\\7-Zip", KEY_QUERY_VALUE, false) && _7zip_key.query_str_nt(_7zip_path, L"Path");
    if (!_7zip_path.empty()) {
      if (arc_libs.empty())
        load_libs(add_trailing_slash(_7zip_path) + L"7z.dll");
      if (sfx_modules.empty())
        find_sfx_modules(add_trailing_slash(_7zip_path) + L"*.sfx");
    }
  }
  if (arc_libs.empty()) {
    wstring _7z_dll_path;
    IGNORE_ERRORS(_7z_dll_path = search_path(L"7z.dll"));
    if (!_7z_dll_path.empty())
      load_libs(_7z_dll_path);
  }
  for (unsigned i = 0; i < arc_libs.size(); i++) {
    const ArcLib& arc_lib = arc_libs[i];

    UInt32 num_formats;
    if (arc_lib.GetNumberOfFormats) {
      if (arc_lib.GetNumberOfFormats(&num_formats) != S_OK)
        num_formats = 0;
    }
    else
      num_formats = 1;

    for (UInt32 idx = 0; idx < num_formats; idx++) {
      ArcFormat arc_format;
      arc_format.lib_index = i;
      ArcType type;
      if (arc_lib.get_bytes_prop(idx, NArchive::kClassID, type) != S_OK) continue;
      arc_lib.get_string_prop(idx, NArchive::kName, arc_format.name);
      if (arc_lib.get_bool_prop(idx, NArchive::kUpdate, arc_format.updatable) != S_OK)
        arc_format.updatable = false;
      arc_lib.get_bytes_prop(idx, NArchive::kStartSignature, arc_format.start_signature);
      wstring extension_list_str;
      arc_lib.get_string_prop(idx, NArchive::kExtension, extension_list_str);
      arc_format.extension_list = split(extension_list_str, L' ');
      wstring add_extension_list_str;
      arc_lib.get_string_prop(idx, NArchive::kAddExtension, add_extension_list_str);
      std::list<wstring> add_extension_list = split(add_extension_list_str, L' ');
      auto add_ext_iter = add_extension_list.cbegin();
      for (auto ext_iter = arc_format.extension_list.begin(); ext_iter != arc_format.extension_list.end(); ++ext_iter) {
        ext_iter->insert(0, 1, L'.');
        if (add_ext_iter != add_extension_list.cend()) {
          if (*add_ext_iter != L"*") {
            arc_format.nested_ext_mapping[upcase(*ext_iter)] = *add_ext_iter;
          }
          ++add_ext_iter;
        }
      }

      ArcFormats::const_iterator existing_format = arc_formats.find(type);
      if (existing_format == arc_formats.end() || arc_libs[existing_format->second.lib_index].version < arc_lib.version)
        arc_formats[type] = arc_format;
    }
  }
  // unload unused libraries
  set<unsigned> used_libs;
  for_each(arc_formats.begin(), arc_formats.end(), [&] (const pair<ArcType, ArcFormat>& arc_format) {
    used_libs.insert(arc_format.second.lib_index);
  });
  for (unsigned i = 0; i < arc_libs.size(); i++) {
    if (used_libs.count(i) == 0) {
      FreeLibrary(arc_libs[i].h_module);
      arc_libs[i].h_module = nullptr;
    }
  }
}

void ArcAPI::create_in_archive(const ArcType& arc_type, IInArchive** in_arc) {
  CHECK_COM(libs()[formats().at(arc_type).lib_index].CreateObject(reinterpret_cast<const GUID*>(arc_type.data()), &IID_IInArchive, reinterpret_cast<void**>(in_arc)));
}

void ArcAPI::create_out_archive(const ArcType& arc_type, IOutArchive** out_arc) {
  CHECK_COM(libs()[formats().at(arc_type).lib_index].CreateObject(reinterpret_cast<const GUID*>(arc_type.data()), &IID_IOutArchive, reinterpret_cast<void**>(out_arc)));
}

void ArcAPI::free() {
  if (arc_api) {
    delete arc_api;
    arc_api = nullptr;
  }
}


wstring Archive::get_default_name() const {
  wstring name = arc_name();
  wstring ext = extract_file_ext(name);
  name.erase(name.size() - ext.size(), ext.size());
  if (arc_chain.empty())
    return name;
  const ArcType& arc_type = arc_chain.back().type;
  auto& nested_ext_mapping = ArcAPI::formats().at(arc_type).nested_ext_mapping;
  auto nested_ext_iter = nested_ext_mapping.find(upcase(ext));
  if (nested_ext_iter == nested_ext_mapping.end())
    return name;
  const wstring& nested_ext = nested_ext_iter->second;
  ext = extract_file_ext(name);
  if (upcase(nested_ext) == upcase(ext))
    return name;
  name.replace(name.size() - ext.size(), ext.size(), nested_ext);
  return name;
}

wstring Archive::get_temp_file_name() const {
  GUID guid;
  CHECK_COM(CoCreateGuid(&guid));
  wchar_t guid_str[50];
  CHECK(StringFromGUID2(guid, guid_str, ARRAYSIZE(guid_str)));
  return add_trailing_slash(arc_dir()) + guid_str + L".tmp";
}


bool ArcFileInfo::operator<(const ArcFileInfo& file_info) const {
  if (parent == file_info.parent)
    if (is_dir == file_info.is_dir)
      return lstrcmpiW(name.c_str(), file_info.name.c_str()) < 0;
    else
      return is_dir;
  else
    return parent < file_info.parent;
}


void Archive::make_index() {
  num_indices = 0;
  CHECK_COM(in_arc->GetNumberOfItems(&num_indices));
  file_list.clear();
  file_list.reserve(num_indices);

  struct DirInfo {
    UInt32 index;
    UInt32 parent;
    wstring name;
    bool operator<(const DirInfo& dir_info) const {
      if (parent == dir_info.parent)
        return lstrcmpiW(name.c_str(), dir_info.name.c_str()) < 0;
      else
        return parent < dir_info.parent;
    }
  };
  typedef set<DirInfo> DirList;
  map<UInt32, unsigned> dir_index_map;
  DirList dir_list;

  DirInfo dir_info;
  UInt32 dir_index = 0;
  ArcFileInfo file_info;
  wstring path;
  PropVariant prop;
  for (UInt32 i = 0; i < num_indices; i++) {
    // is directory?
    file_info.is_dir = in_arc->GetProperty(i, kpidIsDir, prop.ref()) == S_OK && prop.is_bool() && prop.get_bool();

    // file name
    if (in_arc->GetProperty(i, kpidPath, prop.ref()) == S_OK && prop.is_str())
      path.assign(prop.get_str());
    else
      path.assign(get_default_name());
    size_t name_end_pos = path.size();
    while (name_end_pos && is_slash(path[name_end_pos - 1])) name_end_pos--;
    size_t name_pos = name_end_pos;
    while (name_pos && !is_slash(path[name_pos - 1])) name_pos--;
    file_info.name.assign(path.data() + name_pos, name_end_pos - name_pos);

    // split path into individual directories and put them into DirList
    dir_info.parent = c_root_index;
    size_t begin_pos = 0;
    while (begin_pos < name_pos) {
      dir_info.index = dir_index;
      size_t end_pos = begin_pos;
      while (end_pos < name_pos && !is_slash(path[end_pos])) end_pos++;
      if (end_pos != begin_pos) {
        dir_info.name.assign(path.data() + begin_pos, end_pos - begin_pos);
        pair<DirList::iterator, bool> ins_pos = dir_list.insert(dir_info);
        if (ins_pos.second)
          dir_index++;
        dir_info.parent = ins_pos.first->index;
      }
      begin_pos = end_pos + 1;
    }
    file_info.parent = dir_info.parent;

    if (file_info.is_dir) {
      dir_info.index = dir_index;
      dir_info.parent = file_info.parent;
      dir_info.name = file_info.name;
      pair<DirList::iterator, bool> ins_pos = dir_list.insert(dir_info);
      if (ins_pos.second) {
        dir_index++;
        dir_index_map[dir_info.index] = i;
      }
      else {
        if (dir_index_map.count(ins_pos.first->index))
          file_info.parent = c_dup_index;
        else
          dir_index_map[ins_pos.first->index] = i;
      }
    }

    file_list.push_back(file_info);
  }

  // add directories that not present in archive index
  file_list.reserve(file_list.size() + dir_list.size() - dir_index_map.size());
  dir_index = num_indices;
  for_each(dir_list.begin(), dir_list.end(), [&] (const DirInfo& dir_info) {
    if (dir_index_map.count(dir_info.index) == 0) {
      dir_index_map[dir_info.index] = dir_index;
      file_info.parent = dir_info.parent;
      file_info.name = dir_info.name;
      file_info.is_dir = true;
      dir_index++;
      file_list.push_back(file_info);
    }
  });

  // fix parent references
  for_each(file_list.begin(), file_list.end(), [&] (ArcFileInfo& file_info) {
    if (file_info.parent != c_root_index)
      file_info.parent = dir_index_map[file_info.parent];
  });

  // create search index
  file_list_index.clear();
  file_list_index.reserve(file_list.size());
  for (UInt32 i = 0; i < file_list.size(); i++) {
    file_list_index.push_back(i);
  }
  sort(file_list_index.begin(), file_list_index.end(), [&] (UInt32 left, UInt32 right) -> bool {
    return file_list[left] < file_list[right];
  });

  load_arc_attr();
}

UInt32 Archive::find_dir(const wstring& path) {
  if (file_list.empty())
    make_index();

  ArcFileInfo dir_info;
  dir_info.is_dir = true;
  dir_info.parent = c_root_index;
  size_t begin_pos = 0;
  while (begin_pos < path.size()) {
    size_t end_pos = begin_pos;
    while (end_pos < path.size() && !is_slash(path[end_pos])) end_pos++;
    if (end_pos != begin_pos) {
      dir_info.name.assign(path.data() + begin_pos, end_pos - begin_pos);
      FileIndexRange fi_range = equal_range(file_list_index.begin(), file_list_index.end(), -1, [&] (UInt32 left, UInt32 right) -> bool {
        const ArcFileInfo& fi_left = left == -1 ? dir_info : file_list[left];
        const ArcFileInfo& fi_right = right == -1 ? dir_info : file_list[right];
        return fi_left < fi_right;
      });
      if (fi_range.first == fi_range.second)
        FAIL(HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND));
      dir_info.parent = *fi_range.first;
    }
    begin_pos = end_pos + 1;
  }
  return dir_info.parent;
}

FileIndexRange Archive::get_dir_list(UInt32 dir_index) {
  if (file_list.empty())
    make_index();

  ArcFileInfo file_info;
  file_info.parent = dir_index;
  FileIndexRange index_range = equal_range(file_list_index.begin(), file_list_index.end(), -1, [&] (UInt32 left, UInt32 right) -> bool {
    const ArcFileInfo& fi_left = left == -1 ? file_info : file_list[left];
    const ArcFileInfo& fi_right = right == -1 ? file_info : file_list[right];
    return fi_left.parent < fi_right.parent;
  });

  return index_range;
}

wstring Archive::get_path(UInt32 index) {
  if (file_list.empty())
    make_index();

  wstring file_path = file_list[index].name;
  UInt32 parent = file_list[index].parent;
  while (parent != c_root_index) {
    file_path.insert(0, 1, L'\\').insert(0, file_list[parent].name);
    parent = file_list[parent].parent;
  }
  return file_path;
}

FindData Archive::get_file_info(UInt32 index) {
  if (file_list.empty())
    make_index();

  FindData file_info;
  memzero(file_info);
  wcscpy(file_info.cFileName, file_list[index].name.c_str());
  file_info.dwFileAttributes = get_attr(index);
  file_info.set_size(get_size(index));
  file_info.ftCreationTime = get_ctime(index);
  file_info.ftLastWriteTime = get_mtime(index);
  file_info.ftLastAccessTime = get_atime(index);
  return file_info;
}

bool Archive::get_main_file(UInt32& index) const {
  PropVariant prop;
  if (in_arc->GetArchiveProperty(kpidMainSubfile, prop.ref()) != S_OK || prop.vt != VT_UI4)
    return false;
  index = prop.ulVal;
  return true;
}

DWORD Archive::get_attr(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return FILE_ATTRIBUTE_DIRECTORY;
  else if (in_arc->GetProperty(index, kpidAttrib, prop.ref()) == S_OK && prop.is_uint())
    return static_cast<DWORD>(prop.get_uint());
  else if (file_list[index].is_dir)
    return FILE_ATTRIBUTE_DIRECTORY;
  else
    return 0;
}

unsigned __int64 Archive::get_size(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return 0;
  else if (!file_list[index].is_dir && in_arc->GetProperty(index, kpidSize, prop.ref()) == S_OK && prop.is_uint())
    return prop.get_uint();
  else
    return 0;
}

unsigned __int64 Archive::get_psize(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return 0;
  else if (!file_list[index].is_dir && in_arc->GetProperty(index, kpidPackSize, prop.ref()) == S_OK && prop.is_uint())
    return prop.get_uint();
  else
    return 0;
}

FILETIME Archive::get_ctime(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return arc_info.ftCreationTime;
  else if (in_arc->GetProperty(index, kpidCTime, prop.ref()) == S_OK && prop.is_filetime())
    return prop.get_filetime();
  else
    return arc_info.ftCreationTime;
}

FILETIME Archive::get_mtime(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return arc_info.ftLastWriteTime;
  else if (in_arc->GetProperty(index, kpidMTime, prop.ref()) == S_OK && prop.is_filetime())
    return prop.get_filetime();
  else
    return arc_info.ftLastWriteTime;
}

FILETIME Archive::get_atime(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return arc_info.ftLastAccessTime;
  else if (in_arc->GetProperty(index, kpidATime, prop.ref()) == S_OK && prop.is_filetime())
    return prop.get_filetime();
  else
    return arc_info.ftLastAccessTime;
}

unsigned Archive::get_crc(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return 0;
  else if (in_arc->GetProperty(index, kpidCRC, prop.ref()) == S_OK && prop.is_uint())
    return static_cast<DWORD>(prop.get_uint());
  else
    return 0;
}
