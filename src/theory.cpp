#include <cstdint>
#include <string>

#include "disp.hpp"
#include "keys.hpp"

#include "theory.hpp"

void init_theory() {}

void update_music_state(key keys[]) {
    set_chord("t0.txt=\"Cm\"", "t1.txt=\"C->D#\"");
}
