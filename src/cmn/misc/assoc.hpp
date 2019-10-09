#ifndef CMN__MISC__ASSOC__HPP
#define CMN__MISC__ASSOC__HPP

#include <cstdint>

namespace cmn::misc {

enum class associativity : std::uint8_t {
  NONE, LEFT, RIGHT, BOTH
};

template<typename Out>
inline Out& operator<<(Out& os, const associativity assoc) {
  switch(assoc) {
  case associativity::NONE : { os << "none" ; return os; }
  case associativity::BOTH : { os << "both" ; return os; }
  case associativity::LEFT : { os << "left" ; return os; }
  case associativity::RIGHT: { os << "right"; return os; }
  }
}

} // namespace cmn::misc

#endif // CMN__MISC__ASSOC__HPP
