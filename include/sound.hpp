#ifndef DAW_SOUND_H
#define DAW_SOUND_H

uint8_t init_sound();

void cleanup_sound();

void trigger_gate(uint8_t index);

void trigger_sample(uint8_t index);

void trigger_vibrato();

#endif
