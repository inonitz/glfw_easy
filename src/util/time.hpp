#pragma once
#include <chrono>
#include "ifcrash.hpp"


namespace Time {


using nanosecond  = std::chrono::nanoseconds;
using millisecond = std::chrono::milliseconds;
using timepoint_nano  = std::chrono::time_point<std::chrono::high_resolution_clock, nanosecond>;
using timepoint_milli = std::chrono::time_point<std::chrono::high_resolution_clock, millisecond>;
using dursecondf32 = std::chrono::duration<float>;
using dursecondf64 = std::chrono::duration<double>;


template<
    class clock_t    = std::chrono::high_resolution_clock,
    class result_t   = millisecond,
    class duration_t = millisecond>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}


template<class clock_t = std::chrono::high_resolution_clock> auto now() { return clock_t::now(); }


template<
    class DT = std::chrono::nanoseconds,
    class ClockT = std::chrono::high_resolution_clock>
class Timer
{
public:
    using timep_t = typename ClockT::time_point;
    using timep_dt = DT;


    void tick() { 
        _end = timep_t{}; 
        _start = ClockT::now(); 
    }    
    void tock() { _end = ClockT::now(); }
    

    template <class T = DT> 
    auto duration() const { 
        ifcrashstr(_end == timep_t{}, "tock before reporting"); 
        return std::chrono::duration_cast<T>(_end - _start); 
    }
private:
    timep_t _start = ClockT::now(), _end = {};
};


template<
    class TimeT  = std::chrono::nanoseconds,
    class ClockT = std::chrono::high_resolution_clock>
struct measure
{
    template<class F, class ...Args>
    static auto duration(F&& func, Args&&... args)
    {
        auto start = ClockT::now();
        std::invoke(std::forward<F>(func), std::forward<Args>(args)...);
        return std::chrono::duration_cast<TimeT>(ClockT::now()-start);
    }
};


template<
    typename Before,
    typename After>
auto duration_cast(Before const& timepoint) {
    return std::chrono::duration_cast<After>(timepoint);
}


template< typename _From > auto to_milli(_From const& timepoint) {
    return duration_cast<_From, millisecond>(timepoint);
}
template< typename _From > auto to_nano(_From const& timepoint) {
    return duration_cast<_From, nanosecond>(timepoint);
}


} // namespace Time