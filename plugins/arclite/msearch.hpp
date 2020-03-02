#pragma once

typedef unsigned short StrIndex;

struct StrPos {
  StrIndex idx;
  size_t pos;
  bool operator<(const StrPos& a) const {
    return pos < a.pos;
  }
  StrPos(StrIndex idx, size_t pos): idx(idx), pos(pos) {
  }
};

struct ArcFormat;

struct SigData {
  const ByteVector& signature;
  const ArcFormat& format;
  SigData(const ByteVector& sig, const ArcFormat& fmt) : signature(sig), format(fmt) {}
};

std::vector<StrPos> msearch(unsigned char* data, size_t size, const std::vector<SigData>& str_list, bool eof);
