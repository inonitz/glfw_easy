// #include "test_func.hpp"
#include "sim.hpp"



int main() {
    // return test_functionality();
    
    SimulationData* sim = amalloc_t(
        SimulationData, 
        sizeof(SimulationData), 
        round2(sizeof(SimulationData))
    );


    mark(); sim->init(10.0f, 100, 100, 3000);
    mark(); sim->run();

    mark(); sim->destroy();
}