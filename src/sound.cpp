#include <cstdint>
#include <iostream>
#include <math.h>
#include <portaudio.h>
#include <sndfile.h>

#include "sound.hpp"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

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
    float phase6;
    float phase7;
    float phase8;
    float phase9;
    float phase10;
    float phase11;
    float phase12;
    float frequency1;
    float frequency2;
    float frequency3;
    float frequency4;
    float frequency5;
    float frequency6;
    float frequency7;
    float frequency8;
    float frequency9;
    float frequency10;
    float frequency11;
    float frequency12;
    float amplitude1;
    float amplitude2;
    float amplitude3;
    float amplitude4;
    float amplitude5;
    float amplitude6;
    float amplitude7;
    float amplitude8;
    float amplitude9;
    float amplitude10;
    float amplitude11;
    float amplitude12;
    volatile float vibratoPhase;
    volatile uint8_t repetitions_left;
} Wave;

static Wave data = {
    .phase1 = 0.0f,
    .phase2 = 0.0f,
    .phase3 = 0.0f,
    .phase4 = 0.0f,
    .phase5 = 0.0f,
    .phase6 = 0.0f,
    .phase7 = 0.0f,
    .phase8 = 0.0f,
    .phase9 = 0.0f,
    .phase10 = 0.0f,
    .phase11 = 0.0f,
    .phase12 = 0.0f,
    .frequency1 = 261.63f,
    .frequency2 = 277.18f,
    .frequency3 = 293.66f,
    .frequency4 = 311.13f,
    .frequency5 = 329.63f,
    .frequency6 = 349.23f,
    .frequency7 = 369.99f,
    .frequency8 = 392.00f,
    .frequency9 = 415.30f,
    .frequency10 = 440.00f,
    .frequency11 = 466.16f,
    .frequency12 = 493.88f,
    .amplitude1 = 0.25f,
    .amplitude2 = 0.25f,
    .amplitude3 = 0.25f,
    .amplitude4 = 0.25f,
    .amplitude5 = 0.25f,
    .amplitude6 = 0.25f,
    .amplitude7 = 0.25f,
    .amplitude8 = 0.25f,
    .amplitude9 = 0.25f,
    .amplitude10 = 0.25f,
    .amplitude11 = 0.25f,
    .amplitude12 = 0.25f,
    .vibratoPhase = 0.0f,
    .repetitions_left = 0
};

static PaStream *stream;

static bool gates[] = {
    false, false, false, false,
    false, false, false, false,
    false, false, false, false
};

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
    float phase6 = data->phase6;
    float phase7 = data->phase7;
    float phase8 = data->phase8;
    float phase9 = data->phase9;
    float phase10 = data->phase10;
    float phase11 = data->phase11;
    float phase12 = data->phase12;
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
        float current_frequency6 = data->frequency6 + vibratoMod;
        float current_frequency7 = data->frequency7 + vibratoMod;
        float current_frequency8 = data->frequency8 + vibratoMod;
        float current_frequency9 = data->frequency9 + vibratoMod;
        float current_frequency10 = data->frequency10 + vibratoMod;
        float current_frequency11 = data->frequency11 + vibratoMod;
        float current_frequency12 = data->frequency12 + vibratoMod;

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
        
        float sample1  = gates[0]  ? data->amplitude1 * sinf(phase1)  : 0.0f;
        float sample2  = gates[1]  ? data->amplitude2 * sinf(phase2)  : 0.0f;
        float sample3  = gates[2]  ? data->amplitude3 * sinf(phase3)  : 0.0f;
        float sample4  = gates[3]  ? data->amplitude4 * sinf(phase4)  : 0.0f;
        float sample5  = gates[4]  ? data->amplitude5 * sinf(phase5)  : 0.0f;
        float sample6  = gates[5]  ? data->amplitude5 * sinf(phase6)  : 0.0f;
        float sample7  = gates[6]  ? data->amplitude5 * sinf(phase7)  : 0.0f;
        float sample8  = gates[7]  ? data->amplitude5 * sinf(phase8)  : 0.0f;
        float sample9  = gates[8]  ? data->amplitude5 * sinf(phase9)  : 0.0f;
        float sample10 = gates[9]  ? data->amplitude5 * sinf(phase10) : 0.0f;
        float sample11 = gates[10] ? data->amplitude5 * sinf(phase11) : 0.0f;
        float sample12 = gates[11] ? data->amplitude5 * sinf(phase12) : 0.0f;

        *out++ = sample1 + sample2 + sample3 + sample4
                 + sample5 + sample6 + sample7 + sample8
                 + sample9 + sample10 + sample11 + sample12
                 + left_kick_sample + left_snare_sample + left_hi_hat_sample;   // Left
        *out++ = sample1 + sample2 + sample3 + sample4
                 + sample5 + sample6 + sample7 + sample8
                 + sample9 + sample10 + sample11 + sample12
                 + right_kick_sample + right_snare_sample + right_hi_hat_sample;// Right
        
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

        phase6 += 2.0f * (float)M_PI * current_frequency6 / SAMPLE_RATE;
        if (phase6 >= 2.0f * (float)M_PI)
            phase6 -= 2.0f * (float)M_PI;

        phase7 += 2.0f * (float)M_PI * current_frequency7 / SAMPLE_RATE;
        if (phase7 >= 2.0f * (float)M_PI)
            phase7 -= 2.0f * (float)M_PI;

        phase8 += 2.0f * (float)M_PI * current_frequency8 / SAMPLE_RATE;
        if (phase8 >= 2.0f * (float)M_PI)
            phase8 -= 2.0f * (float)M_PI;

        phase9 += 2.0f * (float)M_PI * current_frequency9 / SAMPLE_RATE;
        if (phase9 >= 2.0f * (float)M_PI)
            phase9 -= 2.0f * (float)M_PI;

        phase10 += 2.0f * (float)M_PI * current_frequency10 / SAMPLE_RATE;
        if (phase10 >= 2.0f * (float)M_PI)
            phase10 -= 2.0f * (float)M_PI;

        phase11 += 2.0f * (float)M_PI * current_frequency11 / SAMPLE_RATE;
        if (phase11 >= 2.0f * (float)M_PI)
            phase11 -= 2.0f * (float)M_PI;

        phase12 += 2.0f * (float)M_PI * current_frequency12 / SAMPLE_RATE;
        if (phase12 >= 2.0f * (float)M_PI)
            phase12 -= 2.0f * (float)M_PI;

        vibratoPhase += 2.0f * (float)M_PI * VIBRATO_FREQUENCY / SAMPLE_RATE;
        if (vibratoPhase >= 2.0f * (float)M_PI)
            vibratoPhase -= 2.0f * (float)M_PI;
    }

    data->phase1 = phase1;
    data->phase2 = phase2;
    data->phase3 = phase3;
    data->phase4 = phase4;
    data->phase5 = phase5;
    data->phase6 = phase6;
    data->phase7 = phase7;
    data->phase8 = phase8;
    data->phase9 = phase9;
    data->phase10 = phase10;
    data->phase11 = phase11;
    data->phase12 = phase12;

    data->vibratoPhase = vibratoPhase;

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
