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

static const int kFrameSize = 160;

void fillinMissingFrames(SF_INFO sfinfo, short *audio, int channelIndex);

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

            if (zeroSamplesCounter >= (kFrameSize - 60) && i > (sizeof(history_buffer) + kFrameSize)) {
                if (zeroSamplesCounter < kFrameSize) {
                    i += (kFrameSize - zeroSamplesCounter + 30) / 2;
                }

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
