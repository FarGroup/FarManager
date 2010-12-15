#include "utils.hpp"
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
    assert(max_states <= numeric_limits<State>::max());
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

StateMatrix* create_state_matrix(const vector<ByteVector>& str_list) {
  size_t max_states = 0;
  for (unsigned i = 0; i < str_list.size(); i++) {
    max_states += str_list[i].size();
  }
  StateMatrix* matrix = new StateMatrix(max_states);
  State current_state = 0;
  for (StrIndex i = 0; i < str_list.size(); i++) {
    const ByteVector& str = str_list[i];
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

vector<StrPos> msearch(unsigned char* data, size_t size, const vector<ByteVector>& str_list) {
  vector<StrPos> result;
  if (str_list.empty())
    return result;
  result.reserve(str_list.size());
  auto_ptr<StateMatrix> matrix(create_state_matrix(str_list));
  vector<bool> found(str_list.size(), false);
  for (size_t i = 0; i < size; i++) {
    State state = 0;
    for (size_t j = i; j < size; j++) {
      const StateElement& st_elem = matrix->at(state, data[j]);
      if (st_elem.str_index) { //found
        StrIndex str_index = st_elem.str_index - 1;
        if (!found[str_index]) {
          found[str_index] = true;
          result.push_back(StrPos(str_index, i));
        }
      }
      if (st_elem.next_state) {
        state = st_elem.next_state;
      }
      else
        break;
    }
  }
  sort(result.begin(), result.end());
  result.shrink_to_fit();
  return result;
}
