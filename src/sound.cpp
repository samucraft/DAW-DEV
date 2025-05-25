#include <cstdint>
#include <iostream>
#include <math.h>
#include <portaudio.h>

#include "sound.hpp"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

#define MAX_TONES 5

typedef struct {
    float phase1;
    float phase2;
    float phase3;
    float phase4;
    float phase5;
    float frequency1;
    float frequency2;
    float frequency3;
    float frequency4;
    float frequency5;
    float amplitude1;
    float amplitude2;
    float amplitude3;
    float amplitude4;
    float amplitude5;
} Wave;

static Wave data = {
    .phase1 = 0.0f,
    .phase2 = 0.0f,
    .phase3 = 0.0f,
    .phase4 = 0.0f,
    .phase5 = 0.0f,
    .frequency1 = 440.63f,
    .frequency2 = 440.63f,
    .frequency3 = 440.63f,
    .frequency4 = 440.63f,
    .frequency5 = 440.63f,
    .amplitude1 = 0.25f,
    .amplitude2 = 0.25f,
    .amplitude3 = 0.25f,
    .amplitude4 = 0.25f,
    .amplitude5 = 0.25f
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
    float phase1 = data->phase1;
    float phase2 = data->phase2;
    float phase3 = data->phase3;
    float phase4 = data->phase4;
    float phase5 = data->phase5;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float sample1 = gates[0] ? data->amplitude1 * sinf(phase1) : 0.0f;
        float sample2 = gates[1] ? data->amplitude2 * sinf(phase2) : 0.0f;
        float sample3 = gates[2] ? data->amplitude3 * sinf(phase3) : 0.0f;
        float sample4 = gates[3] ? data->amplitude4 * sinf(phase4) : 0.0f;
        float sample5 = gates[4] ? data->amplitude5 * sinf(phase5) : 0.0f;

        *out++ = sample1 + sample2 + sample3 + sample4 + sample5;  // Left
        *out++ = sample1 + sample2 + sample3 + sample4 + sample5;  // Right
        
        phase1 += 2.0f * (float)M_PI * data->frequency1 / SAMPLE_RATE;
        if (phase1 >= 2.0f * (float)M_PI)
            phase1 -= 2.0f * (float)M_PI;

        phase2 += 2.0f * (float)M_PI * data->frequency2 / SAMPLE_RATE;
        if (phase2 >= 2.0f * (float)M_PI)
            phase2 -= 2.0f * (float)M_PI;

        phase3 += 2.0f * (float)M_PI * data->frequency3 / SAMPLE_RATE;
        if (phase3 >= 2.0f * (float)M_PI)
            phase3 -= 2.0f * (float)M_PI;

        phase4 += 2.0f * (float)M_PI * data->frequency4 / SAMPLE_RATE;
        if (phase4 >= 2.0f * (float)M_PI)
            phase4 -= 2.0f * (float)M_PI;

        phase5 += 2.0f * (float)M_PI * data->frequency5 / SAMPLE_RATE;
        if (phase5 >= 2.0f * (float)M_PI)
            phase5 -= 2.0f * (float)M_PI;
    }

    data->phase1 = phase1;
    data->phase2 = phase2;
    data->phase3 = phase3;
    data->phase4 = phase4;
    data->phase5 = phase5;
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
