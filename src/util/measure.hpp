#pragma once
#include "base.hpp"
#include <chrono>
#include <array>
#include <vector>
#include <string_view>
#include <memory>



#define BENCHMARK_STATUS_FINISHED    (0b000)
#define BENCHMARK_STATUS_TBD         (0b001)
#define BENCHMARK_STATUS_SUCCESS     (0b010)
#define BENCHMARK_STATUS_FAIL        (0b100)


struct BenchmarkResult
{
    using chrono_ns = std::chrono::nanoseconds;
    std::array<char, 200> name; /* totally arbitrary array size, i exepct no more than 200 chars per name */
    f32                   measured_ns;
    u8                    status;
};


struct Benchmark {
    std::string_view title;
    std::unique_ptr<std::vector<BenchmarkResult>> data;
};


template <class TimeUnit = std::chrono::nanoseconds, class ClockType = std::chrono::steady_clock> class Timer {
    using TimePoint = typename ClockType::time_point;
    
	TimePoint m_start, m_end;

public:
    void tick() { 
        m_end = TimePoint{}; 
        m_start = ClockType::now(); 
    }
    
    void tock() { m_end = ClockType::now(); }
    
    template <class T = TimeUnit> auto duration() const {  
        return std::chrono::duration_cast<T>(m_end - m_start); 
    }

    template<class DataType> auto duration_cast(TimeUnit const& tu) const {
        return std::chrono::duration_cast<std::chrono::duration<DataType, std::nano>>(tu);
    }
};