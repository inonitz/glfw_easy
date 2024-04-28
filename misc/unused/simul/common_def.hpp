#pragma once
#include "util/vec.hpp"
#include "util/time.hpp"
#include <vector>
#include <array>


struct Particle
{
    math::vec2f pos;
    math::vec2f vel;
};


using ParticleBuffer = std::vector<Particle>;



template<u32 k_sizePerCapture>
struct measure_send_buffer
{
    using snapshot = std::array<Time::Timer<>::timep_dt, k_sizePerCapture>;
    Time::Timer<> clock;
    std::vector< snapshot > measure_pts;
    u32 push;


    measure_send_buffer(u32 minimumCaptures = 0) : measure_pts(minimumCaptures), push{k_sizePerCapture} {}


    __force_inline void begin_measure() { clock.tick(); return; } 
    __force_inline void end_measure() { 
        clock.tock();
        if(push == k_sizePerCapture) {
            measure_pts.push_back(snapshot{});
            push = 0;
        }
        measure_pts.back()[push] = clock.duration();
        ++push;
        return;
    }


    auto& points() const { return measure_pts; }
};


static measure_send_buffer<28> s_capture_buf{0};


#define capture_line(...) s_capture_buf.begin_measure(); __VA_ARGS__; s_capture_buf.end_measure();
#define capture_buffer(index) s_capture_buf.points()[index]