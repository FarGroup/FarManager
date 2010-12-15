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

vector<StrPos> msearch(unsigned char* data, size_t size, const vector<ByteVector>& str_list);
