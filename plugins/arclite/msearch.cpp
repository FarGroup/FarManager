#include "utils.hpp"
#include "sysutils.hpp"
#include "farutils.hpp"
#include "common.hpp"
#include "archive.hpp"
#include "msearch.hpp"

typedef unsigned short State;

#pragma pack (push, 1)
struct StateElement {
  State next_state;
  StrIndex str_index;
};
#pragma pack (pop)

const unsigned c_alphabet_size = 256;

class StateMatrix {
private:
  StateElement* data;
public:
  StateMatrix(size_t max_states) {
    assert(max_states <= std::numeric_limits<State>::max());
    data = new StateElement[max_states * c_alphabet_size];
    memset(data, 0, max_states * c_alphabet_size * sizeof(StateElement));
  }
  ~StateMatrix() {
    delete[] data;
  }
  StateElement& at(State state, unsigned char value) {
    return data[state * c_alphabet_size + value];
  }
  const StateElement& at(State state, unsigned char value) const {
    return data[state * c_alphabet_size + value];
  }
};

static std::unique_ptr<StateMatrix> create_state_matrix(const std::vector<SigData>& str_list) {
  size_t max_states = 0;
  for (unsigned i = 0; i < str_list.size(); i++) {
    max_states += str_list[i].signature.size();
  }
  auto matrix = std::make_unique<StateMatrix>(max_states);
  State current_state = 0;
  for (StrIndex i = 0; i < str_list.size(); i++) {
    const ByteVector& str = str_list[i].signature;
    State state = 0;
    for (unsigned j = 0; j + 1 < str.size(); j++) {
      StateElement& st_elem = matrix->at(state, str[j]);
      if (st_elem.next_state) { // state already present
        state = st_elem.next_state;
      }
      else {
        current_state++;
        state = current_state;
        st_elem.next_state = state;
      }
    }
    if (str.size()) {
      matrix->at(state, str[str.size() - 1]).str_index = i + 1;
    }
  }
  return matrix;
}

std::vector<StrPos> msearch(unsigned char* data, size_t size, const std::vector<SigData>& str_list, bool eof) {
  static bool uniq_by_type = false;
  std::vector<StrPos> result;
  if (str_list.empty())
    return result;
  result.reserve(str_list.size());
  const auto matrix = create_state_matrix(str_list);
  std::vector<bool> found(str_list.size(), false);
  for (size_t i = 0; i < size; i++) {
    State state = 0;
    for (size_t j = i; j < size; j++) {
      const StateElement& st_elem = matrix->at(state, data[j]);
      if (st_elem.str_index) { // found signature
        StrIndex str_index = st_elem.str_index - 1;
        auto format = str_list[str_index].format;
        if (i >= format.SignatureOffset && (!uniq_by_type || !found[str_index])) { // more detailed check
          UInt32 is_arc = format.IsArc ? format.IsArc(data+i-format.SignatureOffset, size-i+format.SignatureOffset) : k_IsArc_Res_YES;
          if (is_arc == k_IsArc_Res_YES || (is_arc == k_IsArc_Res_NEED_MORE && !eof)) {
            found[str_index] = true;
            result.emplace_back(StrPos(str_index, i));
          }
        }
      }
      if (st_elem.next_state) {
        state = st_elem.next_state;
      }
      else
        break;
    }
  }
  std::sort(result.begin(), result.end());
  result.shrink_to_fit();
  return result;
}
