#include "comutils.hpp"

// simple format name like '7z', 'zip', etc.
const wchar_t* c_name = L"example";
// unique format id
const GUID c_guid = { 0x75397649, 0x718b, 0x4a02, { 0x9d, 0x35, 0x64, 0xf7, 0x1d, 0x11, 0x99, 0x65 } };
// space separated list of supported file extensions (just a hint)
const wchar_t* c_ext = L"example";

/* simple archive format:
   1. signature
   2. 32-bit unsigned integer - size of file path in characters
   3. file path (array of UTF-16 characters)
   4. 64-bit unsigned integer - size of file data in bytes
   5. file data (array of bytes)
   6. next file...
*/
const unsigned c_sig_size = 7;
const char c_sig[] = "EXAMPLE";

// report information about supported formats
UInt32 WINAPI GetHandlerProperty(PROPID prop_id, PROPVARIANT* value) {
  COM_ERROR_HANDLER_BEGIN
  PropVariant prop;
  switch (prop_id) {
  case NArchive::kName: // simple format name
    prop = c_name;
    break;
  case NArchive::kClassID: // unique format id
    prop.set_binary(&c_guid, sizeof(c_guid));
    break;
  case NArchive::kExtension: // list of extensions
    prop = c_ext;
    break;
  case NArchive::kUpdate: // are modifications supported?
    prop = true;
    break;
  case NArchive::kStartSignature: // signature to help identify format
    prop.set_binary(c_sig, c_sig_size);
    break;
  case NArchive::kAddExtension:
  case NArchive::kKeepName:
  case NArchive::kFinishSignature:
  case NArchive::kAssociate:
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
  kpidSample = kpidUserDefined
};

const STATPROPSTG c_arc_props[] = {
  { L"sample prop", kpidSample, VT_BSTR },
};

// implement IInArchive to support archive extraction
// implement IOutArchive to support archive modification
class Archive: public IInArchive, public IOutArchive, public ComBase {
private:
  struct FileInfo {
    wstring path;
    UInt64 offset;
    UInt64 size;
  };
  vector<FileInfo> files;
  ComObject<IInStream> in_stream;

public:
  UNKNOWN_IMPL_BEGIN
  UNKNOWN_IMPL_ITF(IInArchive)
  UNKNOWN_IMPL_ITF(IOutArchive)
  UNKNOWN_IMPL_END

  // read archive contents from IInStream
  // return S_FALSE if format is not recognized
  STDMETHODIMP Open(IInStream* stream, const UInt64* max_check_start_position, IArchiveOpenCallback* open_archive_callback) {
    COM_ERROR_HANDLER_BEGIN
    in_stream = stream;

    UInt64 file_size = 0;
    CHECK_COM(in_stream->Seek(0, STREAM_SEEK_END, &file_size));
    CHECK_COM(in_stream->Seek(0, STREAM_SEEK_SET, nullptr));

    open_archive_callback->SetTotal(nullptr, &file_size);

    char sig_buf[c_sig_size];
    UInt32 size_read;
    CHECK_COM(in_stream->Read(sig_buf, c_sig_size, &size_read));
    if (size_read != c_sig_size)
      return S_FALSE;
    if (memcmp(c_sig, sig_buf, c_sig_size) != 0)
      return S_FALSE;

    UInt64 file_count = 0;
    list<FileInfo> file_list;
    size_t path_buf_size = MAX_PATH;
    unique_ptr<wchar_t[]> path_buf(new wchar_t[path_buf_size]);
    while (true) {
      FileInfo file_info;
      unsigned path_size;
      CHECK_COM(in_stream->Read(&path_size, sizeof(path_size), &size_read));
      if (size_read == 0)
        break;
      if (size_read != sizeof(path_size))
        return S_FALSE;
      if (path_size > 32000)
        return S_FALSE;
      if (path_size > path_buf_size) {
        path_buf_size = path_size;
        path_buf.reset(new wchar_t[path_buf_size]);
      }
      CHECK_COM(in_stream->Read(path_buf.get(), path_size * sizeof(wchar_t), &size_read));
      if (size_read != path_size * sizeof(wchar_t))
        return S_FALSE;
      file_info.path.assign(path_buf.get(), path_size);

      CHECK_COM(in_stream->Read(&file_info.size, sizeof(file_info.size), &size_read));
      if (size_read != sizeof(file_info.size))
        return S_FALSE;
      CHECK_COM(in_stream->Seek(0, STREAM_SEEK_CUR, &file_info.offset));
      UInt64 file_pos;
      if (in_stream->Seek(file_info.size, STREAM_SEEK_CUR, &file_pos) != S_OK)
        return S_FALSE;
      
      file_list.push_back(file_info);

      file_count++;
      open_archive_callback->SetCompleted(&file_count, &file_pos);
    }

    files.reserve(file_list.size());
    copy(file_list.begin(), file_list.end(), back_inserter(files));

    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // cleanup: free archive stream and other data
  STDMETHODIMP Close() {
    COM_ERROR_HANDLER_BEGIN
    files.clear();
    in_stream = nullptr;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // number of items in archives
  STDMETHODIMP GetNumberOfItems(UInt32* num_items) {
    COM_ERROR_HANDLER_BEGIN
    *num_items = files.size();
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // provide file property values (see PropID.h)
  STDMETHODIMP GetProperty(UInt32 index, PROPID prop_id, PROPVARIANT* value) {
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
  STDMETHODIMP GetNumberOfProperties(UInt32* num_properties) {
    COM_ERROR_HANDLER_BEGIN
    *num_properties = 3;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  // file properties supported by this format
  STDMETHODIMP GetPropertyInfo(UInt32 index, BSTR* name, PROPID* prop_id, VARTYPE* var_type) {
    COM_ERROR_HANDLER_BEGIN
    if (index >= ARRAYSIZE(c_props))
      return E_INVALIDARG;
    // name of custom property
    // null if standard property (see PropID.h)
    if (c_props[index].lpwstrName)
      BStr(c_props[index].lpwstrName).detach(name);
    else
      *name = nullptr;
    // property id (kpidUserDefined is the base for custom properties)
    *prop_id = c_props[index].propid;
    // property type (VT_BSTR, VT_UI8, etc.)
    *var_type = c_props[index].vt;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP Extract(const UInt32* indices, UInt32 num_items, Int32 testMode, IArchiveExtractCallback* extract_callback) {
    COM_ERROR_HANDLER_BEGIN
    Int32 ask_extract_mode = testMode ? NArchive::NExtract::NAskMode::kTest : NArchive::NExtract::NAskMode::kExtract;
    // num_items == -1 means extract all items
    UInt32 total_items = num_items != -1 ? num_items : files.size();
    UInt64 total_size = 0;
    for (UInt32 i = 0; i < total_items; i++) {
      UInt32 file_index = num_items != -1 ? indices[i] : i;
      CHECK(file_index < files.size());
      total_size += files[file_index].size;
    }
    extract_callback->SetTotal(total_size); // progress reporting

    total_size = 0;
    for (UInt32 i = 0; i < total_items; i++) {
      UInt32 file_index = num_items != -1 ? indices[i] : i;

      ComObject<ISequentialOutStream> file_stream;
      CHECK_COM(extract_callback->GetStream(file_index, file_stream.ref(), ask_extract_mode));
      // client may decide to skip file
      if (!file_stream && !testMode)
        continue;

      CHECK_COM(extract_callback->PrepareOperation(ask_extract_mode)); //???
      const FileInfo& file_info = files[file_index];
      const UInt32 c_buffer_size = 1024 * 1024;
      unique_ptr<unsigned char[]> buffer(new unsigned char[c_buffer_size]);
      UInt32 size_read, size_written;
      UInt64 total_size_read = 0;
      CHECK_COM(in_stream->Seek(file_info.offset, STREAM_SEEK_SET, nullptr));
      while (true) {
        UInt32 size = c_buffer_size;
        if (total_size_read + size > file_info.size)
          size = static_cast<UInt32>(file_info.size - total_size_read);
        if (size == 0)
          break;
        CHECK_COM(in_stream->Read(buffer.get(), size, &size_read));
        CHECK(size_read == size);
        total_size_read += size_read;
        if (!testMode) {
          CHECK_COM(file_stream->Write(buffer.get(), size_read, &size_written));
          CHECK(size_written == size_read);
        }
        total_size += size_read;
        extract_callback->SetCompleted(&total_size); // progress reporting
      }
      // report that file is extracted successfully
      CHECK_COM(extract_callback->SetOperationResult(NArchive::NExtract::NOperationResult::kOK));
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetArchiveProperty(PROPID prop_id, PROPVARIANT* value) {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    switch (prop_id) {
    case kpidSample:
      prop = L"sample value";
      break;
    }
    prop.detach(value);
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetNumberOfArchiveProperties(UInt32* num_properties) {
    COM_ERROR_HANDLER_BEGIN
    *num_properties = 1;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetArchivePropertyInfo(UInt32 index, BSTR* name, PROPID* prop_id, VARTYPE* var_type) {
    COM_ERROR_HANDLER_BEGIN
    if (index >= ARRAYSIZE(c_arc_props))
      return E_INVALIDARG;
    if (c_arc_props[index].lpwstrName)
      BStr(c_arc_props[index].lpwstrName).detach(name);
    else
      *name = nullptr;
    *prop_id = c_arc_props[index].propid;
    *var_type = c_arc_props[index].vt;
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP UpdateItems(ISequentialOutStream* out_stream, UInt32 num_items, IArchiveUpdateCallback* update_callback) {
    COM_ERROR_HANDLER_BEGIN
    PropVariant prop;
    UInt64 total_size = 0;
    map<UInt32, FileInfo> new_files;
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
          file_info.size = prop.get_uint();
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
          CHECK(prop.get_uint() == file_info.size);
        }
      }
      new_files[i] = file_info;
      total_size += file_info.size;
    }
    update_callback->SetTotal(total_size);

    UInt32 size_written;
    CHECK_COM(out_stream->Write(c_sig, c_sig_size, &size_written));
    CHECK(size_written == c_sig_size);

    total_size = 0;
    for (auto iter = new_files.begin(); iter != new_files.end(); iter++) {
      const FileInfo& file_info = iter->second;
      ComObject<ISequentialInStream> file_stream;
      if (file_info.offset == 0) {
        CHECK_COM(update_callback->GetStream(iter->first, file_stream.ref()));
        if (!file_stream) // client may choose to skip file
          continue;
      }
       
      unsigned path_size = file_info.path.size();
      CHECK_COM(out_stream->Write(&path_size, sizeof(path_size), &size_written));
      CHECK(size_written == sizeof(path_size));
      CHECK_COM(out_stream->Write(file_info.path.data(), file_info.path.size() * sizeof(wchar_t), &size_written));
      CHECK(size_written == file_info.path.size() * sizeof(wchar_t));

      const UInt32 c_buffer_size = 1024 * 1024;
      unique_ptr<unsigned char[]> buffer(new unsigned char[c_buffer_size]);
      UInt32 size_read;
      UInt64 total_size_read = 0;
      CHECK_COM(out_stream->Write(&file_info.size, sizeof(file_info.size), &size_written));
      CHECK(size_written == sizeof(file_info.size));
      if (file_info.offset == 0) {
        while (true) {
          CHECK_COM(file_stream->Read(buffer.get(), c_buffer_size, &size_read));
          if (size_read == 0)
            break;
          total_size_read += size_read;
          CHECK(total_size_read <= file_info.size);
          CHECK_COM(out_stream->Write(buffer.get(), size_read, &size_written));
          CHECK(size_written == size_read);
          total_size += size_written;
          update_callback->SetCompleted(&total_size);
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
          CHECK_COM(in_stream->Read(buffer.get(), size, &size_read));
          CHECK(size_read == size);
          total_size_read += size_read;
          CHECK_COM(out_stream->Write(buffer.get(), size_read, &size_written));
          CHECK(size_written == size_read);
          total_size += size_written;
          update_callback->SetCompleted(&total_size);
        }
      }
      // report that file is added successfully
      update_callback->SetOperationResult(NArchive::NUpdate::NOperationResult::kOK);
    }
    return S_OK;
    COM_ERROR_HANDLER_END
  }

  STDMETHODIMP GetFileTimeType(UInt32* type) {
    COM_ERROR_HANDLER_BEGIN
    *type = NFileTimeType::kWindows;
    return S_OK;
    COM_ERROR_HANDLER_END
  }
};

// create implementation of required interface (interface_id) for given archive format (class_id)
UInt32 WINAPI CreateObject(const GUID* class_is, const GUID* interface_id, void** out_object) {
  COM_ERROR_HANDLER_BEGIN
  if (*class_is != c_guid)
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
