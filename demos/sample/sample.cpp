#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 256

#define SAMPLE_PATH "kick.wav"

bool trigger;

struct Sample {
    float* data;       // Interleaved stereo or mono
    int length;        // In frames
    int channels;
    int sampleRate;
    volatile int position;      // Playback position
    volatile bool playing;
};

Sample sample;

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
    float *out = (float *)outputBuffer;

    for (unsigned int i = 0; i < framesPerBuffer; i++) {
        // Mix sample playback (kick) if active
        if (sample.playing && sample.position < sample.length) {
            *out++ = sample.data[sample.position * sample.channels];    // Left
            *out++ = sample.data[sample.position * sample.channels + 1];// Right

            // Advance sample positions
            sample.position++;
            if (sample.position >= sample.length) {
                sample.playing = false;
            }
        } else {
            *out++ = 0.0f;  // Left
            *out++ = 0.0f;  // Right
        }
    }

    return paContinue;
}

int main(void) {
    PaStream *stream;
    PaError err;

    sample.data = loadWavFile(SAMPLE_PATH,
                              &sample.length,
                              &sample.channels,
                              &sample.sampleRate);
    sample.position = 0;
    sample.playing = false;

    if (sample.sampleRate != SAMPLE_RATE) {
        fprintf(stderr, "Warning: sample rate mismatch (sample: %d, stream: %d)\n",
                sample.sampleRate, SAMPLE_RATE);
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
        NULL
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

    char key;
    bool running = true;
    trigger = false;
    while(running) {
        key = getchar();
        switch (key) {
            case 'q':
                running = false;
                break;

            case 's':
                trigger = true;
                printf("Trigger!\n");

                if (!sample.playing) {
                    sample.position = 0;
                    sample.playing = true;
                }

                trigger = false;
                break;

            default:
                break;
        }
    }

    delete[] sample.data;

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
