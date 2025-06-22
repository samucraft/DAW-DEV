#include <cstdint>
#include <iostream>
#include <math.h>
#include <portaudio.h>
#include <sndfile.h>
#include <vector>

#include "keys.hpp"
#include "sound.hpp"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

#define KICK_SAMPLE_PATH   "sounds/kick.wav"
#define SNARE_SAMPLE_PATH  "sounds/snare.wav"
#define HI_HAT_SAMPLE_PATH "sounds/hi-hat.wav"

#define VIBRATO_FREQUENCY   10.0f // 5 Hz vibrato
#define VIBRATO_DEPTH       10.0f // deviation in Hz
#define MAX_VIBRATO_WINDOWS 200

#define DEFAULT_AMPLITUDE   0.25f
#define DEFAULT_PHASE       0.0f

#define C_FREQ  261.63f
#define Db_FREQ 277.18f
#define D_FREQ  293.66f
#define Eb_FREQ 311.13f
#define E_FREQ  329.63f
#define F_FREQ  349.23f
#define Gb_FREQ 369.99f
#define G_FREQ  392.00f
#define Ab_FREQ 415.30f
#define A_FREQ  440.00f
#define Bb_FREQ 466.16f
#define B_FREQ  493.88f

// For Karplus-Strong
#define KS_DECAY    0.996f // Damping factor

enum signal_type {
    WAVE_e = 0,
    KS_e   = 1,
};

typedef struct wave {
    float amplitude;
    float frequency;
    float phase;
} WAVE;

typedef struct ks {
    std::vector<float> buffer;
    int                index;
} KS;

typedef struct signal {
    bool             gate;
    enum signal_type type;
    WAVE             wave;
    KS               ks;
} SIGNAL;

typedef struct vibrato {
    volatile float   vibratoPhase;
    volatile uint8_t repetitions_left;
} VIBRATO;

typedef struct stream_data {
    SIGNAL           signals[MAX_KEYS];
    volatile VIBRATO vibrato;
} STREAM_DATA;

static STREAM_DATA stream_data = {
    {
        {false, KS_e,
            {DEFAULT_AMPLITUDE, C_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, Db_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, D_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, Eb_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, E_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, F_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, Gb_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, G_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, Ab_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, A_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, Bb_FREQ, DEFAULT_PHASE},
            {{}, 0}
        },
        {false, KS_e,
            {DEFAULT_AMPLITUDE, B_FREQ, DEFAULT_PHASE},
            {{}, 0}
        }
    },
    {DEFAULT_PHASE, 0}
};

static PaStream *stream;

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

static void initialize_ks(SIGNAL *signal) {
    int buffer_size = static_cast<int>(SAMPLE_RATE / signal->wave.frequency);
    signal->ks.buffer.resize(buffer_size);
    for (int i = 0; i < buffer_size; ++i) {
        signal->ks.buffer[i] = static_cast<float>((rand() / (float)RAND_MAX)
                                                  * 2.0 - 1.0);
    }
    signal->ks.index = 0;
}

void compute_waves(STREAM_DATA *data, float *left_sample, float *right_sample) {
    // Compute vibrato modulation if enabled
    float vibratoMod = vibrato ? vibratoDepth * sinf(data->vibrato.vibratoPhase)
                                 : 0.0f;

    for (size_t i = 0; i < MAX_KEYS; i++) {
        // If normal wave:
        if (data->signals[i].type == WAVE_e) {
            // Add vibrato modulation to wave frequency if needed (FM mod.)
            float current_frequency = data->signals[i].wave.frequency
                                      + vibratoMod;

            // Generate wave (sine wave is the only one supported at the moment)
            float sample = data->signals[i].gate
                            ? data->signals[i].wave.amplitude
                            * sinf(data->signals[i].wave.phase)
                            : 0.0f;
            
            // Save the sample for this signal into left and right full sample
            *left_sample += sample;
            *right_sample += sample;

            // Update wave phase
            data->signals[i].wave.phase += 2.0f * (float)M_PI
                                           * current_frequency / SAMPLE_RATE;
            if (data->signals[i].wave.phase >= 2.0f * (float)M_PI) {
                data->signals[i].wave.phase -= 2.0f * (float)M_PI;
            }
        }

        // If Karplus-Strong synthesis:
        else if (data->signals[i].type == KS_e) {
            if (data->signals[i].gate) {
                // Apply Karplus-Strong feedback
                float first = data->signals[i].ks.buffer[data->signals[i].ks.index];
                float next  = data->signals[i].ks.buffer[(data->signals[i].ks.index + 1)
                              % data->signals[i].ks.buffer.size()];
                float new_sample = KS_DECAY * 0.25f * (first + next);

                data->signals[i].ks.buffer[data->signals[i].ks.index] = new_sample;
                float sample = new_sample;

                data->signals[i].ks.index = (data->signals[i].ks.index + 1) % data->signals[i].ks.buffer.size();

                *left_sample += sample;
                *right_sample += sample;
            }
        }

        else {
            std::cout << "Key " << i << " is configured to invalid signal type"
                      << std::endl;
        }
    }

    // Update vibrato phase
    data->vibrato.vibratoPhase += 2.0f * (float)M_PI * VIBRATO_FREQUENCY
                                  / SAMPLE_RATE;
    if (data->vibrato.vibratoPhase >= 2.0f * (float)M_PI) {
        data->vibrato.vibratoPhase -= 2.0f * (float)M_PI;
    }
}

// Audio callback function
static int audioCallback(const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo *timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData) {
    STREAM_DATA *data = (STREAM_DATA *)userData;
    float       *out  = (float *)outputBuffer;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        /***********************************************************************
         ************************ Sample playback logic ************************
         **********************************************************************/

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

        /***********************************************************************
         ************************ Wave generation logic ************************
         **********************************************************************/

        float left_wave_samples = 0.0f;
        float right_wave_samples = 0.0f;
        compute_waves(data, &left_wave_samples, &right_wave_samples);

        /***********************************************************************
         ************************** Writing to output **************************
         **********************************************************************/

        *out++ = left_wave_samples + left_kick_sample + left_snare_sample
                 + left_hi_hat_sample;
        *out++ = right_wave_samples + right_kick_sample + right_snare_sample
                 + right_hi_hat_sample;
    }

    if (vibrato) {
        data->vibrato.repetitions_left--;
        if (data->vibrato.repetitions_left == 0) {
            vibrato = false;
            std::cout << "Vibrato ended\n";
        }
    }

    return paContinue;
}

uint8_t init_sound() {
    PaError err;

    // For random -> Karplus-Strong
    srand(time(nullptr));

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
        &stream_data
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
    if (index < MAX_KEYS) {
        stream_data.signals[index].gate = !stream_data.signals[index].gate;

        if (stream_data.signals[index].type == KS_e) {
            // Need to initialize buffer for Karplus-Strong
            initialize_ks(&stream_data.signals[index]);
        }
    } else {
        std::cout << "Trigger error: " << index << "is not a valid key!" << std::endl;
    }
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
    stream_data.vibrato.repetitions_left = MAX_VIBRATO_WINDOWS;
    std::cout << "Vibrato started\n";
}
