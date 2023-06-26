#include "comutils.hpp"
#include <memory>

// simple format name like '7z', 'zip', etc.
const wchar_t* c_name = L"example";
// unique format id
const GUID c_format_id = { 0x75397649, 0x718b, 0x4a02, { 0x9d, 0x35, 0x64, 0xf7, 0x1d, 0x11, 0x99, 0x65 } };
// space separated list of supported file extensions (hint to optimize format detection; default file extension)
const wchar_t* c_ext = L"example";

/* simple archive format:
   1. signature
   2. 32-bit unsigned integer - compression level (just an illustration of ISetProperties, files are not really compressed)
   3. 32-bit unsigned integer - size of file path in characters
   4. file path (array of UTF-16 characters)
   5. 64-bit unsigned integer - size of file data in bytes
   6. file data (array of bytes)
   7. next file...
*/
const unsigned c_sig_size = 7;
const char c_sig[] = "EXAMPLE";

// report information about supported formats
UInt32 WINAPI GetHandlerProperty(PROPID prop_id, PROPVARIANT* value) {
  COM_ERROR_HANDLER_BEGIN
  PropVariant prop;
  switch (prop_id) {
  case NArchive::NHandlerPropID::kName: // simple format name
    prop = c_name;
    break;
  case NArchive::NHandlerPropID::kClassID: // unique format id
    prop.set_binary(&c_format_id, sizeof(c_format_id));
    break;
  case NArchive::NHandlerPropID::kExtension: // list of extensions
    prop = c_ext;
    break;
  case NArchive::NHandlerPropID::kUpdate: // are modifications supported?
    prop = true;
    break;
  case NArchive::NHandlerPropID::kSignature: // signature to help identify format
    prop.set_binary(c_sig, c_sig_size);
    break;
  case NArchive::NHandlerPropID::kAddExtension:
  case NArchive::NHandlerPropID::kKeepName:
  case NArchive::NHandlerPropID::kMultiSignature:
  case NArchive::NHandlerPropID::kSignatureOffset:
  case NArchive::NHandlerPropID::kAltStreams:
  case NArchive::NHandlerPropID::kNtSecure:
  case NArchive::NHandlerPropID::kFlags:
    break;
  }
  prop.detach(value);
  return S_OK;
  COM_ERROR_HANDLER_END
}

const STATPROPSTG c_props[] = {
  { nullptr, kpidPath, VT_BSTR },
  { nullptr, kpidIsDir, VT_BOOL },
  { nullptr, kpidSize, VT_UI8 },
};

enum {
  kpidCompLevel = kpidUserDefined
};

const STATPROPSTG c_arc_props[] = {
  { const_cast<wchar_t*>(L"Compression level"), kpidCompLevel, VT_UI4 },
};

// implement IInArchive to support archive extraction
// implement IOutArchive to support archive modification
// implement ISetProperties to support compression settings (compression level, etc.)
// implement IInArchiveGetStream to support individual file streams (ability to access archive items without the need to extract them first)
class Archive: public IInArchive, public IOutArchive, public ISetProperties, public IInArchiveGetStream, private ComBase {
private:
  struct FileInfo {
    std::wstring path;
    UInt64 offset;
    UInt64 size;
  };
  std::vector<FileInfo> files;
  ComObject<IInStream> in_stream;
  UInt32 comp_level;

public:
  Archive(): comp_level(0) {
  }

  // *** IUnknown ***

  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IInArchive)
  UNKNOWN_IMPL_ITF(IOutArchive)
  UNKNOWN_IMPL_ITF(ISetProperties)
  UNKNOWN_IMPL_ITF(IInArchiveGetStream)
  UNKNOWN_IMPL_END

  // *** IInArchive ***

  // read archive contents from IInStream
  // return S_FALSE if format is not recognized
  STDMETHODIMP Open(IInStream* stream, const UInt64* max_check_start_position, IArchiveOpenCallback* open_archive_callback) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    in_stream = stream;

    UInt64 file_size = 0;
    CHECK_COM(in_stream->Seek(0, STREAM_SEEK_END, &file_size));
    CHECK_COM(in_stream->Seek(0, STREAM_SEEK_SET, nullptr));

    CHECK_COM(open_archive_callback->SetTotal(nullptr, &file_size));

    char sig_buf[c_sig_size];
    if (read_stream(in_stream, sig_buf, c_sig_size) != c_sig_size)
      return S_FALSE;
    if (memcmp(c_sig, sig_buf, c_sig_size) != 0)
      return S_FALSE;

    if (read_stream(in_stream, &comp_level, sizeof(comp_level)) != sizeof(comp_level))
      return S_FALSE;

    UInt64 file_count = 0;
    std::list<FileInfo> file_list;
    size_t path_buf_size = MAX_PATH;
    std::wstring path_buf(path_buf_size, 0);
    while (true) {
      FileInfo file_info;
      UInt32 path_size;
      UInt32 size_read = read_stream(in_stream, &path_size, sizeof(path_size));
      if (size_read == 0)
        break;
      if (size_read != sizeof(path_size))
        return S_FALSE;
      if (path_size > 32000)
        return S_FALSE;
      if (path_size > path_buf_size) {
        path_buf_size = path_size;
        path_buf.resize(path_buf_size);
      }
      if (read_stream(in_stream, path_buf.data(), path_size * sizeof(wchar_t)) != path_size * sizeof(wchar_t))
        return S_FALSE;
      file_info.path.assign(path_buf.data(), path_size);

      if (read_stream(in_stream, &file_info.size, sizeof(file_info.size)) != sizeof(file_info.size))
        return S_FALSE;
      CHECK_COM(in_stream->Seek(0, STREAM_SEEK_CUR, &file_info.offset));
      UInt64 file_pos;
      if (in_stream->Seek(file_info.size, STREAM_SEEK_CUR, &file_pos) != S_OK)
        return S_FALSE;

      file_list.push_back(file_info);

      file_count++;
      CHECK_COM(open_archive_callback->SetCompleted(&file_count, &file_pos));
    }

    files.reserve(file_list.size());
    copy(file_list.begin(), file_list.end(), back_inserter(files));

    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // cleanup: free archive stream and other data
  STDMETHODIMP Close() noexcept override {
    COM_ERROR_HANDLER_BEGIN
    files.clear();
    in_stream = nullptr;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // number of items in archives
  STDMETHODIMP GetNumberOfItems(UInt32* num_items) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    *num_items = static_cast<UInt32>(files.size());
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // provide file property values (see PropID.h)
  STDMETHODIMP GetProperty(UInt32 index, PROPID prop_id, PROPVARIANT* value) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    if (index >= files.size())
      return E_INVALIDARG;
    switch (prop_id) {
    case kpidPath: // full path inside archive
      prop = files[index].path;
      break;
    case kpidIsDir: // directory or file
      prop = false;
      break;
    case kpidSize: // size of file in bytes
      prop = files[index].size;
      break;
    }
    prop.detach(value);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // report number of file properties supported by this format
  STDMETHODIMP GetNumberOfProperties(UInt32* num_properties) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    *num_properties = ARRAYSIZE(c_props);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // file properties supported by this format
  STDMETHODIMP GetPropertyInfo(UInt32 index, BSTR* name, PROPID* prop_id, VARTYPE* var_type) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (index >= ARRAYSIZE(c_props))
      return E_INVALIDARG;
    // name of custom property
    // null if standard property (see PropID.h)
    BStr(c_props[index].lpwstrName).detach(name);
    // property id (kpidUserDefined is the base for custom properties)
    *prop_id = c_props[index].propid;
    // property type (VT_BSTR, VT_UI8, etc.)
    *var_type = c_props[index].vt;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Extract(const UInt32* indices, UInt32 num_items, Int32 test_mode, IArchiveExtractCallback* extract_callback) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    Int32 ask_extract_mode = test_mode ? NArchive::NExtract::NAskMode::kTest : NArchive::NExtract::NAskMode::kExtract;
    // num_items == -1 means extract all items
    UInt32 total_items = num_items != -1 ? num_items : static_cast<UInt32>(files.size());
    UInt64 total_size = 0;
    for (UInt32 i = 0; i < total_items; i++) {
      UInt32 file_index = num_items != -1 ? indices[i] : i;
      CHECK(file_index < files.size());
      total_size += files[file_index].size;
    }
    CHECK_COM(extract_callback->SetTotal(total_size)); // progress reporting

    total_size = 0;
    for (UInt32 i = 0; i < total_items; i++) {
      UInt32 file_index = num_items != -1 ? indices[i] : i;

      ComObject<ISequentialOutStream> file_stream;
      CHECK_COM(extract_callback->GetStream(file_index, file_stream.ref(), ask_extract_mode));
      // client may decide to skip file
      if (!file_stream && !test_mode)
        continue;

      CHECK_COM(extract_callback->PrepareOperation(ask_extract_mode)); //???
      const FileInfo& file_info = files[file_index];
      const UInt32 c_buffer_size = 1024 * 1024;
      const auto buffer = std::make_unique<unsigned char[]>(c_buffer_size);
      UInt64 total_size_read = 0;
      CHECK_COM(in_stream->Seek(file_info.offset, STREAM_SEEK_SET, nullptr));
      while (true) {
        UInt32 size = c_buffer_size;
        if (total_size_read + size > file_info.size)
          size = static_cast<UInt32>(file_info.size - total_size_read);
        if (size == 0)
          break;
        UInt32 size_read = read_stream(in_stream, buffer.get(), size);
        CHECK(size_read == size);
        total_size_read += size_read;
        if (!test_mode) {
          write_stream(file_stream, buffer.get(), size_read);
        }
        total_size += size_read;
        CHECK_COM(extract_callback->SetCompleted(&total_size)); // progress reporting
      }
      // report that file is extracted successfully
      CHECK_COM(extract_callback->SetOperationResult(NArchive::NExtract::NOperationResult::kOK));
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetArchiveProperty(PROPID prop_id, PROPVARIANT* value) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    switch (prop_id) {
    case kpidCompLevel:
      prop = comp_level;
      break;
    }
    prop.detach(value);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetNumberOfArchiveProperties(UInt32* num_properties) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    *num_properties = ARRAYSIZE(c_arc_props);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetArchivePropertyInfo(UInt32 index, BSTR* name, PROPID* prop_id, VARTYPE* var_type) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (index >= ARRAYSIZE(c_arc_props))
      return E_INVALIDARG;
    BStr(c_arc_props[index].lpwstrName).detach(name);
    *prop_id = c_arc_props[index].propid;
    *var_type = c_arc_props[index].vt;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // *** IOutArchive ***

  STDMETHODIMP UpdateItems(ISequentialOutStream* out_stream, UInt32 num_items, IArchiveUpdateCallback* update_callback) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    UInt64 total_size = 0;
    std::map<UInt32, FileInfo> new_files;
    for (UInt32 i = 0; i < num_items; i++) {
      FileInfo file_info;
      Int32 new_data;
      Int32 new_properties;
      UInt32 index_in_archive;
      CHECK_COM(update_callback->GetUpdateItemInfo(i, &new_data, &new_properties, &index_in_archive));
      if (new_data) {
        if (new_properties) {
          // new file is added to archive
          CHECK_COM(update_callback->GetProperty(i, kpidPath, prop.ref()));
          file_info.path = prop.get_str();
          CHECK_COM(update_callback->GetProperty(i, kpidIsDir, prop.ref()));
          if (prop.get_bool())
            continue;
          CHECK_COM(update_callback->GetProperty(i, kpidSize, prop.ref()));
          file_info.size = prop.get_uint64();
        }
        else {
          // file exists in archive, need update data
          CHECK(index_in_archive < files.size());
          file_info = files[index_in_archive];
        }
        file_info.offset = 0;
      }
      else {
        // file exists in archive and its data is not changed
        CHECK(in_stream);
        CHECK(index_in_archive < files.size());
        file_info = files[index_in_archive];
        if (new_properties) {
          // but properties changed (i.e. file renamed, attr change, etc.)
          CHECK_COM(update_callback->GetProperty(i, kpidPath, prop.ref()));
          file_info.path = prop.get_str();
          CHECK_COM(update_callback->GetProperty(i, kpidIsDir, prop.ref()));
          CHECK(!prop.get_bool());
          CHECK_COM(update_callback->GetProperty(i, kpidSize, prop.ref()));
          CHECK(prop.get_uint64() == file_info.size);
        }
      }
      new_files[i] = file_info;
      total_size += file_info.size;
    }
    CHECK_COM(update_callback->SetTotal(total_size));

    write_stream(out_stream, c_sig, c_sig_size);

    write_stream(out_stream, &comp_level, sizeof(comp_level));

    total_size = 0;
    for (auto iter = new_files.begin(); iter != new_files.end(); iter++) {
      const FileInfo& file_info = iter->second;
      ComObject<ISequentialInStream> file_stream;
      if (file_info.offset == 0) {
        CHECK_COM(update_callback->GetStream(iter->first, file_stream.ref()));
        if (!file_stream) // client may choose to skip file
          continue;
      }

      UInt32 path_size = static_cast<UInt32>(file_info.path.size());
      write_stream(out_stream, &path_size, sizeof(path_size));
      write_stream(out_stream, file_info.path.data(), static_cast<UInt32>(file_info.path.size() * sizeof(wchar_t)));

      const UInt32 c_buffer_size = 1024 * 1024;
      const auto buffer = std::make_unique<unsigned char[]>(c_buffer_size);
      UInt64 total_size_read = 0;
      write_stream(out_stream, &file_info.size, sizeof(file_info.size));
      if (file_info.offset == 0) {
        while (true) {
          UInt32 size_read = read_stream(file_stream, buffer.get(), c_buffer_size);
          if (size_read == 0)
            break;
          total_size_read += size_read;
          CHECK(total_size_read <= file_info.size);
          write_stream(out_stream, buffer.get(), size_read);
          total_size += size_read;
          CHECK_COM(update_callback->SetCompleted(&total_size));
        }
        CHECK(total_size_read == file_info.size);
      }
      else {
        CHECK_COM(in_stream->Seek(file_info.offset, STREAM_SEEK_SET, nullptr));
        while (true) {
          UInt32 size = c_buffer_size;
          if (total_size_read + size > file_info.size)
            size = static_cast<UInt32>(file_info.size - total_size_read);
          if (size == 0)
            break;
          UInt32 size_read = read_stream(in_stream, buffer.get(), size);
          CHECK(size_read == size);
          total_size_read += size_read;
          write_stream(out_stream, buffer.get(), size_read);
          total_size += size_read;
          CHECK_COM(update_callback->SetCompleted(&total_size));
        }
      }
      // report that file is added successfully
      CHECK_COM(update_callback->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK));
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetFileTimeType(UInt32* type) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    *type = NFileTimeType::kWindows;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // *** ISetProperties ***

  // set compression properties
  STDMETHODIMP SetProperties(const wchar_t*const* names, const PROPVARIANT* values, UInt32 num_properties) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    for (Int32 i = 0; i < (int)num_properties; i++) {
      PropVariant prop = values[i];

      size_t buf_size = wcslen(names[i]) + 1;
      const auto buf = std::make_unique<wchar_t[]>(buf_size);
      std::wcscpy(buf.get(), names[i]);
      _wcsupr_s(buf.get(), buf_size);
      std::wstring name(buf.get(), buf_size - 1);

      if (name == L"X") { // typical name for compression level property
        if (prop.is_uint())
          comp_level = prop.get_uint();
        else if (prop.is_str()) // property can be passed as a string even if it is numeric
          comp_level = _wtoi(prop.get_str().c_str());
      }
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // *** IInArchiveGetStream ***

  // support for IInArchiveGetStream allows client to open nested archives
  // without the need to extract them into temporary directory first
  // if possible always implement IInStream in addition to ISequentialInStream
  class FileStream: public IInStream, private ComBase {
  private:
    FileInfo file_info;
    ComObject<IInStream> in_stream;
    UInt64 pos;
  public:
    FileStream(IInStream* in_stream, const FileInfo& file_info): file_info(file_info), in_stream(in_stream), pos(0) {
    }

    UNKNOWN_IMPL_BEGIN
    UNKNOWN_IMPL_ITF(ISequentialInStream)
    UNKNOWN_IMPL_ITF(IInStream)
    UNKNOWN_IMPL_END

    STDMETHODIMP Read(void* data, UInt32 size, UInt32* processed_size) noexcept override {
      COM_ERROR_HANDLER_BEGIN
      if (pos > file_info.size)
        size = 0;
      else if (pos + size > file_info.size)
        size = static_cast<UInt32>(file_info.size - pos);
      CHECK_COM(in_stream->Seek(file_info.offset + pos, STREAM_SEEK_SET, nullptr));
      UInt32 size_read;
      CHECK_COM(in_stream->Read(data, size, &size_read));
      pos += size_read;
      if (processed_size)
        *processed_size = size_read;
      return S_OK;
      COM_ERROR_HANDLER_END
    }

    STDMETHODIMP Seek(Int64 offset, UInt32 seek_origin, UInt64* new_position) noexcept override {
      COM_ERROR_HANDLER_BEGIN
      switch (seek_origin) {
      case STREAM_SEEK_SET:
        pos = offset;
        break;
      case STREAM_SEEK_CUR:
        pos += offset;
        break;
      case STREAM_SEEK_END:
        pos = file_info.size + offset;
        break;
      default:
        return E_INVALIDARG;
      }
      if (new_position)
        *new_position = pos;
      return S_OK;
      COM_ERROR_HANDLER_END
    }
  };

  // client will request IInStream via QueryInterface if needed
  STDMETHODIMP GetStream(UInt32 index, ISequentialInStream** stream) noexcept override {
    COM_ERROR_HANDLER_BEGIN
    if (index >= files.size())
      return E_INVALIDARG;
    ComObject<ISequentialInStream>(new FileStream(in_stream, files[index])).detach(stream);
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};

// create implementation of required interface (interface_id) for given archive format (class_id)
UInt32 WINAPI CreateObject(const GUID* class_id, const GUID* interface_id, void** out_object) {
  COM_ERROR_HANDLER_BEGIN
  if (*class_id != c_format_id)
    return CLASS_E_CLASSNOTAVAILABLE;
  if (*interface_id == IID_IInArchive)
    ComObject<IInArchive>(new Archive()).detach(reinterpret_cast<IInArchive**>(out_object));
  else if (*interface_id == IID_IOutArchive)
    ComObject<IOutArchive>(new Archive()).detach(reinterpret_cast<IOutArchive**>(out_object));
  else
    return E_NOINTERFACE;
  return S_OK;
  COM_ERROR_HANDLER_END
}
