#include "win_instance.hpp"
#include "internal.hpp"


namespace AWC {


WindowContext* getCurrentlyActiveWindow() {
    return getActiveContext().win;
}


} // namespace AWC