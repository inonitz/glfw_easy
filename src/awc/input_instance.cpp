#include "input_instance.hpp"
#include "internal.hpp"


namespace AWC::Input {


InputUnit* getCurrentlyActiveInputUnit() {
    return getActiveContext().unit;
}


} // namespace AWC::Input