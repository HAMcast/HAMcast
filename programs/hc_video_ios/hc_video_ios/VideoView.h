//
//  VideoViewViewController.h
//  hc_video_ios
//
//  Created by Sebastian Meiling on 27.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <UIKit/UIImageView.h>
#import "hamcast/uri.hpp"
#import "hamcast/multicast_socket.hpp"

#include <queue>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/pixfmt.h"
}

@interface VideoView : UIViewController
{
    hamcast::uri                group;
    hamcast::multicast_socket   rsock;
    std::queue<UIImage*>        frames;
    volatile BOOL               running;
    UIImageView*                video;
    
    NSString*                   group_str;
}

- (void)receive;
- (void)playVideo;
- (UIImage*)getDecodedFrame:(AVCodecContext*)codecCtx :(struct SwsContext*)convertCtx :(AVFrame*)srcFrame :(AVFrame*)dstFrame;

@property (nonatomic, assign) hamcast::uri  group;
@property (nonatomic, retain) IBOutlet UIImageView* video; 
@property (nonatomic, retain) NSString *group_str;

@end
