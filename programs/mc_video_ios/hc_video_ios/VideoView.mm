//
//  VideoViewViewController.m
//  hc_video_ios
//
//  Created by Sebastian Meiling on 27.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "VideoView.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <string>
#include "hamcast/hamcast.hpp"

#define MAX_RECV_BUFFER 1024*100

@interface VideoView ()

@end

@implementation VideoView

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

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view.
    running = true;
    m_if_index = 0;
    
    [self performSelectorInBackground:@selector(receive) withObject:nil];
    [self performSelectorInBackground:@selector(playVideo) withObject:nil];
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

//int mcast_join (int sockfd, const struct sockaddr *grp, socklen_t grplen)
- (int) mcast_join:(int)sockfd :(const struct sockaddr*)grp :(socklen_t)grplen
{
//#ifdef FUCK_YOU
#ifdef MCAST_JOIN_GROUP
    struct group_req req;
    
    if (m_if_index > 0) {
        std::cout << "m_if_index > 0" << std::endl;
        req.gr_interface = m_if_index;
    } else if (!m_if_name.empty()) {
        std::cout << "m_if_name not empty" << std::endl; 
        if ( (req.gr_interface = if_nametoindex (m_if_name.c_str())) == 0 ) {
            errno = ENXIO;
            return (-1);
        }
    } else {
        //req.gr_interface = 0;
        req.gr_interface = [self get_ifindex];
        //std::cout << "ifindex: " << req.gr_interface << std::endl;
    }
    
    if (grplen > sizeof (req.gr_group)) {
        errno = EINVAL;
        std::cout << "grplen to big" << std::endl;
        return (-1);
    }
    //std::copy (grp, grp + sizeof(struct sockaddr), &req.gr_group);
    memcpy (&req.gr_group, grp, grplen);
    
    int family = -1;
    switch (grp->sa_family) {
        case AF_INET:
            family = IPPROTO_IP;
            break;
        case AF_INET6:
            family = IPPROTO_IPV6;
            break;
        default:
            family = -1;
//            throw ipm_address_exception ("IPaddr family not supported!");
            std::cout << "IPaddr family not supported!" << std::endl;
            return -1;
    }
    //Group: 50463215, len: 16
    std::cout << "socket: " << sockfd << " proto: " << family << " opt: " << MCAST_JOIN_GROUP << std::endl;
    return (setsockopt (sockfd, family, MCAST_JOIN_GROUP, &req, sizeof(req)));
#else
    struct ip_mreq mreq;
    struct ifreq ifreq;
    
    memcpy (&mreq.imr_multiaddr,
            &((const sockaddr_in *)grp)->sin_addr,
            sizeof (struct in_addr));
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    std::cout << "socket: " << sockfd << " proto: " << IPPROTO_IP << " opt: " << IP_ADD_MEMBERSHIP << std::endl;
    return (setsockopt (sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                        &mreq, sizeof (mreq)));
#endif
}

//int mcast_leave (int sockfd, const struct sockaddr *grp, socklen_t grplen)
- (int) mcast_leave:(int)sockfd :(const struct sockaddr*)grp :(socklen_t)grplen
{
    struct group_req req;
    if (m_if_index > 0) {
        req.gr_interface = m_if_index;
    } else if (!m_if_name.empty()) {
        if ( (req.gr_interface = if_nametoindex (m_if_name.c_str())) == 0 ) {
            errno = ENXIO;
            return (-1);
        }
    } else {
        req.gr_interface = [self get_ifindex];
    }
    if (grplen > sizeof (req.gr_group)) {
        errno = EINVAL;
        std::cout << "grplen to big" << std::endl;
        return (-1);
    }
    memcpy (&req.gr_group, grp, grplen);
    
    int family = -1;
    switch (grp->sa_family) {
        case AF_INET:
            family = IPPROTO_IP;
            break;
        case AF_INET6:
            family = IPPROTO_IPV6;
            break;
        default:
            family = -1;
            //            throw ipm_address_exception ("IPaddr family not supported!");
            std::cout << "IPaddr family not supported!" << std::endl;
            return -1;
    }
    
    return (setsockopt (sockfd, family, MCAST_LEAVE_GROUP, &req, sizeof(req)));
}


- (int) get_ifindex
{
    struct ifaddrs* ifaddrs;
    if (getifaddrs(&ifaddrs) == -1)
    {
        std::cerr << "getifaddr failed" << std::endl;
    }
    else
    {
        std::string iface_ip;
        std::string iface_name;
        // FIXME: this is crap, but we need it, thus it's useful
        for (struct ifaddrs* ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (ifa ->ifa_addr->sa_family == AF_INET) // check wheter it's IPv4
            {
                char ip_addr[NI_MAXHOST];
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            ip_addr, sizeof(ip_addr), NULL, 0, NI_NUMERICHOST);
                struct sockaddr_storage* iface_addr =
                ((struct sockaddr_storage *)ifa->ifa_addr);
                iface_ip = ip_addr;
                iface_name = ifa->ifa_name;
                unsigned int iface_index = if_nametoindex(iface_name.c_str());
                if (   iface_addr != NULL
                    && iface_ip.find("127.0.0.1") == std::string::npos
                    && iface_ip.find("127.0.1.1") == std::string::npos)
                {
                    // we've found a valid IP interface
                    if (ifaddrs != NULL) freeifaddrs(ifaddrs);
                    return iface_index;
                }
            }
        }
    }
    return 0;
}

//to be started as a thread
- (void)receive
{   
    std::cout << "Starte recv thread!" << std::endl;
    
    int                     m_sockfd;
    int                     rv;
    int                     recvdbytes;
    
    struct addrinfo         hints;
    struct addrinfo         *myinfo;
    struct addrinfo         *p;
    struct sockaddr_storage their_addr;
    
    socklen_t               their_addrlen;
    
    uint8_t                 buf[MAX_RECV_BUFFER];
    
    hamcast::uri group_uri([group_str UTF8String]);
//    NSArray                 *info  = [group_str componentsSeparatedByString:@":"];
//    NSString                *addr  = [[info objectAtIndex:1] stringByReplacingOccurrencesOfString:@"//" withString:@""];
//    NSString                *port  = [info objectAtIndex:2];
    std::string                  addr = group_uri.host();
    std::string                  port = group_uri.port();
    
    std::cout << "Join group " << addr << ":" << port << std::endl;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_DGRAM;
    hints.ai_flags      = AI_PASSIVE;
    
    if ((rv = getaddrinfo(addr.c_str(), port.c_str(), &hints, &myinfo)) != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return;
    }
    
    for(p = myinfo; p!= NULL; p=p->ai_next)
    {
        std::cout << "Open socket ..." << std::endl;
        if ((m_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            std::cerr << "listener: socket" << std::endl;
            continue;
        }
        std::cout << "Set reuse addr ..." << std::endl;
        int one = 1;
        if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) == -1)
        {
            std::cerr << "Failed set reuseaddr!" << std::endl;
        }
        std::cout << "Join group: " << addr.c_str() << ":" << port.c_str() << std::endl;
        if ([self mcast_join:m_sockfd :p->ai_addr :p->ai_addrlen] == -1)
        {
            perror("try joining");
            return;
        }
        std::cout << "Bind socket ..." << std::endl;
        if (bind(m_sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(m_sockfd);
            std::cerr << "listener: bind" << std::endl;
            continue;
        }
        memcpy(&their_addr, p->ai_addr, p->ai_addrlen);
        their_addrlen = p->ai_addrlen;
        break;
    }
    
    if (myinfo == NULL)
    {
        std::cerr << "listener: failed to bind socket" << std::endl;
        return;
    }
    freeaddrinfo(myinfo);
    
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
    av_init_packet(&avpkt);
    avpkt.pts = avpkt.dts = 0;
    avpkt.flags |= AV_PKT_FLAG_KEY; 
    while(running) {
        if ((recvdbytes = recv(m_sockfd, buf, MAX_RECV_BUFFER-1 , 0)) == -1) {
            std::cerr << "error receiving" << std::endl;
            perror("lalala");
            return;
        }
        avpkt.size = recvdbytes;
        avpkt.data = buf;
        buf[recvdbytes] = '\0';
        
        if (avpkt.size > 0) {
            len = avcodec_decode_video2(ctx, frame_yuv, &got_picture, &avpkt);
            if (len < 0) {
                //std::cerr << "Failed to decode frame!" << std::endl;
                continue;
            }
            if (got_picture) {
                [self decodeFrame :ctx :conv_ctx :frame_yuv :frame_rgb];
            }
        }

    }

    [self mcast_leave:m_sockfd :((struct sockaddr*)&their_addr) :(their_addrlen)];
    av_free (frame_yuv);
    av_free (frame_rgb);
    av_free_packet(&avpkt);
    avcodec_close(ctx);
    av_free(ctx);
}

@end
