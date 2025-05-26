#include <cstdint>
#include <iostream>
#include <math.h>
#include <portaudio.h>
#include <sndfile.h>

#include "sound.hpp"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

#define MAX_TONES 5

#define KICK_SAMPLE_PATH   "sounds/kick.wav"
#define SNARE_SAMPLE_PATH  "sounds/snare.wav"
#define HI_HAT_SAMPLE_PATH "sounds/hi-hat.wav"

#define VIBRATO_FREQUENCY   10.0f // 5 Hz vibrato
#define VIBRATO_DEPTH       10.0f // deviation in Hz
#define MAX_VIBRATO_WINDOWS 200

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
    volatile float vibratoPhase;
    volatile uint8_t repetitions_left;
} Wave;

static Wave data = {
    .phase1 = 0.0f,
    .phase2 = 0.0f,
    .phase3 = 0.0f,
    .phase4 = 0.0f,
    .phase5 = 0.0f,
    .frequency1 = 261.63f,
    .frequency2 = 329.63f,
    .frequency3 = 392.00f,
    .frequency4 = 493.88f,
    .frequency5 = 523.26f,
    .amplitude1 = 0.25f,
    .amplitude2 = 0.25f,
    .amplitude3 = 0.25f,
    .amplitude4 = 0.25f,
    .amplitude5 = 0.25f,
    .vibratoPhase = 0.0f,
    .repetitions_left = 0
};

static PaStream *stream;

static bool gates[] = {false, false, false, false, false};

struct Sample {
    float* data;       // Interleaved stereo or mono
    int length;        // In frames
    int channels;
    int sampleRate;
    volatile int position;      // Playback position
    volatile bool playing;
};

static Sample kick_sample;
static Sample snare_sample;
static Sample hi_hat_sample;

static volatile bool vibrato;
static float vibratoDepth;

float* loadWavFile(const char *path, int *numFrames, int *numChannels, int *sampleRate) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(path, SFM_READ, &sfinfo);
    if (!file) {
        fprintf(stderr, "Failed to open file %s\n", path);
        return nullptr;
    }

    *numFrames = sfinfo.frames;
    *numChannels = sfinfo.channels;
    *sampleRate = sfinfo.samplerate;

    float* data = new float[(*numFrames) * (*numChannels)];
    sf_readf_float(file, data, (*numFrames));
    sf_close(file);
    return data;
}


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
    float vibratoPhase = data->vibratoPhase;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        // Comput vibrato if enabled
        float vibratoMod = vibrato ? vibratoDepth * sinf(vibratoPhase) : 0.0f;

        // Add vibrato to frequency is computed (FM modulation)
        float current_frequency1 = data->frequency1 + vibratoMod;
        float current_frequency2 = data->frequency2 + vibratoMod;
        float current_frequency3 = data->frequency3 + vibratoMod;
        float current_frequency4 = data->frequency4 + vibratoMod;
        float current_frequency5 = data->frequency5 + vibratoMod;

        // Mix sample playback (kick) if active
        float left_kick_sample  = 0.0f;
        float right_kick_sample = 0.0f;
        if (kick_sample.playing && kick_sample.position < kick_sample.length) {
            left_kick_sample = kick_sample.data[kick_sample.position * kick_sample.channels];    // Left
            right_kick_sample = kick_sample.data[kick_sample.position * kick_sample.channels + 1];// Right

            // Advance sample positions
            kick_sample.position++;
            if (kick_sample.position >= kick_sample.length) {
                kick_sample.playing = false;
            }
        }

        float left_snare_sample  = 0.0f;
        float right_snare_sample = 0.0f;
        if (snare_sample.playing && snare_sample.position < snare_sample.length) {
            left_snare_sample = snare_sample.data[snare_sample.position * snare_sample.channels];    // Left
            right_snare_sample = snare_sample.data[snare_sample.position * snare_sample.channels + 1];// Right

            // Advance sample positions
            snare_sample.position++;
            if (snare_sample.position >= snare_sample.length) {
                snare_sample.playing = false;
            }
        }

        float left_hi_hat_sample  = 0.0f;
        float right_hi_hat_sample = 0.0f;
        if (hi_hat_sample.playing && hi_hat_sample.position < hi_hat_sample.length) {
            left_hi_hat_sample = hi_hat_sample.data[hi_hat_sample.position * hi_hat_sample.channels];    // Left
            right_hi_hat_sample = hi_hat_sample.data[hi_hat_sample.position * hi_hat_sample.channels + 1];// Right

            // Advance sample positions
            hi_hat_sample.position++;
            if (hi_hat_sample.position >= hi_hat_sample.length) {
                hi_hat_sample.playing = false;
            }
        }
        
        float sample1 = gates[0] ? data->amplitude1 * sinf(phase1) : 0.0f;
        float sample2 = gates[1] ? data->amplitude2 * sinf(phase2) : 0.0f;
        float sample3 = gates[2] ? data->amplitude3 * sinf(phase3) : 0.0f;
        float sample4 = gates[3] ? data->amplitude4 * sinf(phase4) : 0.0f;
        float sample5 = gates[4] ? data->amplitude5 * sinf(phase5) : 0.0f;

        *out++ = sample1 + sample2 + sample3 + sample4 + sample5 + left_kick_sample + left_snare_sample + left_hi_hat_sample;   // Left
        *out++ = sample1 + sample2 + sample3 + sample4 + sample5 + right_kick_sample + right_snare_sample + right_hi_hat_sample;// Right
        
        phase1 += 2.0f * (float)M_PI * current_frequency1 / SAMPLE_RATE;
        if (phase1 >= 2.0f * (float)M_PI)
            phase1 -= 2.0f * (float)M_PI;

        phase2 += 2.0f * (float)M_PI * current_frequency2 / SAMPLE_RATE;
        if (phase2 >= 2.0f * (float)M_PI)
            phase2 -= 2.0f * (float)M_PI;

        phase3 += 2.0f * (float)M_PI * current_frequency3 / SAMPLE_RATE;
        if (phase3 >= 2.0f * (float)M_PI)
            phase3 -= 2.0f * (float)M_PI;

        phase4 += 2.0f * (float)M_PI * current_frequency4 / SAMPLE_RATE;
        if (phase4 >= 2.0f * (float)M_PI)
            phase4 -= 2.0f * (float)M_PI;

        phase5 += 2.0f * (float)M_PI * current_frequency5 / SAMPLE_RATE;
        if (phase5 >= 2.0f * (float)M_PI)
            phase5 -= 2.0f * (float)M_PI;

        vibratoPhase += 2.0f * (float)M_PI * VIBRATO_FREQUENCY / SAMPLE_RATE;
        if (vibratoPhase >= 2.0f * (float)M_PI)
            vibratoPhase -= 2.0f * (float)M_PI;
    }

    data->phase1 = phase1;
    data->phase2 = phase2;
    data->phase3 = phase3;
    data->phase4 = phase4;
    data->phase5 = phase5;

    if (vibrato) {
        data->repetitions_left--;
        if (data->repetitions_left == 0) {
            vibrato = false;
            std::cout << "Vibrato ended\n";
        }
    }

    return paContinue;
}

uint8_t init_sound() {
    PaError err;

    kick_sample.data = loadWavFile(KICK_SAMPLE_PATH,
                                   &kick_sample.length,
                                   &kick_sample.channels,
                                   &kick_sample.sampleRate);
    kick_sample.position = 0;
    kick_sample.playing = false;

    if (kick_sample.sampleRate != SAMPLE_RATE) {
        fprintf(stderr, "Warning: kick sample rate mismatch (sample: %d, stream: %d)\n",
                kick_sample.sampleRate, SAMPLE_RATE);
        return 1;
    }

    snare_sample.data = loadWavFile(SNARE_SAMPLE_PATH,
                                    &snare_sample.length,
                                    &snare_sample.channels,
                                    &snare_sample.sampleRate);
    snare_sample.position = 0;
    snare_sample.playing = false;

    if (snare_sample.sampleRate != SAMPLE_RATE) {
        fprintf(stderr, "Warning: snare sample rate mismatch (sample: %d, stream: %d)\n",
                snare_sample.sampleRate, SAMPLE_RATE);
        return 1;
    }

    hi_hat_sample.data = loadWavFile(HI_HAT_SAMPLE_PATH,
                                     &hi_hat_sample.length,
                                     &hi_hat_sample.channels,
                                     &hi_hat_sample.sampleRate);
    hi_hat_sample.position = 0;
    hi_hat_sample.playing = false;

    if (hi_hat_sample.sampleRate != SAMPLE_RATE) {
        fprintf(stderr, "Warning: hi hat sample rate mismatch (sample: %d, stream: %d)\n",
                hi_hat_sample.sampleRate, SAMPLE_RATE);
        return 1;
    }

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

    vibrato = false;
    vibratoDepth = (VIBRATO_DEPTH / VIBRATO_FREQUENCY) * 2.0f * M_PI;

    return 0;
}

void cleanup_sound() {
    delete[] kick_sample.data;
    delete[] snare_sample.data;
    delete[] hi_hat_sample.data;

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
}

void trigger_gate(uint8_t index) {
    gates[index] = !gates[index];
}

void trigger_sample(uint8_t index) {
    switch (index) {
        case 0:
            std::cout << "kick trigger\n";
            kick_sample.position = 0;
            kick_sample.playing = true;
            break;

        case 1:
            std::cout << "snare trigger\n";
            snare_sample.position = 0;
            snare_sample.playing = true;
            break;

        case 2:
            std::cout << "hi hat trigger\n";
            hi_hat_sample.position = 0;
            hi_hat_sample.playing = true;
            break;

        default:
            std::cout << "trigger index ?\n";
            break;
    }
}

void trigger_vibrato() {
    vibrato = true;
    data.repetitions_left = MAX_VIBRATO_WINDOWS;
    std::cout << "Vibrato started\n";
}
