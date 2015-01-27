//
//  VideoViewViewController.m
//  hc_video_ios
//
//  Created by Sebastian Meiling on 27.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "VideoView.h"
#include "hamcast/hamcast.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <string>
#include "multicast_module.hpp"
#include "tcp_client_connection.hpp"

using hamcast::uri;

@interface VideoView ()

@end

@implementation VideoView

@synthesize group;
@synthesize video;
@synthesize group_str;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void) startDaemon
{
    int up = 1975;
    boost::asio::io_service io_service;
    std::string group = "ip://239.0.0.2:1234";
    std::string daemonid= "iPad";
    multicast_module mod(io_service,up,group,daemonid,0);
    mod.start_receive();
}

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
    running = true;


    
    [self performSelectorInBackground:@selector(receive) withObject:nil];
    [self performSelectorInBackground:@selector(playVideo) withObject:nil];
    [self performSelectorInBackground:@selector(startDaemon) withObject:nil];

}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.

}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)orientation
{
    return (orientation == UIDeviceOrientationLandscapeLeft) ||
    (orientation == UIDeviceOrientationLandscapeRight);
}

- (UIImage*)getDecodedFrame:(AVCodecContext*)codecCtx :(struct SwsContext*)convertCtx :(AVFrame*)srcFrame :(AVFrame*)dstFrame
{	
	sws_scale(convertCtx, (const uint8_t**)srcFrame->data, srcFrame->linesize, 0, codecCtx->height, dstFrame->data, dstFrame->linesize);
	
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, dstFrame->data[0], dstFrame->linesize[0]*codecCtx->height,kCFAllocatorNull);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImage = CGImageCreate(codecCtx->width, 
                                       codecCtx->height, 
                                       8, 
                                       24, 
                                       dstFrame->linesize[0], 
                                       colorSpace, 
                                       bitmapInfo, 
                                       provider, 
                                       NULL, 
                                       NO, 
                                       kCGRenderingIntentDefault);
    CGColorSpaceRelease(colorSpace);
    UIImage *image = [UIImage imageWithCGImage:cgImage];
    CGImageRelease(cgImage);
    CGDataProviderRelease(provider);
    CFRelease(data);
    
    return image;
}

- (void)decodeFrame:(AVCodecContext*)codecCtx :(struct SwsContext*)convertCtx :(AVFrame*)srcFrame :(AVFrame*)dstFrame
{	
	sws_scale(convertCtx, (const uint8_t**)srcFrame->data, srcFrame->linesize, 0, codecCtx->height, dstFrame->data, dstFrame->linesize);
	
    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, dstFrame->data[0], dstFrame->linesize[0]*codecCtx->height,kCFAllocatorNull);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImage = CGImageCreate(codecCtx->width, 
                                       codecCtx->height, 
                                       8, 
                                       24, 
                                       dstFrame->linesize[0], 
                                       colorSpace, 
                                       bitmapInfo, 
                                       provider, 
                                       NULL, 
                                       NO, 
                                       kCGRenderingIntentDefault);
    CGColorSpaceRelease(colorSpace);
    UIImage *image = [UIImage imageWithCGImage:cgImage];
    CGImageRelease(cgImage);
    CGDataProviderRelease(provider);
    CFRelease(data);
    frames.push(image);
}

- (void)playVideo
{
    while (running) {
        if (frames.size() > 37) {
            while (frames.size() > 12) {
                @autoreleasepool {
                    UIImage *image= frames.front();
                    [video performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:NO];
                    frames.pop();
                    [NSThread sleepForTimeInterval:0.045];
                }
            }
        }
        else {
            [NSThread sleepForTimeInterval:0.1];
        }
    }
}

//to be started as a thread
- (void)receive
{   
    std::cout << "Starte recv thread!" << std::endl;
//    group = uri ("ip://239.1.2.3:42001");
    group = uri([group_str UTF8String]);
    std::cout << "Join group " << group.str() << std::endl;
    rsock.join(group);
    
    /* LIBAV STUFF */
    avcodec_register_all();
    AVCodec *codec;
    AVCodecContext *ctx= NULL;

    size_t eds = 38;
    uint8_t *ed = new uint8_t[38];
    ed[0]=0x0;
    ed[1]=0x0;
    ed[2]=0x0;
    ed[3]=0x1;
    ed[4]=0x67;
    ed[5]=0x64;
    ed[6]=0x0;
    ed[7]=0x1e;
    ed[8]=0xac;
    ed[9]=0xd9;
    ed[10]=0x40;
    ed[11]=0xb4;
    ed[12]=0x12;
    ed[13]=0x6c;
    ed[14]=0x10;
    ed[15]=0x40;
    ed[16]=0x0;
    ed[17]=0x0;
    ed[18]=0x3;
    ed[19]=0x0;
    ed[20]=0x40;
    ed[21]=0x0;
    ed[22]=0x0;
    ed[23]=0xc;
    ed[24]=0xa3;
    ed[25]=0xc5;
    ed[26]=0x8b;
    ed[27]=0x65;
    ed[28]=0x80;
    ed[29]=0x0;
    ed[30]=0x0;
    ed[31]=0x0;
    ed[32]=0x1;
    ed[33]=0x68;
    ed[34]=0xeb;
    ed[35]=0xec;
    ed[36]=0xb2;
    ed[37]=0x2c;
    
    codec = avcodec_find_decoder(CODEC_ID_H264);
    if (!codec) {
        std::cerr << "Failed to find codec" << std::endl;
        return;
    }
    
    ctx = avcodec_alloc_context3(codec);
    if (ctx == NULL) {
        std::cerr << "Failed to alloc codec context!" << std::endl;
        return;
    }
    //ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ctx->width = 720;
    ctx->height = 576;
    ctx->time_base.num = 1;
    ctx->time_base.den = 50;
    ctx->profile=FF_PROFILE_H264_HIGH;
    ctx->pix_fmt = PIX_FMT_YUV420P;
    ctx->extradata = ed;
    ctx->extradata_size = eds;
    
    
    if(codec->capabilities&CODEC_CAP_TRUNCATED)
        ctx->flags|= CODEC_FLAG_TRUNCATED; /* we do not send complete frames */
    
    if (avcodec_open2(ctx, codec, NULL) < 0) {
        std::cerr << "Failed to open codec" << std::endl;
        return;
    }
    
    enum PixelFormat src_pix_fmt = PIX_FMT_YUV420P;
    enum PixelFormat dst_pix_fmt = PIX_FMT_RGB24;
    int got_picture, len;
    int w = ctx->width;
    int h = ctx->height;
    
    struct SwsContext *conv_ctx= sws_getContext(w, h, src_pix_fmt, w, h,
                                                dst_pix_fmt, SWS_BICUBIC,
                                                NULL, NULL, NULL);
    
    if (conv_ctx == NULL) {
        std::cerr << "Failed to get Convert Context!" << std::endl;
        return;
    }
    uint8_t *buffer; int numBytes;
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(dst_pix_fmt, ctx->width,ctx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
    AVPacket avpkt;
    AVFrame *frame_yuv = avcodec_alloc_frame();
    AVFrame *frame_rgb = avcodec_alloc_frame();
    if (frame_rgb == NULL) {
        std::cerr << "Failed to alloc frame_rgb!" << std::endl;
        return;
    }
    avpicture_fill((AVPicture *)frame_rgb, buffer, dst_pix_fmt, ctx->width, ctx->height);
    /* END LIBAV STUFF */
    hamcast::multicast_packet mp;
    av_init_packet(&avpkt);
    avpkt.pts = avpkt.dts = 0;
    avpkt.flags |= AV_PKT_FLAG_KEY; 
    while(running)
    {
        // receiving message from multicast socket
        mp = rsock.receive();
        avpkt.size = mp.size();
        avpkt.data = (uint8_t*) mp.data();
        if (avpkt.size > 0) {
            len = avcodec_decode_video2(ctx, frame_yuv, &got_picture, &avpkt);
            if (len < 0) {
                //std::cerr << "Failed to decode frame!" << std::endl;
                continue;
            }
            if (got_picture) {
                [self decodeFrame :ctx :conv_ctx :frame_yuv :frame_rgb];
                /*
                @autoreleasepool {
                    UIImage *image= [self getDecodedFrame :ctx :conv_ctx :frame_yuv :frame_rgb];
                    [video performSelectorOnMainThread:@selector(setImage:) withObject:image waitUntilDone:NO];
                }
                 */
            }
        }

    }
    av_free (frame_yuv);
    av_free (frame_rgb);
    av_free_packet(&avpkt);
    avcodec_close(ctx);
    av_free(ctx);
}

@end
