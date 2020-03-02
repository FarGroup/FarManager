#include "msg.h"
#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "ui.hpp"
#include "archive.hpp"
#include "options.hpp"

const wchar_t* c_method_copy   = L"Copy";

const wchar_t* c_method_lzma   = L"LZMA";
const wchar_t* c_method_lzma2  = L"LZMA2";
const wchar_t* c_method_ppmd   = L"PPMD";

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
DEFINE_ARC_ID(SWFc, "\xD8")
DEFINE_ARC_ID(dmg, "\xE4")
DEFINE_ARC_ID(hfs, "\xE3")

#undef DEFINE_ARC_ID

const UInt64 c_min_volume_size = 16 * 1024;

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

HRESULT ArcLib::get_uint_prop(UInt32 index, PROPID prop_id, UInt32& value) const {
  PropVariant prop;
  HRESULT res = get_prop(index, prop_id, prop.ref());
  if (res != S_OK)
    return res;
  if (!prop.is_uint())
    return E_FAIL;
  value = (UInt32)prop.get_uint();
  return S_OK;
}

HRESULT ArcLib::get_string_prop(UInt32 index, PROPID prop_id, std::wstring& value) const {
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


std::wstring ArcFormat::default_extension() const {
  if (extension_list.empty())
    return std::wstring();
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

ArcTypes ArcFormats::find_by_name(const std::wstring& name) const {
  ArcTypes types;
  std::wstring uc_name = upcase(name);
  for (const_iterator fmt = begin(); fmt != end(); fmt++) {
    if (upcase(fmt->second.name) == uc_name)
      types.push_back(fmt->first);
  }
  return types;
}

ArcTypes ArcFormats::find_by_ext(const std::wstring& ext) const {
  ArcTypes types;
  if (ext.empty())
    return types;
  std::wstring uc_ext = upcase(ext);
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


uintptr_t SfxModules::find_by_name(const std::wstring& name) const {
  for (const_iterator sfx_module = begin(); sfx_module != end(); sfx_module++) {
    if (upcase(extract_file_name(sfx_module->path)) == upcase(name))
      return distance(begin(), sfx_module);
  }
  return size();
}


std::wstring ArcChain::to_string() const {
  std::wstring result;
  std::for_each(begin(), end(), [&] (const ArcEntry& arc) {
    if (!result.empty())
      result += L"\x2192";
    result += ArcAPI::formats().at(arc.type).name;
  });
  return result;
}

static bool GetCoderInfo(Func_GetMethodProperty getMethodProperty, UInt32 index, CDllCodecInfo& info) {
  info.DecoderIsAssigned = info.EncoderIsAssigned = false;
  std::fill((char *)&info.Decoder, sizeof(info.Decoder) + (char *)&info.Decoder, 0);
  info.Encoder = info.Decoder;
  PropVariant prop1, prop2, prop3, prop4;
  if (S_OK != getMethodProperty(index, NMethodPropID::kDecoder, prop1.ref()))
    return false;
  if (prop1.vt != VT_EMPTY) {
    if (prop1.vt != VT_BSTR || (size_t)SysStringByteLen(prop1.bstrVal) < sizeof(CLSID))
      return false;
    info.Decoder = *(const GUID *)prop1.bstrVal;
    info.DecoderIsAssigned = true;
  }
  if (S_OK != getMethodProperty(index, NMethodPropID::kEncoder, prop2.ref()))
    return false;
  if (prop2.vt != VT_EMPTY) {
    if (prop2.vt != VT_BSTR || (size_t)SysStringByteLen(prop2.bstrVal) < sizeof(CLSID))
      return false;
    info.Encoder = *(const GUID *)prop2.bstrVal;
    info.EncoderIsAssigned = true;
  }
  if (S_OK != getMethodProperty(index, NMethodPropID::kName, prop3.ref()) || !prop3.is_str())
    return false;
  info.Name = prop3.get_str();
  if (S_OK != getMethodProperty(index, NMethodPropID::kID, prop4.ref()) || !prop4.is_uint())
    return false;
  info.CodecId = static_cast<UInt32>(prop4.get_uint());
  return info.DecoderIsAssigned || info.EncoderIsAssigned;
}

class MyCompressCodecsInfo : public ICompressCodecsInfo, public IHashers, private ComBase {
private:
  const ArcLibs& libs_;
  ArcCodecs codecs_;
  ArcHashers hashers_;
public:
  MyCompressCodecsInfo(const ArcLibs& libs, const ArcCodecs& codecs, const ArcHashers& hashers, size_t skip_lib_index)
   : libs_(libs)
  {
    for (size_t ilib = 0; ilib < 1; ++ilib) { // 7z.dll only
      if (ilib == skip_lib_index)
        continue;
      const auto& arc_lib = libs[ilib];
      if ((arc_lib.CreateObject || arc_lib.CreateDecoder || arc_lib.CreateEncoder) && arc_lib.GetMethodProperty) {
        UInt32 numMethods = 1;
        bool ok = true;
        if (arc_lib.GetNumberOfMethods)
          ok = S_OK == arc_lib.GetNumberOfMethods(&numMethods);
        for (UInt32 i = 0; ok && i < numMethods; ++i) {
          CDllCodecInfo info;
          info.LibIndex = static_cast<UInt32>(ilib);
          info.CodecIndex = i;
          if (GetCoderInfo(arc_lib.GetMethodProperty, i, info))
            codecs_.push_back(info);
        }
      }
      if (arc_lib.ComHashers) {
        UInt32 numHashers = arc_lib.ComHashers->GetNumHashers();
        for (UInt32 i = 0; i < numHashers; ++i) {
          CDllHasherInfo info;
          info.LibIndex = static_cast<UInt32>(ilib);
          info.HasherIndex = i;
          hashers_.push_back(info);
        }
      }
    }
    for (const auto& codec : codecs) {
      if (codec.LibIndex != skip_lib_index)
        codecs_.push_back(codec);
    }
    for (const auto& hasher : hashers) {
      if (hasher.LibIndex != skip_lib_index)
        hashers_.push_back(hasher);
    }
  }

  ~MyCompressCodecsInfo() {}

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(ICompressCodecsInfo)
  UNKNOWN_IMPL_ITF(IHashers)
  UNKNOWN_IMPL_END

  STDMETHODIMP_(UInt32) GetNumHashers() {
    return static_cast<UInt32>(hashers_.size());
  }

  STDMETHODIMP GetHasherProp(UInt32 index, PROPID propID, PROPVARIANT *value) {
    const CDllHasherInfo &hi = hashers_[index];
    const auto &lib = libs_[hi.LibIndex];
    return lib.ComHashers->GetHasherProp(hi.HasherIndex, propID, value);
  }

  STDMETHODIMP CreateHasher(UInt32 index, IHasher **hasher) {
    const CDllHasherInfo &hi = hashers_[index];
    const auto &lib = libs_[hi.LibIndex];
    return lib.ComHashers->CreateHasher(hi.HasherIndex, hasher);
  }

  STDMETHODIMP GetNumMethods(UInt32 *numMethods) {
    *numMethods = static_cast<UInt32>(codecs_.size());
    return S_OK;
  }

  STDMETHODIMP GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value) {
    const CDllCodecInfo &ci = codecs_[index];
    if (propID == NMethodPropID::kDecoderIsAssigned || propID == NMethodPropID::kEncoderIsAssigned) {
      PropVariant prop;
      prop = (bool)((propID == NMethodPropID::kDecoderIsAssigned) ? ci.DecoderIsAssigned : ci.EncoderIsAssigned);
      prop.detach(value);
      return S_OK;
    }
    const auto &lib = libs_[ci.LibIndex];
    return lib.GetMethodProperty(ci.CodecIndex, propID, value);
  }

  STDMETHODIMP CreateDecoder(UInt32 index, const GUID *iid, void **coder) {
    const CDllCodecInfo &ci = codecs_[index];
    if (ci.DecoderIsAssigned) {
      const auto &lib = libs_[ci.LibIndex];
      if (lib.CreateDecoder)
        return lib.CreateDecoder(ci.CodecIndex, iid, (void **)coder);
      else
        return lib.CreateObject(&ci.Decoder, iid, (void **)coder);
    }
    return S_OK;
  }

  STDMETHODIMP CreateEncoder(UInt32 index, const GUID *iid, void **coder) {
    const CDllCodecInfo &ci = codecs_[index];
    if (ci.EncoderIsAssigned) {
      const auto &lib = libs_[ci.LibIndex];
      if (lib.CreateEncoder)
        return lib.CreateEncoder(ci.CodecIndex, iid, (void **)coder);
      else
        return lib.CreateObject(&ci.Encoder, iid, (void **)coder);
    }
    return S_OK;
  }
};

ArcAPI* ArcAPI::arc_api = nullptr;

ArcAPI::~ArcAPI() {
  for (auto& arc_lib : arc_libs) {
    if (arc_lib.h_module && arc_lib.SetCodecs)
      arc_lib.SetCodecs(nullptr); // calls ~MyCompressInfo()
  }
  for (auto arc_lib = arc_libs.rbegin(); arc_lib != arc_libs.rend(); ++arc_lib) {
    if (arc_lib->h_module) {
      arc_lib->ComHashers = nullptr;
      FreeLibrary(arc_lib->h_module);
    }
  }
}

ArcAPI* ArcAPI::get() {
  if (arc_api == nullptr) {
    arc_api = new ArcAPI();
    arc_api->load();
    Patch7zCP::SetCP(static_cast<UINT>(g_options.oemCP), static_cast<UINT>(g_options.ansiCP));
  }
  return arc_api;
}

void ArcAPI::load_libs(const std::wstring& path) {
  FileEnum file_enum(path);
  std::wstring dir = extract_file_path(path);
  bool more;
  while (file_enum.next_nt(more) && more) {
    ArcLib arc_lib;
    arc_lib.module_path = add_trailing_slash(dir) + file_enum.data().cFileName;
    arc_lib.h_module = LoadLibraryW(arc_lib.module_path.c_str());
    if (arc_lib.h_module == nullptr)
      continue;
    arc_lib.CreateObject = reinterpret_cast<Func_CreateObject>(GetProcAddress(arc_lib.h_module, "CreateObject"));
    arc_lib.CreateDecoder = reinterpret_cast<Func_CreateDecoder>(GetProcAddress(arc_lib.h_module, "CreateDecoder"));
    arc_lib.CreateEncoder = reinterpret_cast<Func_CreateEncoder>(GetProcAddress(arc_lib.h_module, "CreateEncoder"));
    arc_lib.GetNumberOfMethods = reinterpret_cast<Func_GetNumberOfMethods>(GetProcAddress(arc_lib.h_module, "GetNumberOfMethods"));
    arc_lib.GetMethodProperty = reinterpret_cast<Func_GetMethodProperty>(GetProcAddress(arc_lib.h_module, "GetMethodProperty"));
    arc_lib.GetNumberOfFormats = reinterpret_cast<Func_GetNumberOfFormats>(GetProcAddress(arc_lib.h_module, "GetNumberOfFormats"));
    arc_lib.GetHandlerProperty = reinterpret_cast<Func_GetHandlerProperty>(GetProcAddress(arc_lib.h_module, "GetHandlerProperty"));
    arc_lib.GetHandlerProperty2 = reinterpret_cast<Func_GetHandlerProperty2>(GetProcAddress(arc_lib.h_module, "GetHandlerProperty2"));
    arc_lib.GetIsArc = reinterpret_cast<Func_GetIsArc>(GetProcAddress(arc_lib.h_module, "GetIsArc"));
    arc_lib.SetCodecs = reinterpret_cast<Func_SetCodecs>(GetProcAddress(arc_lib.h_module, "SetCodecs"));
    if (arc_lib.CreateObject && ((arc_lib.GetNumberOfFormats && arc_lib.GetHandlerProperty2) || arc_lib.GetHandlerProperty)) {
      arc_lib.version = get_module_version(arc_lib.module_path);
      Func_GetHashers getHashers = reinterpret_cast<Func_GetHashers>(GetProcAddress(arc_lib.h_module, "GetHashers"));
      if (getHashers) {
        IHashers *hashers = nullptr;
        if (S_OK == getHashers(&hashers) && hashers)
          arc_lib.ComHashers = hashers;
      }
      arc_libs.push_back(arc_lib);
    }
    else
      FreeLibrary(arc_lib.h_module);
  }
}

void ArcAPI::load_codecs(const std::wstring& path) {
  if (n_base_format_libs <= 0)
    return;

  const auto& add_codecs = [this](ArcLib &arc_lib, size_t lib_index) {
    if ((arc_lib.CreateObject || arc_lib.CreateDecoder || arc_lib.CreateEncoder) && arc_lib.GetMethodProperty) {
      UInt32 numMethods = 1;
      bool ok = true;
      if (arc_lib.GetNumberOfMethods)
        ok = S_OK == arc_lib.GetNumberOfMethods(&numMethods);
      for (UInt32 i = 0; ok && i < numMethods; ++i) {
        CDllCodecInfo info;
        info.LibIndex = static_cast<UInt32>(lib_index);
        info.CodecIndex = i;
        if (!GetCoderInfo(arc_lib.GetMethodProperty, i, info))
          return;
        for (const auto& codec : arc_codecs)
          if (codec.Name == info.Name)
            return;
        arc_codecs.push_back(info);
      }
    }
  };

  const auto& add_hashers = [this](ArcLib &arc_lib, size_t lib_index) {
    if (arc_lib.ComHashers) {
      UInt32 numHashers = arc_lib.ComHashers->GetNumHashers();
      for (UInt32 i = 0; i < numHashers; i++) {
        CDllHasherInfo info;
        info.LibIndex = static_cast<UInt32>(lib_index);
        info.HasherIndex = i;
        arc_hashers.push_back(info);
      }
    }
  };

  for (size_t ii = 1; ii < n_format_libs; ++ii) { // all but 7z.dll
    auto& arc_lib = arc_libs[ii];
    add_codecs(arc_lib, ii);
    add_hashers(arc_lib, ii);
  }

  FileEnum codecs_enum(path);
  std::wstring dir = extract_file_path(path);
  bool more;
  while (codecs_enum.next_nt(more) && more && !codecs_enum.data().is_dir()) {
    ArcLib arc_lib;
    arc_lib.module_path = add_trailing_slash(dir) + codecs_enum.data().cFileName;
    arc_lib.h_module = LoadLibraryW(arc_lib.module_path.c_str());
    if (arc_lib.h_module == nullptr)
      continue;
    arc_lib.CreateObject = reinterpret_cast<Func_CreateObject>(GetProcAddress(arc_lib.h_module, "CreateObject"));
    arc_lib.CreateDecoder = reinterpret_cast<Func_CreateDecoder>(GetProcAddress(arc_lib.h_module, "CreateDecoder"));
    arc_lib.CreateEncoder = reinterpret_cast<Func_CreateEncoder>(GetProcAddress(arc_lib.h_module, "CreateEncoder"));
    arc_lib.GetNumberOfMethods = reinterpret_cast<Func_GetNumberOfMethods>(GetProcAddress(arc_lib.h_module, "GetNumberOfMethods"));
    arc_lib.GetMethodProperty = reinterpret_cast<Func_GetMethodProperty>(GetProcAddress(arc_lib.h_module, "GetMethodProperty"));
    arc_lib.GetNumberOfFormats = nullptr;
    arc_lib.GetHandlerProperty = nullptr;
    arc_lib.GetHandlerProperty2 = nullptr;
    arc_lib.GetIsArc = nullptr;
    arc_lib.SetCodecs = nullptr;
    arc_lib.version = 0;
    auto n_start_codecs = arc_codecs.size();
    auto n_start_hashers = arc_hashers.size();
    add_codecs(arc_lib, arc_libs.size());
    Func_GetHashers getHashers = reinterpret_cast<Func_GetHashers>(GetProcAddress(arc_lib.h_module, "GetHashers"));
    if (getHashers) {
      IHashers *hashers = nullptr;
      if (S_OK == getHashers(&hashers) && hashers) {
        arc_lib.ComHashers = hashers;
        add_hashers(arc_lib, arc_libs.size());
      }
    }
    if (n_start_codecs < arc_codecs.size() || n_start_hashers < arc_hashers.size())
      arc_libs.push_back(arc_lib);
    else
      FreeLibrary(arc_lib.h_module);
  }

  n_7z_codecs = 0;
  if (arc_codecs.size() > 0) {
    std::sort(arc_codecs.begin(), arc_codecs.end(), [&](const auto&a, const auto& b) {
      bool a_is_zip = (a.CodecId & 0xffffff00U) == 0x040100;
      bool b_is_zip = (b.CodecId & 0xffffff00U) == 0x040100;
      if (a_is_zip != b_is_zip)
        return b_is_zip;
      else
        return _wcsicmp(a.Name.c_str(), b.Name.c_str()) < 0;
    });
    for (const auto& c : arc_codecs) { if ((c.CodecId & 0xffffff00U) != 0x040100) ++n_7z_codecs; }
  }
  for (size_t i = 0; i < n_format_libs; ++i) {
    if (arc_libs[i].SetCodecs) {
      auto compressinfo = new MyCompressCodecsInfo(arc_libs, arc_codecs, arc_hashers, i);
      UInt32 nm = 0, nh = compressinfo->GetNumHashers();
      compressinfo->GetNumMethods(&nm);
      if (nm > 0 || nh > 0) {
        arc_libs[i].SetCodecs(compressinfo);
      }
      else {
        delete compressinfo;
      }
    }
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

const SfxModuleInfo* find(const std::wstring& path) {
  unsigned i = 0;
  for (; i < ARRAYSIZE(c_known_sfx_modules) && upcase(extract_file_name(path)) != upcase(c_known_sfx_modules[i].module_name); i++);
  if (i < ARRAYSIZE(c_known_sfx_modules))
    return c_known_sfx_modules + i;
  else
    return nullptr;
}

std::wstring SfxModule::description() const {
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

void ArcAPI::find_sfx_modules(const std::wstring& path) {
  FileEnum file_enum(path);
  std::wstring dir = extract_file_path(path);
  bool more;
  while (file_enum.next_nt(more) && more) {
    SfxModule sfx_module;
    sfx_module.path = add_trailing_slash(dir) + file_enum.data().cFileName;
    File file;
    if (!file.open_nt(sfx_module.path, FILE_READ_DATA, FILE_SHARE_READ, OPEN_EXISTING, 0))
      continue;
    Buffer<char> buffer(2);
    size_t sz;
    if (!file.read_nt(buffer.data(), buffer.size(), sz))
      continue;
    std::string sig(buffer.data(), sz);
    if (sig != "MZ")
      continue;
    sfx_modules.push_back(sfx_module);
  }
}

static bool ParseSignatures(const Byte *data, size_t size, std::vector<ByteVector> &signatures)
{
  signatures.clear();
  while (size > 0)
  {
    size_t len = *data++;
    size--;
    if (len > size)
      return false;

    ByteVector v(data, data+len);
    signatures.push_back(v);

    data += len;
    size -= len;
  }
  return true;
}

void ArcAPI::load() {
  auto dll_path = add_trailing_slash(Far::get_plugin_module_path());
  load_libs(dll_path + L"*.dll");
  find_sfx_modules(dll_path + L"*.sfx");
  if (arc_libs.empty() || sfx_modules.empty()) {
    std::wstring _7zip_path;
    Key _7zip_key;
    _7zip_key.open_nt(HKEY_CURRENT_USER, L"Software\\7-Zip", KEY_QUERY_VALUE, false) && _7zip_key.query_str_nt(_7zip_path, L"Path");
    if (_7zip_path.empty())
      _7zip_key.open_nt(HKEY_LOCAL_MACHINE, L"Software\\7-Zip", KEY_QUERY_VALUE, false) && _7zip_key.query_str_nt(_7zip_path, L"Path");
    if (!_7zip_path.empty()) {
      _7zip_path = add_trailing_slash(_7zip_path);
      if (arc_libs.empty()) {
        dll_path = _7zip_path;
        load_libs(_7zip_path + L"7z.dll");
      }
      if (!arc_libs.empty() && sfx_modules.empty())
        find_sfx_modules(_7zip_path + L"*.sfx");
    }
  }
  if (arc_libs.empty()) {
    std::wstring _7z_dll_path;
    IGNORE_ERRORS(_7z_dll_path = search_path(L"7z.dll"));
    if (!_7z_dll_path.empty()) {
      load_libs(_7z_dll_path);
      if (!arc_libs.empty()) {
        dll_path = add_trailing_slash(extract_file_path(_7z_dll_path));
        if (sfx_modules.empty())
          find_sfx_modules(dll_path + L"*.sfx");
      }
    }
  }

  n_base_format_libs = n_format_libs = arc_libs.size();
  load_libs(dll_path + L"Formats\\*.format");
  n_format_libs = arc_libs.size();
  load_codecs(dll_path + L"Codecs\\*.codec");

  for (unsigned i = 0; i < n_format_libs; i++) {
    const ArcLib& arc_lib = arc_libs[i];

    UInt32 num_formats;
    if (arc_lib.GetNumberOfFormats) {
      if (arc_lib.GetNumberOfFormats(&num_formats) != S_OK)
        num_formats = 0;
    }
    else
      num_formats = 1;

    for (UInt32 idx = 0; idx < num_formats; idx++) {
      ArcFormat format;

      if (arc_lib.get_bytes_prop(idx, NArchive::NHandlerPropID::kClassID, format.ClassID) != S_OK)
        continue;

      arc_lib.get_string_prop(idx, NArchive::NHandlerPropID::kName, format.name);
      if (arc_lib.get_bool_prop(idx, NArchive::NHandlerPropID::kUpdate, format.updatable) != S_OK)
        format.updatable = false;

      std::wstring extension_list_str;
      arc_lib.get_string_prop(idx, NArchive::NHandlerPropID::kExtension, extension_list_str);
      format.extension_list = split(extension_list_str, L' ');
      std::wstring add_extension_list_str;
      arc_lib.get_string_prop(idx, NArchive::NHandlerPropID::kAddExtension, add_extension_list_str);
      std::list<std::wstring> add_extension_list = split(add_extension_list_str, L' ');
      auto add_ext_iter = add_extension_list.cbegin();
      for (auto ext_iter = format.extension_list.begin(); ext_iter != format.extension_list.end(); ++ext_iter) {
        ext_iter->insert(0, 1, L'.');
        if (add_ext_iter != add_extension_list.cend()) {
          if (*add_ext_iter != L"*") {
            format.nested_ext_mapping[upcase(*ext_iter)] = *add_ext_iter;
          }
          ++add_ext_iter;
        }
      }

      format.lib_index = (int)i;
      format.FormatIndex = idx;

      format.NewInterface = arc_lib.get_uint_prop(idx, NArchive::NHandlerPropID::kFlags, format.Flags) == S_OK;
      if (!format.NewInterface) { // support for DLL version < 9.31:
        bool v = false;
        if (arc_lib.get_bool_prop(idx, NArchive::NHandlerPropID::kKeepName, v) != S_OK && v)
          format.Flags |= NArcInfoFlags::kKeepName;
        if (arc_lib.get_bool_prop(idx, NArchive::NHandlerPropID::kAltStreams, v) != S_OK && v)
          format.Flags |= NArcInfoFlags::kAltStreams;
        if (arc_lib.get_bool_prop(idx, NArchive::NHandlerPropID::kNtSecure, v) != S_OK && v)
          format.Flags |= NArcInfoFlags::kNtSecure;
      }

      ByteVector sig;
      arc_lib.get_bytes_prop(idx, NArchive::NHandlerPropID::kSignature, sig);
      if (!sig.empty())
        format.Signatures.push_back(sig);
      else {
        arc_lib.get_bytes_prop(idx, NArchive::NHandlerPropID::kMultiSignature, sig);
        ParseSignatures(sig.data(), sig.size(), format.Signatures);
      }

      if (arc_lib.get_uint_prop(idx, NArchive::NHandlerPropID::kSignatureOffset, format.SignatureOffset) != S_OK)
        format.SignatureOffset = 0;

      if (arc_lib.GetIsArc)
        arc_lib.GetIsArc(idx, &format.IsArc);

      ArcFormats::const_iterator existing_format = arc_formats.find(format.ClassID);
      if (existing_format == arc_formats.end() || arc_libs[existing_format->second.lib_index].version < arc_lib.version)
        arc_formats[format.ClassID] = format;
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


std::wstring Archive::get_default_name() const {
  std::wstring name = arc_name();
  std::wstring ext = extract_file_ext(name);
  name.erase(name.size() - ext.size(), ext.size());
  if (arc_chain.empty())
    return name;
  const ArcType& arc_type = arc_chain.back().type;
  auto& nested_ext_mapping = ArcAPI::formats().at(arc_type).nested_ext_mapping;
  auto nested_ext_iter = nested_ext_mapping.find(upcase(ext));
  if (nested_ext_iter == nested_ext_mapping.end())
    return name;
  const std::wstring& nested_ext = nested_ext_iter->second;
  ext = extract_file_ext(name);
  if (upcase(nested_ext) == upcase(ext))
    return name;
  name.replace(name.size() - ext.size(), ext.size(), nested_ext);
  return name;
}

std::wstring Archive::get_temp_file_name() const {
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
    std::wstring name;
    bool operator<(const DirInfo& dir_info) const {
      if (parent == dir_info.parent)
        return lstrcmpiW(name.c_str(), dir_info.name.c_str()) < 0;
      else
        return parent < dir_info.parent;
    }
  };
  typedef std::set<DirInfo> DirList;
  std::map<UInt32, unsigned> dir_index_map;
  DirList dir_list;

  DirInfo dir_info;
  UInt32 dir_index = 0;
  ArcFileInfo file_info;
  std::wstring path;
  PropVariant prop;
  for (UInt32 i = 0; i < num_indices; i++) {
    // is directory?
    file_info.is_dir = in_arc->GetProperty(i, kpidIsDir, prop.ref()) == S_OK && prop.is_bool() && prop.get_bool();
    file_info.is_altstream = get_isaltstream(i);

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
    std::stack<UInt32> dir_parents;
    size_t begin_pos = 0;
    while (begin_pos < name_pos) {
      dir_info.index = dir_index;
      size_t end_pos = begin_pos;
      while (end_pos < name_pos && !is_slash(path[end_pos])) end_pos++;
      if (end_pos != begin_pos) {
        dir_info.name=path.substr(begin_pos, end_pos - begin_pos);
        if(dir_info.name == L"..")
        {
          if(!dir_parents.empty())
          {
            dir_info.parent = dir_parents.top();
            dir_parents.pop();
          }
        }
        else if(dir_info.name != L".")
        {
          std::pair<DirList::iterator, bool> ins_pos = dir_list.insert(dir_info);
          if (ins_pos.second)
            dir_index++;
          dir_parents.push(dir_info.parent);
          dir_info.parent = ins_pos.first->index;
        }
      }
      begin_pos = end_pos + 1;
    }
    file_info.parent = dir_info.parent;

    if (file_info.is_dir) {
      dir_info.index = dir_index;
      dir_info.parent = file_info.parent;
      dir_info.name = file_info.name;
      std::pair<DirList::iterator, bool> ins_pos = dir_list.insert(dir_info);
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
  std::for_each(dir_list.begin(), dir_list.end(), [&] (const DirInfo& dir_info) {
    if (dir_index_map.count(dir_info.index) == 0) {
      dir_index_map[dir_info.index] = dir_index;
      file_info.parent = dir_info.parent;
      file_info.name = dir_info.name;
      file_info.is_dir = true;
      file_info.is_altstream = false;
      dir_index++;
      file_list.push_back(file_info);
    }
  });

  // fix parent references
  std::for_each(file_list.begin(), file_list.end(), [&] (ArcFileInfo& file_info) {
    if (file_info.parent != c_root_index && file_info.parent != c_dup_index)
      file_info.parent = dir_index_map[file_info.parent];
  });

  // create search index
  file_list_index.clear();
  file_list_index.reserve(file_list.size());
  for (UInt32 i = 0; i < file_list.size(); i++) {
    file_list_index.push_back(i);
  }
  std::sort(file_list_index.begin(), file_list_index.end(), [&] (UInt32 left, UInt32 right) -> bool {
    return file_list[left] < file_list[right];
  });

  load_arc_attr();
}

UInt32 Archive::find_dir(const std::wstring& path) {
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
      FileIndexRange fi_range = std::equal_range(file_list_index.begin(), file_list_index.end(), -1, [&] (UInt32 left, UInt32 right) -> bool {
        const ArcFileInfo& fi_left = left == (UInt32)-1 ? dir_info : file_list[left];
        const ArcFileInfo& fi_right = right == (UInt32)-1 ? dir_info : file_list[right];
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
  FileIndexRange index_range = std::equal_range(file_list_index.begin(), file_list_index.end(), -1, [&] (UInt32 left, UInt32 right) -> bool {
    const ArcFileInfo& fi_left = left == (UInt32)-1 ? file_info : file_list[left];
    const ArcFileInfo& fi_right = right == (UInt32)-1 ? file_info : file_list[right];
    return fi_left.parent < fi_right.parent;
  });

  return index_range;
}

std::wstring Archive::get_path(UInt32 index) {
  if (file_list.empty())
    make_index();

  std::wstring file_path = file_list[index].name;
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

  FindData file_info{};
  std::wcscpy(file_info.cFileName, file_list[index].name.c_str());
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
  DWORD attr = 0;

  if (index >= num_indices)
    return FILE_ATTRIBUTE_DIRECTORY;

  if (in_arc->GetProperty(index, kpidAttrib, prop.ref()) == S_OK && prop.is_uint())
    attr = get_posix_and_nt_attributes(static_cast<DWORD>(prop.get_uint())).second;

  if (file_list[index].is_dir)
    attr |= FILE_ATTRIBUTE_DIRECTORY;

  return attr;
}

UInt64 Archive::get_size(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return 0;
  else if (!file_list[index].is_dir && in_arc->GetProperty(index, kpidSize, prop.ref()) == S_OK && prop.is_uint())
    return prop.get_uint();
  else
    return 0;
}

UInt64 Archive::get_psize(UInt32 index) const {
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

bool Archive::get_anti(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return false;
  else if (in_arc->GetProperty(index, kpidIsAnti, prop.ref()) == S_OK && prop.is_bool())
    return prop.get_bool();
  else
    return false;
}

bool Archive::get_isaltstream(UInt32 index) const {
  PropVariant prop;
  if (index >= num_indices)
    return false;
  else if (in_arc->GetProperty(index, kpidIsAltStream, prop.ref()) == S_OK && prop.is_bool())
    return prop.get_bool();
  else
    return false;
}

void Archive::read_open_results()
{
  PropVariant prop;

  error_flags = 0;
  if (in_arc->GetArchiveProperty(kpidErrorFlags, prop.ref()) == S_OK && (prop.vt == VT_UI4 || prop.vt == VT_UI8))
    error_flags = static_cast<UInt32>(prop.get_uint());

  error_text.clear();
  if (in_arc->GetArchiveProperty(kpidError, prop.ref()) == S_OK && prop.is_str())
    error_text = prop.get_str();

  warning_flags = 0;
  if (in_arc->GetArchiveProperty(kpidWarningFlags, prop.ref()) == S_OK && (prop.vt == VT_UI4 || prop.vt == VT_UI8))
    warning_flags = static_cast<UInt32>(prop.get_uint());

  warning_text.clear();
  if (in_arc->GetArchiveProperty(kpidWarning, prop.ref()) == S_OK && prop.is_str())
    warning_text = prop.get_str();

  UInt64 phy_size = 0;
  if (in_arc->GetArchiveProperty(kpidPhySize, prop.ref()) == S_OK && prop.is_size())
    phy_size = prop.get_size();

  if (phy_size) {
    UInt64 offset = 0;
    if (in_arc->GetArchiveProperty(kpidOffset, prop.ref()) == S_OK && prop.is_size())
      offset = prop.get_size();
    auto file_size = archive_filesize();
    auto end_pos = offset + phy_size;
    if (end_pos < file_size)
      warning_flags |= kpv_ErrorFlags_DataAfterEnd;
    else if (end_pos > file_size)
      error_flags |= kpv_ErrorFlags_UnexpectedEnd;
  }
}

static std::list<std::wstring> flags2texts(UInt32 flags)
{
  std::list<std::wstring> texts;
  if (flags != 0) {
    if ((flags & kpv_ErrorFlags_IsNotArc) == kpv_ErrorFlags_IsNotArc) { // 1
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_IS_NOT_ARCHIVE));
      flags &= ~kpv_ErrorFlags_IsNotArc;
    }
    if ((flags & kpv_ErrorFlags_HeadersError) == kpv_ErrorFlags_HeadersError) { // 2
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_HEADERS_ERROR));
      flags &= ~kpv_ErrorFlags_HeadersError;
    }
    if ((flags & kpv_ErrorFlags_EncryptedHeadersError) == kpv_ErrorFlags_EncryptedHeadersError) { // 4
      texts.push_back(L"EncryptedHeadersError"); // TODO: localize
      flags &= ~kpv_ErrorFlags_EncryptedHeadersError;
    }
    if ((flags & kpv_ErrorFlags_UnavailableStart) == kpv_ErrorFlags_UnavailableStart) { // 8
      //errors.push_back(L"UnavailableStart"); // TODO: localize
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_UNAVAILABLE_DATA));
      flags &= ~kpv_ErrorFlags_UnavailableStart;
    }
    if ((flags & kpv_ErrorFlags_UnconfirmedStart) == kpv_ErrorFlags_UnconfirmedStart) { // 16
      texts.push_back(L"UnconfirmedStart"); // TODO: localize
      flags &= ~kpv_ErrorFlags_UnconfirmedStart;
    }
    if ((flags & kpv_ErrorFlags_UnexpectedEnd) == kpv_ErrorFlags_UnexpectedEnd) { // 32
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_UNEXPECTED_END_DATA));
      flags &= ~kpv_ErrorFlags_UnexpectedEnd;
    }
    if ((flags & kpv_ErrorFlags_DataAfterEnd) == kpv_ErrorFlags_DataAfterEnd) { // 64
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_DATA_AFTER_END));
      flags &= ~kpv_ErrorFlags_DataAfterEnd;
    }
    if ((flags & kpv_ErrorFlags_UnsupportedMethod) == kpv_ErrorFlags_UnsupportedMethod) { // 128
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_UNSUPPORTED_METHOD));
      flags &= ~kpv_ErrorFlags_UnsupportedMethod;
    }
    if ((flags & kpv_ErrorFlags_UnsupportedFeature) == kpv_ErrorFlags_UnsupportedFeature) { // 256
      texts.push_back(L"UnsupportedFeature"); // TODO: localize
      flags &= ~kpv_ErrorFlags_UnsupportedFeature;
    }
    if ((flags & kpv_ErrorFlags_DataError) == kpv_ErrorFlags_DataError) { // 512
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_DATA_ERROR));
      flags &= ~kpv_ErrorFlags_DataError;
    }
    if ((flags & kpv_ErrorFlags_CrcError) == kpv_ErrorFlags_CrcError) { // 1024
      texts.push_back(Far::get_msg(MSG_ERROR_EXTRACT_CRC_ERROR));
      flags &= ~kpv_ErrorFlags_CrcError;
    }
    if (flags != 0) {
      wchar_t buf[32];
      texts.emplace_back(L"Unknown error: ");
      texts.back().append(_ui64tow(flags, buf, 10));
    }
  }
  return texts;
}

std::list<std::wstring> Archive::get_open_errors() const {
  auto errors = flags2texts(error_flags);
  if (!error_text.empty())
    errors.emplace_back(error_text);
  return errors;
}

std::list<std::wstring> Archive::get_open_warnings() const {
  auto warnings = flags2texts(warning_flags);
  if (!warning_text.empty())
    warnings.emplace_back(warning_text);
  return warnings;
}
