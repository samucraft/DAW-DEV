#ifndef DAW_SOUND_H
#define DAW_SOUND_H

typedef enum signal_type {
    WAVE_e = 0,
    KS_e   = 1,
} SIGNAL_TYPE;

uint8_t init_sound();

void cleanup_sound();

void trigger_gate(uint8_t index);

void trigger_sample(uint8_t index);

void trigger_vibrato();

void change_sound_type(SIGNAL_TYPE type);

void set_volume(float volume);

#endif
