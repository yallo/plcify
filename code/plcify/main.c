//
//  main.c
//  PLCer
//
//  Created by Nimrod Astrahan on 11/25/15.
//  Copyright © 2015 Nimrod Astrahan. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sndfile.h"
#include "spandsp.h"
#include "plc.h"

// The size of a frame of audio. This is determined by the framesize in ms and the audio sampling frequency.
// 160 is 20ms * 8Khz. This is the size of the lost data we are trying to compensate.
static const int kFrameSize = 160;

/**
* Fills in missing frames in the passed in audio buffer according to PLC algorithm
* with data collected from the input file.
* @param sfinfo A Pointer to an open audio file containing the given audio stream.
* @param audio An audio stream which potentially contains holes in it to be fixed.
* @param channelIndex The index of the channel within the file on which the algorithm should run.
*/
void fillinMissingFrames(SF_INFO sfinfo, short *audio, int channelIndex);

// Controls operation of the program.
int main(int argc, const char *argv[]) {
    if (argc != 3) {
        printf("Usage plcer [input file] [output file]\n");
        exit(1);
    }

    SNDFILE *sndfile;
    SF_INFO sfinfo;
    SF_INFO outputInfo;
    sndfile = sf_open(argv[1], SFM_READ, &sfinfo);
    if (sndfile == 0) {
        printf("error");
        exit(1);
    }

    outputInfo.samplerate = sfinfo.samplerate;
    outputInfo.channels = sfinfo.channels;
    outputInfo.format = sfinfo.format;

    SNDFILE *outputFile;
    outputFile = sf_open(argv[2], SFM_WRITE, &outputInfo);
    if (outputFile == 0) {
        printf("Error creating output file\n");
        exit(1);
    }

    printf("input\n");
    printf("frames: %lld\n", sfinfo.frames);
    printf("samplerate: %d\n", sfinfo.samplerate);
    printf("channels: %d\n", sfinfo.channels);
    printf("format: %d\n", sfinfo.format);
    
    if (sfinfo.samplerate != 8000) {
        printf("Unsupported sample rate %d, plcify only supports 8Khz files. \n", sfinfo.samplerate);
        exit(1);
    }
    
    sf_count_t fileLength = sfinfo.channels * sfinfo.frames;
    short *audio = calloc(fileLength, sizeof(short));

    sf_read_short(sndfile, audio, fileLength);

    fillinMissingFrames(sfinfo, audio, 0);
    fillinMissingFrames(sfinfo, audio, 1);

    sf_count_t outputFrames = sf_write_short(outputFile, audio, fileLength);
    printf("Successfully written %d frames to output file. \n", outputFrames);
    sf_write_sync(outputFile);

    sf_close(sndfile);
    sf_close(outputFile);
    return 0;
}

// Fills in missing frames in the passed in audio buffer according to PLC algorithm
void fillinMissingFrames(SF_INFO sfinfo, short *audio, int channelIndex) {
    plc_state_t *context = plc_init(NULL);
    int zeroSamplesCounter = 0;
    sf_count_t sample_cnt = sfinfo.frames;
    short fillin_data[kFrameSize];
    const int kHistoryBuffers = 6;
    short history_buffer[kFrameSize * kHistoryBuffers];
    sf_count_t i;
    for (i = 0; i < sample_cnt; i++) {
        if (abs(audio[(i * sfinfo.channels) + channelIndex]) <= 1) {
            zeroSamplesCounter++;

            if (zeroSamplesCounter >= kFrameSize && i > (sizeof(history_buffer) + kFrameSize)) {
                printf("FOUND MISSING FRAME at index %lld \n", i);
                zeroSamplesCounter = 0;
                sf_count_t t, z;
                for (t = (kFrameSize * kHistoryBuffers) - 1, z = 0; t >= 0; t--, z++) {
                    history_buffer[t] = audio[((i - z - kFrameSize) * sfinfo.channels) + channelIndex];
                }

                plc_rx(context, history_buffer, (kFrameSize * kHistoryBuffers));

                int n = plc_fillin(context, fillin_data, kFrameSize);
                printf("Number of samples synthsized %d\n", n);
                sf_count_t j;
                for (j = 0; j < kFrameSize; j++) {
                    // writing back the fillin data to the beginning of the missing frame in the correct channel
                    sf_count_t indexInOriginalAudioStream = i - (kFrameSize - 1) + j;
                    audio[(indexInOriginalAudioStream * sfinfo.channels) + channelIndex] = fillin_data[j];
                }
            }
        } else {
            zeroSamplesCounter = 0;
        }
    }
}
