/******************************************************************************\
 *  _   ___     ____  __               _                                      *
 * | | | \ \___/ /  \/  | ___ __ _ ___| |_                                    *
 * | |_| |\     /| |\/| |/ __/ _` / __| __|                                   *
 * |  _  | \ - / | |  | | (_| (_| \__ \ |_                                    *
 * |_| |_|  \_/  |_|  |_|\___\__,_|___/\__|                                   *
 *                                                                            *
 * This file is part of the HAMcast project.                                  *
 *                                                                            *
 * HAMcast is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU Lesser General Public License as published   *
 * by the Free Software Foundation, either version 3 of the License, or       *
 * (at your option) any later version.                                        *
 *                                                                            *
 * HAMcast is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                       *
 * See the GNU Lesser General Public License for more details.                *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with HAMcast. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                            *
 * Contact: HAMcast support <hamcast-support@informatik.haw-hamburg.de>       *
\******************************************************************************/

/* standard headers */
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>

#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/types.h>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}
/* HAMcast headers */
#include "hamcast/hamcast.hpp"
#include "hamcast/ipc.hpp"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::set;

using boost::bad_lexical_cast;
using boost::lexical_cast;
using boost::thread;

//using namespace hamcast;

namespace {
    /* struct to parse long opts */
    struct option long_options[] = {
        {"group",       required_argument,  0,  'g'},
        {"tunnel",      required_argument,  0,  't'},
        {"interface",   required_argument,  0,  'i'},
        {"mode",        required_argument,  0,  'm'},
        {"help",        no_argument,        0,  'h'},
        {0, 0, 0, 0}
    };

    int sockfd;
    struct sockaddr_in dst;
    struct sockaddr_in tun;
    AVCodecContext *pCodecCtx;
    uint8_t *ed = NULL;
    int eds = 0;
    int frame = 0;
    char* group = "ip://239.1.2.3";
    uint16_t glen = strlen (group);
    size_t tunhdr= (4*sizeof(uint16_t)) + glen;
    uint16_t grplen = htons(glen);
    uint16_t tmp_type = 1;
    uint16_t m_type = htons(tmp_type);
    uint16_t m_port = htons(42001);

}

void usage (const std::string& program)
{
        cout << endl;
        cout << "USAGE" << endl;
        cout << "\t" << program << " [-h] [-g GROUP] [-i IFNAME] [-m MODE] FILE" << endl;
        cout << endl;
}

void help (const std::string& program)
{
    usage(program);
    cout << "OPTION" << endl;
    cout << "\t-h, --help" << endl;
    cout << "\t\tPrint this help screen." << endl;
    cout << "\t-g, --group GROUP" << endl;
    cout << "\t\tSet multicast GROUP." << endl;
    cout << "\t-i, --interface IFNAME" << endl;
    cout << "\t\tSet interface." << endl;
    cout << "NOTE" << endl;
    cout << "\tIt is mandatory to specify a video file!" << endl;
    cout << endl;
    cout << "AUTHOR" << endl;
    cout << "\tSebastian Meiling <sebastian.meiling (at) haw-hamburg.de" << endl;
    cout << endl;
}


void recv_packet(AVPacket* packet) {

    cout << "d1" << endl;
    AVCodec *codec=avcodec_find_decoder(CODEC_ID_H264);
    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    ctx->flags2 |= CODEC_FLAG2_FAST;
    ctx->pix_fmt = PIX_FMT_YUV420P;
    ctx->width = 1024;
    ctx->height = 576;
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        cerr << "failed to open codec!" << endl;
        return;
    }
    AVFrame *frame = avcodec_alloc_frame();
    avcodec_get_frame_defaults(frame);
    cout << "d5" << endl;
    int got_picture;
    int retval = avcodec_decode_video2(ctx, frame, &got_picture, packet);
    cout << "d6" << endl;
    if (got_picture && retval > 0) {
        cout << "decoded frame!" << endl;
    }
}

void recv_frame (const uint8_t* data, const size_t size)
{
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int got_picture, len;
    FILE *f;
    AVFrame *picture;
    uint8_t inbuf[size + FF_INPUT_BUFFER_PADDING_SIZE];
    AVPacket avpkt;

    size_t extra_size = 40;
    uint8_t *extra_data = new uint8_t[40];
    extra_data[1]=0x0;
    extra_data[2]=0x0;
    extra_data[3]=0x1;
    extra_data[4]=0x67;
    extra_data[5]=0x64;
    extra_data[6]=0x0;
    extra_data[7]=0x1f;
    extra_data[8]=0xac;
    extra_data[9]=0xd9;
    extra_data[10]=0x40;
    extra_data[11]=0x40;
    extra_data[12]=0x4;
    extra_data[13]=0x9b;
    extra_data[14]=0x1;
    extra_data[15]=0x10;
    extra_data[16]=0x0;
    extra_data[17]=0x0;
    extra_data[18]=0x3;
    extra_data[19]=0x0;
    extra_data[20]=0x10;
    extra_data[21]=0x0;
    extra_data[22]=0x0;
    extra_data[23]=0x3;
    extra_data[24]=0x3;
    extra_data[25]=0x8;
    extra_data[26]=0xf1;
    extra_data[27]=0x83;
    extra_data[28]=0x19;
    extra_data[29]=0x60;
    extra_data[30]=0x0;
    extra_data[31]=0x0;
    extra_data[32]=0x0;
    extra_data[33]=0x1;
    extra_data[34]=0x68;
    extra_data[35]=0xeb;
    extra_data[36]=0xec;
    extra_data[37]=0xb2;
    extra_data[38]=0x2c;
    extra_data[39]=0x0;

    avcodec_register_all();
    av_init_packet(&avpkt);

    /* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
    memset(inbuf + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    printf("Video decoding\n");

    /* find the mpeg1 video decoder */
    codec = avcodec_find_decoder(CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    //c->codec_type = AVMEDIA_TYPE_VIDEO;
    c->width = 1024;
    c->height = 576;
    c->time_base.num = 1;
    c->time_base.den = 48;
    c->profile=FF_PROFILE_H264_HIGH;
    c->pix_fmt = PIX_FMT_YUV420P;
    c->extradata = extra_data;
    c->extradata_size = extra_size;
    picture= avcodec_alloc_frame();

    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        c->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */

    if (avcodec_open2(c, codec, NULL) < 0) {
        cerr << "failed to open codec" << endl;
        return;
    }

    /* the codec gives us the frame size, in samples */
    avpkt.size = size;
    memcpy(inbuf,data,size);
    avpkt.data = inbuf;
    avpkt.pts = avpkt.dts = 0;
    avpkt.flags |= AV_PKT_FLAG_KEY;
    while (avpkt.size > 0) {
        len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
        if (len < 0) {
            fprintf(stderr, "Error while decoding frame %d\n", frame);
            break;
        }
        if (got_picture) {
            cout << "saving frame " << frame << endl;
            frame++;
        }
    }
    avcodec_close(c);
    av_free(c);
    av_free(picture);
}

void send_frame(const uint8_t* data, const size_t size) {
    cout << "Send frame data, size: " << size << endl;

    if (size < 64000) {
    uint16_t buflen1 = size + FF_INPUT_BUFFER_PADDING_SIZE;
    uint8_t buf1[buflen1];
    memcpy(buf1,data,size);
    memset(buf1 + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
    if (sendto (sockfd,buf1,buflen1,0,(struct sockaddr*)&dst,sizeof(dst))<0)
        std::cerr << " - sendto group failed" << std::endl;

    uint16_t buflen2 = tunhdr + size + FF_INPUT_BUFFER_PADDING_SIZE;
    uint8_t buf2[buflen2];
    uint16_t tmplen = htons(buflen2);
    memcpy (buf2, &m_type, sizeof(m_type));
    memcpy (buf2+(1*sizeof(uint16_t)), &tmplen,sizeof(uint16_t));
    memcpy (buf2+(2*sizeof(uint16_t)), &m_port, sizeof(m_port));
    memcpy (buf2+(3*sizeof(uint16_t)), &grplen, sizeof(grplen));
    memcpy (buf2+(4*sizeof(uint16_t)), group, glen);
    memcpy (buf2+tunhdr,data,size);
    memset (buf2+tunhdr+size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    if (sendto (sockfd,buf2,buflen2,0,(struct sockaddr*)&tun,sizeof(tun))<0)
        std::cerr << " - sendto tunnel failed" << std::endl;
    }
    else {
        cerr << " - frame too big" << endl;
    }
    boost::this_thread::sleep( boost::posix_time::milliseconds(40) );
}

void stream_loop (string vfile)
{
    cout << "1" << endl;
    av_register_all();
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    // Open video file
    if(avformat_open_input(&pFormatCtx, vfile.c_str(), NULL, NULL)!=0) {
        cerr << "Failed to open video file!" << endl;
        return; // Couldn't open file
    }
    int64_t duration = pFormatCtx->duration;
    cout << "Duration: " << duration << endl;
    cout << "2" << endl;
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return; // Couldn't find stream information
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, vfile.c_str(), 0);
    int i;
    //AVCodecContext *pCodecCtx;


    // Find the first video stream
    int videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    }
    if(videoStream==-1) {
        cerr << "No video stream found!" << endl;
        return; // Didn't find a video stream
    }
    cout << "3" << endl;
    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
    AVCodec *pCodec;
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        cerr << "Unsupported codec!" << endl;
        return; // Codec not found
    }
    cout << "4" << endl;
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
        cerr << "Failed to open codec!" << endl;
        return; // Could not open codec
    }
    cout << "5" << endl;
    AVFrame *pFrame, *pFrameRGB;
    // Allocate video frame
    pFrame=avcodec_alloc_frame();
    // Allocate an AVFrame structure
    pFrameRGB=avcodec_alloc_frame();
    if(pFrameRGB==NULL) {
        cerr << "Failed to allocate AVFrame!" << endl;
        return;
    }
    cout << "6" << endl;
    uint8_t *buffer;
    int numBytes;
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                                pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
                    pCodecCtx->width, pCodecCtx->height);

    int frameFinished;
    AVPacket packet;
    cout << "7" << endl;
    i=0;
    cout << "Ctx, BR: " << pCodecCtx->bit_rate <<
            ", FPS: " << pCodecCtx->time_base.den <<
            ", TB: " << pCodecCtx->time_base.num << endl;
    ed = pCodecCtx->extradata;
    eds = pCodecCtx->extradata_size;
    cout << "extradata, size: " << eds << "data: ";
    for (int i=0; i<eds; ++i) {
        //printf ("%02x", ((unsigned char*)ed)[i]);
        cout << "ed[" << std::dec << i << "]=" << "0x" << std::hex << (int) ed[i] << std::dec << ";" << endl;
    }
    cout << std::dec << endl;
    sleep(1);
    while(av_read_frame(pFormatCtx, &packet)>=0) {
        if(packet.stream_index==videoStream) {
            send_frame(packet.data, packet.size);
        }
        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }
    cout << endl;
    cout << "100" << endl;
}


int main (int argc, char** argv)
{
    string program (argv[0]);
    string group;
    string tunnel;
    string interface;

    /* parse options */
    while (true) {
        int option_index = 0;
        int c = getopt_long (argc, argv, "hg:i:m:t:",
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        unsigned long ul;

        switch (c) {
        case 0: {
            if (long_options[option_index].flag != 0)
                break;
            cout << "option " << long_options[option_index].name;
            if (optarg)
                cout << " with arg " << optarg << endl;
            break;
        }
        case 'g': {
            group = string (optarg);
            if (group.empty()) {
                cerr << "Invalid multicast group!" << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 't': {
            tunnel = string (optarg);
            if (tunnel.empty()) {
                cerr << "Invalid tunnel endpoint!" << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        case 'i': {
            interface = string(optarg);
            if (interface.empty()) {
                cerr << "Invalid interface name!" << endl;
                return (EXIT_FAILURE);
            }
            break;
        }
        default: {
            usage (program);
            return (EXIT_FAILURE);
            break;
        }
    }
    }
    if (group.empty ()) {
        inet_pton (AF_INET, "239.1.2.3", &(dst.sin_addr));
    }
    else {
        inet_pton (AF_INET, group.c_str (), &(dst.sin_addr));
    }
    if (tunnel.empty ()) {
        inet_pton (AF_INET, "192.168.1.20", &(tun.sin_addr));
    }
    else {
        inet_pton (AF_INET, tunnel.c_str (), &(tun.sin_addr));
    }
    dst.sin_family = AF_INET;
    tun.sin_family = AF_INET;
    dst.sin_port = htons(42001);
    tun.sin_port = htons(1607);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (argc < 2) {
        usage (program);
        return (EXIT_FAILURE);
    }

    string videofile (argv[argc-1]);
    boost::thread t_stream;
/*
    if (!group.empty()) {
        t_stream = thread(stream_loop, videofile);
    }
*/
    int j=0;
    while (j<10) {
        cout << "Video Loop: " << ++j << endl;
        stream_loop(videofile);
    }
    cout << "++++++++++++++++++++++++" << endl;
    cout << endl;
    t_stream.join();
    return (EXIT_SUCCESS);
}
