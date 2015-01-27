#include "ffmpegDecoder.h"
#include "DrawVideo.h"
#include <queue>

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0


#define FF_ALLOC_EVENT   (SDL_USEREVENT)
#define FF_REFRESH_EVENT (SDL_USEREVENT + 1)
#define FF_QUIT_EVENT (SDL_USEREVENT + 2)
#define VIDEO_PICTURE_QUEUE_SIZE 100
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_VIDEO_MASTER


std::queue<AVPicture> q;


typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;
typedef struct VideoPicture {
//    SDL_Overlay *bmp;
    int width, height; /* source height & width */
    int allocated;
    double pts;
} VideoPicture;
typedef struct VideoState {

    AVFormatContext *pFormatCtx;
    int             videoStream;

    int             av_sync_type;
    double          external_clock; /* external clock base */
    int64_t         external_clock_time;
    double          frame_timer;
    double          frame_last_pts;
    double          frame_last_delay;
    double          video_clock; ///<pts of last decoded frame / predicted pts of next decoded frame
    double          video_current_pts; ///<current displayed pts (different from video_clock if frame fifos are used)
    int64_t         video_current_pts_time;  ///<time (av_gettime) at which we updated video_current_pts - used to have running video pts
    AVStream        *video_st;
    PacketQueue     videoq;

    VideoPicture    pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int             pictq_size, pictq_rindex, pictq_windex;
    SDL_mutex       *pictq_mutex;
    SDL_cond        *pictq_cond;
    SDL_Thread      *parse_tid;
    SDL_Thread      *video_tid;
    char            filename[1024];
    int             quit;
} VideoState;
enum {
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_MASTER,
};
//SDL_Surface     *screen;

/* Since we only have one decoding thread, the Big Struct
   can be global in case we need it. */
VideoState *global_video_state;
AVPacket flush_pkt;

void packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}
int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    if(pkt != &flush_pkt && av_dup_packet(pkt) < 0) {
        return -1;
    }
    pkt1 = (AVPacketList*) av_malloc(sizeof(AVPacketList));
    if (!pkt1)
        return -1;
    pkt1->pkt = *pkt;
    pkt1->next = NULL;

    SDL_LockMutex(q->mutex);

    if (!q->last_pkt)
        q->first_pkt = pkt1;
    else
        q->last_pkt->next = pkt1;
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
    return 0;
}
static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for(;;) {

        if(global_video_state->quit) {
            ret = -1;
            break;
        }

        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}
static void packet_queue_flush(PacketQueue *q) {
    AVPacketList *pkt, *pkt1;

    SDL_LockMutex(q->mutex);
    for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        av_free_packet(&pkt->pkt);
        av_freep(&pkt);
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->size = 0;
    SDL_UnlockMutex(q->mutex);
}


double get_video_clock(VideoState *is) {
    double delta;

    delta = (av_gettime() - is->video_current_pts_time) / 1000000.0;
    return is->video_current_pts + delta;
}
double get_external_clock(VideoState *is) {
    return av_gettime() / 1000000.0;
}
double get_master_clock(VideoState *is) {
    if(is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
        return get_video_clock(is);
    } else {
        return get_external_clock(is);
    }
}

static Uint32 sdl_refresh_timer_cb(Uint32 interval, void *opaque) {
    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = opaque;
    SDL_PushEvent(&event);
    return 0; /* 0 means stop timer */
}
/* schedule a video refresh in 'delay' ms */
static void schedule_refresh(VideoState *is, int delay) {
    SDL_AddTimer(delay, sdl_refresh_timer_cb, is);
}

int send_thread(void *arg) {

//    for(;;){
//    SDL_LockMutex(sendq_mutex);
//    AVPicture pic;
//    pic =sendq.front();
//    sendq.pop();
//    SDL_UnlockMutex(sendq_mutex);
//    if(pic !=0)
//    core->doFile(pic);
//}

}

static unsigned char frame[1280*720*2];
void video_display(VideoState *is) {
//    SDL_Rect rect;
//    VideoPicture *vp;
//    AVPicture pict;
//    float aspect_ratio;
    int w, h;/*, x, y;*/
//    int i;

//    vp = &is->pictq[is->pictq_rindex];
//    if(vp->bmp) {
//        if(is->video_st->codec->sample_aspect_ratio.num == 0) {
//            aspect_ratio = 0;
//        } else {
//            aspect_ratio = av_q2d(is->video_st->codec->sample_aspect_ratio) *
//                           is->video_st->codec->width / is->video_st->codec->height;
//        }
//        if(aspect_ratio <= 0.0) {
//            aspect_ratio = (float)is->video_st->codec->width /
//                           (float)is->video_st->codec->height;
//        }
//        // apparently this assumption is bad
//        h = screen->h;
//        w = ((int)rint(h * aspect_ratio)) & -3;
//        if(w > screen->w) {
//            w = screen->w;
//            h = ((int)rint(w / aspect_ratio)) & -3;
//        }
//        x = (screen->w - w) / 2;
//        y = (screen->h - h) / 2;
//        rect.x = x;
//        rect.y = y;
//        rect.w = w;
//        rect.h = h;
/*
        pict.data[0] = vp->bmp->pixels[0];
        pict.data[1] = vp->bmp->pixels[2];
        pict.data[2] = vp->bmp->pixels[1];
*/
//        int line1 = vp->bmp->pitches[0];
//        int line2 = vp->bmp->pitches[2];
//        int line3 = vp->bmp->pitches[1];

//        w=vp->bmp->w;
//        h=vp->bmp->h;

        w=640;
        h=480;

//        memcpy(frame,vp->bmp->pixels[0],w*h);
//        memcpy(frame+w*h,vp->bmp->pixels[2],(w*h)/2);
//        memcpy(frame+ w*h+(w*h)/2,vp->bmp->pixels[1],(w*h)/2);
        AVPicture p = q.front();
        q.pop();

        unsigned char* yuv=frame;
        for(int i = 0; i < h; i++)
        {
            memcpy( yuv + i * w,p.data[0]+i*p.linesize[0], w); //Y

        }
        for(int i = 0; i < h/2; i++)
        {
                memcpy( yuv+w*h + i * w/2,  p.data[1]+i*p.linesize[1],w/2); //U
                memcpy(yuv+w*h*5/4 + i * w/2, p.data[2]+i*p.linesize[2],  w/2); //V
        }

            int w_=640;
            int h_=480;
//            core->out[RAW].SetData(frame,w_*h_*2);
//            core->EncodeFrame();

        //drawVideo->setMem(*vp->bmp->pixels);
//        drawVideo->update();
       // delete frame;
// avpicture_free(&p);
//        SDL_DisplayYUVOverlay(vp->bmp, &rect);
//    }

}

void video_refresh_timer(void *userdata) {

    VideoState *is = (VideoState *)userdata;
    VideoPicture *vp;
    double actual_delay, delay, sync_threshold, ref_clock, diff;

    if(is->video_st) {
        if(is->pictq_size == 0) {
            schedule_refresh(is, 1);
        } else {
            vp = &is->pictq[is->pictq_rindex];

            is->video_current_pts = vp->pts;
            is->video_current_pts_time = av_gettime();

            delay = vp->pts - is->frame_last_pts; /* the pts from last time */
            if(delay <= 0 || delay >= 1.0) {
                /* if incorrect delay, use previous one */
                delay = is->frame_last_delay;
            }
            /* save for next time */
            is->frame_last_delay = delay;
            is->frame_last_pts = vp->pts;

            /* update delay to sync to audio if not master source */
//            if(is->av_sync_type != AV_SYNC_VIDEO_MASTER) {
//                ref_clock = get_master_clock(is);
//                diff = vp->pts - ref_clock;

//                /* Skip or repeat the frame. Take delay into account
//           FFPlay still doesn't "know if this is the best guess." */
//                sync_threshold = (delay > AV_SYNC_THRESHOLD) ? delay : AV_SYNC_THRESHOLD;
//                if(fabs(diff) < AV_NOSYNC_THRESHOLD) {
//                    if(diff <= -sync_threshold) {
//                        delay = 0;
//                    } else if(diff >= sync_threshold) {
//                        delay = 2 * delay;
//                    }
//                }
//            }

            is->frame_timer += delay;
            /* computer the REAL delay */
            actual_delay = is->frame_timer - (av_gettime() / 1000000.0);
            if(actual_delay < 0.010) {
                /* Really it should skip the picture instead */
                actual_delay = 0.010;
            }

//            schedule_refresh(is, (int)(actual_delay * 1000 + 0.5));
            schedule_refresh(is,30);
            /* show the picture! */
            video_display(is);

            /* update queue for next picture! */
            if(++is->pictq_rindex == VIDEO_PICTURE_QUEUE_SIZE) {
                is->pictq_rindex = 0;
            }
            SDL_LockMutex(is->pictq_mutex);
            is->pictq_size--;
            SDL_CondSignal(is->pictq_cond);
            SDL_UnlockMutex(is->pictq_mutex);
        }
    } else {
        schedule_refresh(is, 100);
    }
}

void alloc_picture(void *userdata) {

    VideoState *is = (VideoState *)userdata;
    VideoPicture *vp;

    vp = &is->pictq[is->pictq_windex];
//    if(vp->bmp) {
//        // we already have one make another, bigger/smaller
//        SDL_FreeYUVOverlay(vp->bmp);
//    }
//    // Allocate a place to put our YUV image on that screen
//    vp->bmp = SDL_CreateYUVOverlay(is->video_st->codec->width,
//                                   is->video_st->codec->height,
//                                   SDL_YV12_OVERLAY,
//                                   screen);
//    vp->width = is->video_st->codec->width;
    vp->height = is->video_st->codec->height;

    SDL_LockMutex(is->pictq_mutex);
    vp->allocated = 1;
    SDL_CondSignal(is->pictq_cond);
    SDL_UnlockMutex(is->pictq_mutex);

}

int queue_picture(VideoState *is, AVFrame *pFrame, double pts) {

    VideoPicture *vp;
    int dst_pix_fmt;
    AVPicture pict;
    static struct SwsContext *img_convert_ctx;

    /* wait until we have space for a new pic */
    SDL_LockMutex(is->pictq_mutex);
    while(is->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE &&
          !is->quit) {
        SDL_CondWait(is->pictq_cond, is->pictq_mutex);
    }
    SDL_UnlockMutex(is->pictq_mutex);

    if(is->quit)
        return -1;

    // windex is set to 0 initially
    vp = &is->pictq[is->pictq_windex];

    /* allocate or resize the buffer! */
//    if(!vp->bmp ||
//       vp->width != is->video_st->codec->width ||
//       vp->height != is->video_st->codec->height) {
        SDL_Event event;

        vp->allocated = 0;
        /* we have to do it in the main thread */
        event.type = FF_ALLOC_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);

        /* wait until we have a picture allocated */
        SDL_LockMutex(is->pictq_mutex);
        while(!vp->allocated && !is->quit) {
            SDL_CondWait(is->pictq_cond, is->pictq_mutex);
        }
        SDL_UnlockMutex(is->pictq_mutex);
        if(is->quit) {
            return -1;
        }
//    }
    /* We have a place to put our picture on the queue */
    /* If we are skipping a frame, do we set this to null
     but still return vp->allocated = 1? */


//    if(vp->bmp) {

//        SDL_LockYUVOverlay(vp->bmp);

        dst_pix_fmt = PIX_FMT_YUV420P;
        /* point pict at the queue */

//        pict.data[0] = vp->bmp->pixels[0];
//        pict.data[1] = vp->bmp->pixels[2];
//        pict.data[2] = vp->bmp->pixels[1];

//        pict.linesize[0] = vp->bmp->pitches[0];
//        pict.linesize[1] = vp->bmp->pitches[2];
//        pict.linesize[2] = vp->bmp->pitches[1];

        // Convert the image into YUV format that SDL uses
        if(img_convert_ctx == NULL) {
            int w = is->video_st->codec->width;
            int h = is->video_st->codec->height;
            img_convert_ctx = sws_getContext(w, h,
                                             //is->video_st->codec->pix_fmt, w, h,
                                             is->video_st->codec->pix_fmt, 640,480,
                                             (PixelFormat)dst_pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
            if(img_convert_ctx == NULL) {
                fprintf(stderr, "Cannot initialize the conversion context!\n");
                exit(1);
            }
        }
        AVPicture pico;
        avpicture_alloc(&pico,(PixelFormat)dst_pix_fmt,640,480);



        sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,
                  0, is->video_st->codec->height, pico.data, pico.linesize);

//        SDL_UnlockYUVOverlay(vp->bmp);
        vp->pts = pts;

        /* now we inform our display thread that we have a pic ready */
        if(++is->pictq_windex == VIDEO_PICTURE_QUEUE_SIZE) {
            is->pictq_windex = 0;
        }
        q.push(pico);
//        avpicture_free(&pico);
        SDL_LockMutex(is->pictq_mutex);
        is->pictq_size++;
        SDL_UnlockMutex(is->pictq_mutex);
//    }
    return 0;
}

double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {

    double frame_delay;

    if(pts != 0) {
        /* if we have pts, set video clock to it */
        is->video_clock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        pts = is->video_clock;
    }
    /* update the video clock */
    frame_delay = av_q2d(is->video_st->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;
    return pts;
}

uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;

/* These are called whenever we allocate a frame
 * buffer. We use this to store the global_pts in
 * a frame at the time it is allocated.
 */
int our_get_buffer(struct AVCodecContext *c, AVFrame *pic) {
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t *)av_malloc(sizeof(uint64_t));
    *pts = global_video_pkt_pts;
    pic->opaque = pts;
    return ret;
}
void our_release_buffer(struct AVCodecContext *c, AVFrame *pic) {
    if(pic) av_freep(&pic->opaque);
    avcodec_default_release_buffer(c, pic);
}

int video_thread(void *arg) {
    VideoState *is = (VideoState *)arg;
    AVPacket pkt1, *packet = &pkt1;
    int len1, frameFinished;
    AVFrame *pFrame;
    double pts;

    pFrame = avcodec_alloc_frame();

    for(;;) {
        if(packet_queue_get(&is->videoq, packet, 1) < 0) {
            // means we quit getting packets
            break;
        }
        if(packet->data == flush_pkt.data) {
            avcodec_flush_buffers(is->video_st->codec);
            continue;
        }
        pts = 0;

        // Save global pts to be stored in pFrame
        global_video_pkt_pts = packet->pts;
        // Decode video frame
        len1 = avcodec_decode_video(is->video_st->codec, pFrame, &frameFinished,
                                    packet->data, packet->size);
        if(packet->dts == AV_NOPTS_VALUE
           && pFrame->opaque && *(uint64_t*)pFrame->opaque != AV_NOPTS_VALUE) {
            pts = *(uint64_t *)pFrame->opaque;
        } else if(packet->dts != AV_NOPTS_VALUE) {
            pts = packet->dts;
        } else {
            pts = 0;
        }
        pts *= av_q2d(is->video_st->time_base);



        // Did we get a video frame?
        if(frameFinished) {
            pts = synchronize_video(is, pFrame, pts);
//            std::cout << "pts !!! " << pts << std::endl;
            if(queue_picture(is, pFrame, pts) < 0) {
                break;
            }
        }
        av_free_packet(packet);
    }
    av_free(pFrame);
    return 0;
}



int stream_component_open(VideoState *is, int stream_index) {

    AVFormatContext *pFormatCtx = is->pFormatCtx;
    AVCodecContext *codecCtx;
    AVCodec *codec;


    if(stream_index < 0 || stream_index >= pFormatCtx->nb_streams) {
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    codecCtx = pFormatCtx->streams[stream_index]->codec;

    codec = avcodec_find_decoder(codecCtx->codec_id);
    if(!codec || (avcodec_open(codecCtx, codec) < 0)) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }
    is->videoStream = stream_index;
    is->video_st = pFormatCtx->streams[stream_index];

    is->frame_timer = (double)av_gettime() / 1000000.0;
    is->frame_last_delay = 40e-3;
    is->video_current_pts_time = av_gettime();

    packet_queue_init(&is->videoq);
    is->video_tid = SDL_CreateThread(video_thread, is);
    SDL_CreateThread(send_thread,NULL);
    codecCtx->get_buffer = our_get_buffer;
    codecCtx->release_buffer = our_release_buffer;
}

int decode_interrupt_cb(void) {
    return (global_video_state && global_video_state->quit);
}

int decode_thread(void *arg) {

    VideoState *is = (VideoState *)arg;
    AVFormatContext *pFormatCtx;
    AVPacket pkt1, *packet = &pkt1;

    int video_index = -1;

    int i;

    is->videoStream=-1;

    global_video_state = is;
    // will interrupt blocking functions if we quit!
    url_set_interrupt_cb(decode_interrupt_cb);

    // Open video file
    if(av_open_input_file(&pFormatCtx, is->filename, NULL, 0, NULL)!=0)
        return -1; // Couldn't open file

    is->pFormatCtx = pFormatCtx;

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, is->filename, 0);


    // Find the first video stream

    for(i=0; i<pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO &&
           video_index < 0) {
            video_index=i;
        }
    }

    if(video_index >= 0) {
        stream_component_open(is, video_index);
    }

    if(is->videoStream < 0 ) {
        fprintf(stderr, "%s: could not open codecs\n", is->filename);
        goto fail;
    }

    // main decode loop


    for(;;) {
        if(is->quit) {
            break;
        }


        if(av_read_frame(is->pFormatCtx, packet) < 0) {
            ByteIOContext * ctx = pFormatCtx->pb;

            if(url_ferror(ctx) == 0) {
                SDL_Delay(100); /* no error; wait for user input */
                continue;
            }
            else {
                break;
            }
        }

        // Is this a packet from the video stream?
        if(packet->stream_index == is->videoStream) {
            //std::cout<<"packetqueue!"<<std::endl;
            int64_t dts = packet->dts;
            dts *= av_q2d(is->video_st->time_base);
//            std::cout << dts << "  !!!!decode dts " << std::endl;
            packet_queue_put(&is->videoq, packet);
//            core->out[CODEC].SetData(packet->data,packet->size);
//            core->RTP_Pack();
//            core->Send();
            core->in[CODEC].SetData(packet->data,packet->size);
            core->DecodeFrame();

        } else {
            av_free_packet(packet);
        }
    }
    /* all done - wait for it */
    while(!is->quit) {
        SDL_Delay(100);
    }

    fail:
    {
        SDL_Event event;
        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    return 0;
}





int startDecode(void* arg) {

    SDL_Event       event;
    double          pos;
    VideoState      *is;

//    drawVideo->nextAnimationFrame();

//    doubleDD = qtpic;
    is = (VideoState*)av_mallocz(sizeof(VideoState));


    // Register all formats and codecs
    av_register_all();

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    // Make a screen to put our video

//    screen = SDL_SetVideoMode(1024, 768, 0, 0);

//    if(!screen) {
//        fprintf(stderr, "SDL: could not set video mode - exiting\n");
//        exit(1);
//    }

    //av_strlcpy(is->filename, sizeof(is->filename), argv[1]);
    char * test = globalfilename;//"/home/zagaria/test5.mp4ss";
    memcpy(is->filename,test,sizeof(is->filename));
    is->pictq_mutex = SDL_CreateMutex();
    is->pictq_cond = SDL_CreateCond();

    schedule_refresh(is, 40);

    is->av_sync_type = DEFAULT_AV_SYNC_TYPE;
    is->parse_tid = SDL_CreateThread(decode_thread, is);

    if(!is->parse_tid) {
        av_free(is);
        return -1;
    }


    av_init_packet(&flush_pkt);
    flush_pkt.data = (uint8_t *)"FLUSH";

    for(;;) {
   SDL_WaitEvent(&event);
   switch(event.type) {
    case FF_QUIT_EVENT:
    case SDL_QUIT:
            is->quit = 1;
            SDL_Quit();
            exit(0);
            break;
    case FF_ALLOC_EVENT:
            alloc_picture(event.user.data1);
            break;
    case FF_REFRESH_EVENT:
            video_refresh_timer(event.user.data1);
            break;
    default:
            break;
        }
    }
    return 0;
}



int run(char* filename,DrawVideo * qtpic,Core* c){
    drawVideo = qtpic;
    globalfilename= filename;
    core = c;
    SDL_CreateThread(startDecode,NULL);



}

void stop(){
    SDL_Event event;
    event.type = FF_QUIT_EVENT;
    SDL_PushEvent(&event);
}


