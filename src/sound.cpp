#include <cstdint>
#include <iostream>
#include <math.h>
#include <portaudio.h>

#include "sound.hpp"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

#define MAX_TONES 5

typedef struct {
    float phase;
    float frequency;
    float amplitude;
} Wave;

static Wave data = {
    .phase = 0.0f,
    .frequency = 440.63f,
    .amplitude = 0.25f
};

static PaStream *stream;

static bool gates[] = {false, false, false, false, false};

// Audio callback function
static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    Wave *data = (Wave *)userData;
    float *out = (float *)outputBuffer;
    float phase = data->phase;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sample = gates[0] ? data->amplitude * sinf(phase) : 0.0f;

        *out++ = sample;  // Left
        *out++ = sample;  // Right
        
        phase += 2.0f * (float)M_PI * data->frequency / SAMPLE_RATE;
        if (phase >= 2.0f * (float)M_PI)
            phase -= 2.0f * (float)M_PI;
    }

    data->phase = phase;
    return paContinue;
}

uint8_t init_sound() {
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) {
        fprintf(stderr, "PortAudio error: %s\n", Pa_GetErrorText(err));
        return 1;
    }

    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        fprintf(stderr, "Error: No default output device.\n");
        Pa_Terminate();
        return 1;
    }

    outputParams.channelCount = 2;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency =
        Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = NULL;

    err = Pa_OpenStream(
        &stream,
        NULL,              // No input
        &outputParams,     // Output only
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,
        audioCallback,
        &data
    );

    if (err != paNoError) {
        fprintf(stderr, "Failed to open stream: %s\n", Pa_GetErrorText(err));
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        fprintf(stderr, "Failed to start stream: %s\n", Pa_GetErrorText(err));
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    return 0;
}

void cleanup_sound() {
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}

void trigger_gate(uint8_t index) {
    gates[index] = !gates[index];
}
