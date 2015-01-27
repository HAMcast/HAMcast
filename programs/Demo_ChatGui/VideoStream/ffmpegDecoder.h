#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <libavutil/avstring.h>

}

#include "core.h"
#include <iostream>
#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif
#include <stdio.h>
#include <math.h>
#include "ffmpegDecoder.h"
#include "DrawVideo.h"
#include "core.h"
//DrawVideo * doubleDD;
static DrawVideo * drawVideo;
static char* globalfilename;
static Core* core;

int run(char* filename,DrawVideo * qtpic,Core * c);
unsigned char * getData();

void stop();


#endif // FFMPEGDECODER_H
