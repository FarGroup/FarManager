#pragma once

#include "../error.hpp"

#define COM_ERROR_HANDLER_BEGIN \
  try {

#define COM_ERROR_HANDLER_END \
  } \
  catch (const Error& e) { \
    return e.code; \
  } \
  catch (...) { \
    return E_FAIL; \
  }

class ComBase: public IUnknown {
protected:
  ULONG ref_cnt;
public:
  ComBase(): ref_cnt(0) {}
  virtual ~ComBase() {}
};

#define UNKNOWN_DECL \
  STDMETHOD(QueryInterface)(REFIID riid, void** object) override; \
  STDMETHOD_(ULONG, AddRef)() override; \
  STDMETHOD_(ULONG, Release)() override;

#define UNKNOWN_IMPL_BEGIN \
  STDMETHOD(QueryInterface)(REFIID riid, void** object) override {

#define UNKNOWN_IMPL_ITF(iid) \
    if (riid == IID_##iid) { *object = static_cast<iid*>(this); AddRef(); return S_OK; }

#define UNKNOWN_IMPL_END \
    if (riid == IID_IUnknown) { *object = static_cast<IUnknown*>(static_cast<ComBase*>(this)); AddRef(); return S_OK; } \
    *object = nullptr; return E_NOINTERFACE; \
  } \
  STDMETHOD_(ULONG, AddRef)() override { return ++ref_cnt; } \
  STDMETHOD_(ULONG, Release)() override { if (--ref_cnt == 0) { delete this; return 0; } else return ref_cnt; }

#define UNKNOWN_IMPL \
  UNKNOWN_IMPL_BEGIN \
  UNKNOWN_IMPL_END

template<class Itf> class ComObject {
private:
  Itf* obj;
public:
  ComObject(): obj(nullptr) {}
  ComObject(Itf* param): obj(param) {
    if (obj)
      obj->AddRef();
  }
  ComObject(const ComObject<Itf>& param): obj(param.obj) {
    if (obj)
      obj->AddRef();
  }
  ~ComObject() {
    Release();
  }
  void Release() {
    if (obj) {
      obj->Release();
      obj = nullptr;
    }
  }
  operator Itf*() const {
    return obj;
  }
  operator bool() const {
    return obj != nullptr;
  }
  Itf** ref() {
    Release();
    return &obj;
  }
  Itf* operator->() const {
    return obj;
  }
  Itf* operator=(Itf* param) {
    Release();
    obj = param;
    if (obj)
      obj->AddRef();
    return obj;
  }
  Itf* operator=(const ComObject<Itf>& param) {
    return *this = param.obj;
  }
  void detach(Itf** param) {
    *param = obj;
    obj = nullptr;
  }
};

class PropVariant: public PROPVARIANT {
private:
  void clear() {
    if (vt != VT_EMPTY)
      CHECK_COM(PropVariantClear(this));
  }
public:
  PropVariant() {
    PropVariantInit(this);
  }
  ~PropVariant() {
    clear();
  }
  PROPVARIANT* ref() {
    clear();
    return this;
  }
  void detach(PROPVARIANT* var) {
    if (var->vt != VT_EMPTY)
      CHECK_COM(PropVariantClear(var));
    *var = *this;
    vt = VT_EMPTY;
  }

  PropVariant(const PropVariant& var) {
    CHECK_COM(PropVariantCopy(this, &var));
  }
  PropVariant(const PROPVARIANT& var) {
    CHECK_COM(PropVariantCopy(this, &var));
  }
  PropVariant(const std::wstring& val) {
    vt = VT_BSTR;
    bstrVal = SysAllocStringLen(val.data(), static_cast<UINT>(val.size()));
    if (bstrVal == nullptr) {
      vt = VT_ERROR;
      FAIL(E_OUTOFMEMORY);
    }
  }
  PropVariant(const wchar_t* val) {
    vt = VT_BSTR;
    bstrVal = SysAllocStringLen(val, static_cast<UINT>(wcslen(val)));
    if (bstrVal == nullptr) {
      vt = VT_ERROR;
      FAIL(E_OUTOFMEMORY);
    }
  }
  PropVariant(bool val) {
    vt = VT_BOOL;
    boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
  }
  PropVariant(Int32 val) {
    vt = VT_I4;
    lVal = val;
  }
  PropVariant(Int64 val) {
    vt = VT_I8;
    hVal.QuadPart = val;
  }
  PropVariant(UInt32 val) {
    vt = VT_UI4;
    ulVal = val;
  }
  PropVariant(UInt64 val) {
    vt = VT_UI8;
    uhVal.QuadPart = val;
  }
  PropVariant(const FILETIME &val) {
    vt = VT_FILETIME;
    filetime = val;
  }

  PropVariant& operator=(const PropVariant& var) {
    clear();
    CHECK_COM(PropVariantCopy(this, &var));
    return *this;
  }
  PropVariant& operator=(const PROPVARIANT& var) {
    clear();
    CHECK_COM(PropVariantCopy(this, &var));
    return *this;
  }
  PropVariant& operator=(const std::wstring& val) {
    clear();
    vt = VT_BSTR;
    bstrVal = SysAllocStringLen(val.data(), static_cast<UINT>(val.size()));
    if (bstrVal == nullptr) {
      vt = VT_ERROR;
      FAIL(E_OUTOFMEMORY);
    }
    return *this;
  }
  PropVariant& operator=(const wchar_t* val) {
    clear();
    vt = VT_BSTR;
    bstrVal = SysAllocStringLen(val, static_cast<UINT>(wcslen(val)));
    if (bstrVal == nullptr) {
      vt = VT_ERROR;
      FAIL(E_OUTOFMEMORY);
    }
    return *this;
  }
  PropVariant& operator=(bool val) {
    if (vt != VT_BOOL) {
      clear();
      vt = VT_BOOL;
    }
    boolVal = val ? VARIANT_TRUE : VARIANT_FALSE;
    return *this;
  }
  PropVariant& operator=(Int32 val) {
    if (vt != VT_I4) {
      clear();
      vt = VT_I4;
    }
    lVal = val;
    return *this;
  }
  PropVariant& operator=(Int64 val) {
    if (vt != VT_I8) {
      clear();
      vt = VT_I8;
    }
    hVal.QuadPart = val;
    return *this;
  }
  PropVariant& operator=(UInt32 val) {
    if (vt != VT_UI4) {
      clear();
      vt = VT_UI4;
    }
    ulVal = val;
    return *this;
  }
  PropVariant& operator=(UInt64 val) {
    if (vt != VT_UI8) {
      clear();
      vt = VT_UI8;
    }
    uhVal.QuadPart = val;
    return *this;
  }
  PropVariant& operator=(const FILETIME &val) {
    if (vt != VT_FILETIME) {
      clear();
      vt = VT_FILETIME;
    }
    filetime = val;
    return *this;
  }
  PropVariant& set_binary(const void* val, size_t size) {
    clear();
    vt = VT_BSTR;
    bstrVal = SysAllocStringByteLen(reinterpret_cast<LPCSTR>(val), static_cast<UINT>(size));
    if (bstrVal == nullptr) {
      vt = VT_ERROR;
      FAIL(E_OUTOFMEMORY);
    }
    return *this;
  }

  bool is_int() const {
    return vt == VT_I1 || vt == VT_I2 || vt == VT_I4 || vt == VT_INT;
  }
  bool is_uint() const {
    return vt == VT_UI1 || vt == VT_UI2 || vt == VT_UI4 || vt == VT_UINT;
  }
  bool is_int64() const {
    return is_int() || vt == VT_I8;
  }
  bool is_uint64() const {
    return is_uint() || vt == VT_UI8;
  }
  bool is_str() const {
    return vt == VT_BSTR || vt == VT_LPWSTR;
  }
  bool is_bool() const {
    return vt == VT_BOOL;
  }
  bool is_filetime() const {
    return vt == VT_FILETIME;
  }

  Int32 get_int() const {
    switch (vt) {
    case VT_I1:
      return cVal;
    case VT_I2:
      return iVal;
    case VT_I4:
      return lVal;
    case VT_INT:
      return intVal;
    default:
      FAIL(E_INVALIDARG);
    }
  }
  Int64 get_int64() const {
    if (vt == VT_I8)
      return hVal.QuadPart;
    else
      return get_int();
  }
  UInt32 get_uint() const {
    switch (vt) {
    case VT_UI1:
      return bVal;
    case VT_UI2:
      return uiVal;
    case VT_UI4:
      return ulVal;
    case VT_UINT:
      return uintVal;
    default:
      FAIL(E_INVALIDARG);
    }
  }
  UInt64 get_uint64() const {
    if (vt == VT_UI8)
      return uhVal.QuadPart;
    else
      return get_uint();
  }
  unsigned get_int_size() const {
    switch (vt) {
    case VT_UI1:
      return sizeof(bVal);
    case VT_UI2:
      return sizeof(uiVal);
    case VT_UI4:
      return sizeof(ulVal);
    case VT_UINT:
      return sizeof(uintVal);
    case VT_UI8:
      return sizeof(uhVal);
    case VT_I1:
      return sizeof(cVal);
    case VT_I2:
      return sizeof(iVal);
    case VT_I4:
      return sizeof(lVal);
    case VT_INT:
      return sizeof(intVal);
    case VT_I8:
      return sizeof(hVal);
    default:
      FAIL(E_INVALIDARG);
    }
  }
  std::wstring get_str() const {
    switch (vt) {
    case VT_BSTR:
      return std::wstring(bstrVal, SysStringLen(bstrVal));
    case VT_LPWSTR:
      return std::wstring(pwszVal);
    default:
      FAIL(E_INVALIDARG);
    }
  }
  bool get_bool() const {
    switch (vt) {
    case VT_BOOL:
      return boolVal == VARIANT_TRUE;
    default:
      FAIL(E_INVALIDARG);
    }
  }
  FILETIME get_filetime() const {
    switch (vt) {
    case VT_FILETIME:
      return filetime;
    default:
      FAIL(E_INVALIDARG);
    }
  }
  std::vector<unsigned char> get_binary() const {
    if (vt != VT_BSTR)
      FAIL(E_INVALIDARG);
    unsigned char* data = reinterpret_cast<unsigned char*>(bstrVal);
    return std::vector<unsigned char>(data, data + SysStringByteLen(bstrVal));
  }

};

class BStr {
private:
  BSTR bstr;
  void clear() {
    if (bstr) {
      SysFreeString(bstr);
      bstr = nullptr;
    }
  }

  void alloc(const BStr& str) {
    bstr = SysAllocStringLen(str.bstr, SysStringLen(str.bstr));
    if (bstr == nullptr)
      FAIL(E_OUTOFMEMORY);
  }
  void alloc(const std::wstring& str) {
    bstr = SysAllocStringLen(str.data(), static_cast<UINT>(str.size()));
    if (bstr == nullptr)
      FAIL(E_OUTOFMEMORY);
  }
  void alloc(const wchar_t* str) {
    bstr = SysAllocString(str);
    if (bstr == nullptr)
      FAIL(E_OUTOFMEMORY);
  }

  void realloc(const BStr& str) {
    if (!SysReAllocStringLen(&bstr, str.bstr, SysStringLen(str.bstr)))
      FAIL(E_OUTOFMEMORY);
  }
  void realloc(const std::wstring& str) {
    if (!SysReAllocStringLen(&bstr, str.data(), static_cast<UINT>(str.size())))
      FAIL(E_OUTOFMEMORY);
  }
  void realloc(const wchar_t* str) {
    if (!SysReAllocString(&bstr, str))
      FAIL(E_OUTOFMEMORY);
  }

public:
  BStr(): bstr(nullptr) {
  }
  ~BStr() {
    clear();
  }
  operator const wchar_t*() const {
    return bstr;
  }
  unsigned size() const {
    return SysStringLen(bstr);
  }
  BSTR* ref() {
    clear();
    return &bstr;
  }
  void detach(BSTR* str) {
    *str = bstr;
    bstr = nullptr;
  }

  BStr(const BStr& str) {
    alloc(str);
  }
  BStr(const std::wstring& str) {
    alloc(str);
  }
  BStr(const wchar_t* str) {
    if (str)
      alloc(str);
    else
      bstr = nullptr;
  }

  BStr& operator=(const BStr& str) {
    if (bstr)
      realloc(str);
    else
      alloc(str);
    return *this;
  }
  BStr& operator=(const std::wstring& str) {
    if (bstr)
      realloc(str);
    else
      alloc(str);
    return *this;
  }
  BStr& operator=(const wchar_t* str) {
    if (str == nullptr)
      clear();
    else if (bstr)
      realloc(str);
    else
      alloc(str);
    return *this;
  }
};

// stream reader/writer (read comments in IStream.h for explanation)

const UInt32 c_max_chunk_size = 1024 * 1024; // max. number of bytes to read/write at a time

inline UInt32 read_stream(ISequentialInStream* stream, void* data, UInt32 size) {
  UInt32 pos = 0;
  while (pos < size) {
    UInt32 chunk_size = size - pos;
    if (chunk_size > c_max_chunk_size)
      chunk_size = c_max_chunk_size;
    UInt32 size_read;
    CHECK_COM(stream->Read(static_cast<Byte*>(data) + pos, chunk_size, &size_read));
    if (size_read == 0) // end of stream
      break;
    pos += size_read;
  }
  return pos;
}

inline void write_stream(ISequentialOutStream* stream, const void* data, UInt32 size) {
  UInt32 pos = 0;
  while (pos < size) {
    UInt32 chunk_size = size - pos;
    if (chunk_size > c_max_chunk_size)
      chunk_size = c_max_chunk_size;
    UInt32 size_written;
    CHECK_COM(stream->Write(static_cast<const Byte*>(data) + pos, chunk_size, &size_written));
    CHECK(size_written);
    pos += size_written;
  }
}
