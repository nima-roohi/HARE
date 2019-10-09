#ifndef CMN__MISC__OP_TYPE__HPP
#define CMN__MISC__OP_TYPE__HPP

#include <cstdint>

namespace cmn::misc {

enum class op_type : std::uint8_t {
  PREFIX, INFIX, POSTFIX
};

template<typename Out>
inline Out& operator<<(Out& os, const op_type type) {
  switch(type) {
  case op_type::PREFIX : { os << "prefix" ; return os; }
  case op_type::INFIX  : { os << "infix"  ; return os; }
  case op_type::POSTFIX: { os << "postfix"; return os; }
  }
}

} // namespace cmn::misc

#endif // CMN__MISC__OP_TYPE__HPP
