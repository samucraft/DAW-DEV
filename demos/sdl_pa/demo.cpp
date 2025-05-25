#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

#define VIBRATO_FREQUENCY 10.0f // 5 Hz vibrato
#define VIBRATO_DEPTH     10.0f // deviation in Hz

typedef struct {
    float phase1;
    float phase2;
    float phase3;
    float frequency1;
    float frequency2;
    float frequency3;
    float amplitude1;
    float amplitude2;
    float amplitude3;
    float vibratoPhase;
} SineWaves;

bool gate1, gate2, gate3;
bool mode1, mode2, mode3;
bool vibrato;
float vibratoDepth;

float sine_wave(float amplitude, float phase) {
    return amplitude * sinf(phase);
}

float square_wave(float amplitude, float phase) {
    if (phase >= M_PI)
        return amplitude;
    return -amplitude;
}

float select_wave(float amplitude, float phase, bool mode) {
    if (mode)
        return square_wave(amplitude, phase);
    return sine_wave(amplitude, phase);
}

// Audio callback function
static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    SineWaves *data = (SineWaves *)userData;
    float *out = (float *)outputBuffer;
    float phase1 = data->phase1;
    float phase2 = data->phase2;
    float phase3 = data->phase3;
    float vibratoPhase = data->vibratoPhase;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        float vibratoMod = vibrato ? vibratoDepth * sinf(vibratoPhase) : 0.0f;

        float current_frequency1 = data->frequency1 + vibratoMod;
        float current_frequency2 = data->frequency2 + vibratoMod;
        float current_frequency3 = data->frequency3 + vibratoMod;

        float sample1 = gate1 ? select_wave(data->amplitude1, phase1, mode1) : 0.0f;
        float sample2 = gate2 ? select_wave(data->amplitude2, phase2, mode2) : 0.0f;
        float sample3 = gate3 ? select_wave(data->amplitude3, phase3, mode3) : 0.0f;

        *out++ = sample1 + sample2 + sample3;  // Left
        *out++ = sample1 + sample2 + sample3;  // Right
        
        phase1 += 2.0f * (float)M_PI * current_frequency1 / SAMPLE_RATE;
        if (phase1 >= 2.0f * (float)M_PI)
            phase1 -= 2.0f * (float)M_PI;

        phase2 += 2.0f * (float)M_PI * current_frequency2 / SAMPLE_RATE;
        if (phase2 >= 2.0f * (float)M_PI)
            phase2 -= 2.0f * (float)M_PI;

        phase3 += 2.0f * (float)M_PI * current_frequency3 / SAMPLE_RATE;
        if (phase3 >= 2.0f * (float)M_PI)
            phase3 -= 2.0f * (float)M_PI;

        vibratoPhase += 2.0f * (float)M_PI * VIBRATO_FREQUENCY / SAMPLE_RATE;
        if (vibratoPhase >= 2.0f * (float)M_PI)
            vibratoPhase -= 2.0f * (float)M_PI;
    }

    data->phase1 = phase1;
    data->phase2 = phase2;
    data->phase3 = phase3;
    data->vibratoPhase = vibratoPhase;
    return paContinue;
}

int main(void) {
    PaStream *stream;
    PaError err;
    SineWaves data = {
        .phase1 = 0.0f,
        .phase2 = 0.0f,
        .phase3 = 0.0f,
        .frequency1 = 261.63f,   // Change this to hear different notes
        .frequency2 = 329.63f,   // Change this to hear different notes
        .frequency3 = 392.00f,   // Change this to hear different notes
        .amplitude1 = 0.25f,     // Safe volume
        .amplitude2 = 0.25f,     // Safe volume
        .amplitude3 = 0.25f,     // Safe volume
        .vibratoPhase = 0.0f
    };

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

    printf("Playing 440 Hz sine wave. Press Enter to stop...\n");

    char key;
    bool running = true;
    gate1 = false;
    gate2 = false;
    gate3 = false;
    mode1 = false;
    mode2 = false;
    mode3 = false;
    vibrato = false;
    vibratoDepth = (VIBRATO_DEPTH / VIBRATO_FREQUENCY) * 2.0f * M_PI;
    while(running) {
        key = getchar();
        switch (key) {
            case 'q':
                running = false;
                break;

            case 'c':
                gate1 = !gate1;
                printf("Cg is %u\n", gate1);
                break;

            case 'd':
                gate2 = !gate2;
                printf("Dg is %u\n", gate2);
                break;

            case 'e':
                gate3 = !gate3;
                printf("Eg is %u\n", gate3);
                break;

            case 'v':
                mode1 = !mode1;
                printf("Cm is %u\n", mode1);
                break;

            case 'f':
                mode2 = !mode2;
                printf("Dm is %u\n", mode2);
                break;

            case 'r':
                mode3 = !mode3;
                printf("Em is %u\n", mode3);
                break;

            case 't':
                vibrato = !vibrato;
                printf("Vib. is %u\n", vibrato);
                break;

            default:
                printf("???\n");
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}

