#pragma once
#include "base.hpp"
#include <chrono>
#include <ratio>




namespace Time {


template<
    class result_t   = std::chrono::milliseconds,
    class clock_t    = std::chrono::steady_clock,
    class duration_t = std::chrono::milliseconds>
auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}


template<
    class DT = std::chrono::nanoseconds,
    class ClockT = std::chrono::steady_clock>
class Timer
{
    using timep_t = typename ClockT::time_point;
    timep_t _start = ClockT::now(), _end = {};

public:
    void tick() { 
        _end = timep_t{}; 
        _start = ClockT::now(); 
    }
    
    void tock() { _end = ClockT::now(); }
    
    template <class T = DT> 
    auto duration() const { 
        ifcrashfmt_debug(_end == timep_t{}, "tock before reporting", 0); 
        return std::chrono::duration_cast<T>(_end - _start); 
    }
};


template<
    class TimeT  = std::chrono::nanoseconds,
    class ClockT = std::chrono::steady_clock>
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


} // namespace Time