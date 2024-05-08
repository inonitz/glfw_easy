#include "util/marker.hpp"
#include "render_particles.hpp"
#include "render_old.hpp"


int main() {
    // MARKER_FLAG_REDIRECT_TO_FILE_INIT();
    // auto exitid = render_particles();
    // MARKER_FLAG_REDIRECT_TO_FILE_END();
    // return exitid;
    return render_particles_old();
}