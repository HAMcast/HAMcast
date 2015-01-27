//
//  VideoViewViewController.h
//  hc_video_ios
//
//  Created by Sebastian Meiling on 27.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <UIKit/UIImageView.h>

#include <queue>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
}

@interface VideoView : UIViewController
{
    std::queue<UIImage*>        frames;
    volatile BOOL               running;
    UIImageView*                video;
    int                         m_if_index;
    std::string                 m_if_name;
    
    NSString*                   group_str;
}

- (int) get_ifindex;
- (void)receive;
- (void)playVideo;
- (UIImage*)getDecodedFrame:(AVCodecContext*)codecCtx :(struct SwsContext*)convertCtx :(AVFrame*)srcFrame :(AVFrame*)dstFrame;
- (int) mcast_join:(int)sockfd :(const struct sockaddr*)grp :(socklen_t)grplen;
- (int) mcast_leave:(int)sockfd :(const struct sockaddr*)grp :(socklen_t)grplen;


@property (nonatomic, retain) IBOutlet UIImageView* video;
@property (nonatomic, retain) NSString *group_str;


@end
