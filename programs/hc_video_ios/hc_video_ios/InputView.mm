//
//  ViewController.m
//  hc_video_ios
//
//  Created by Sebastian Meiling on 27.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "InputView.h"
#import "VideoView.h"
#include "hamcast/hamcast.hpp"

@interface InputView ()

@end

@implementation InputView
@synthesize group_uri_input;

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    [group_uri_input becomeFirstResponder]; // get keys
}

- (void)viewDidUnload
{
    [self setGroup_uri_input:nil];
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    VideoView *next = (VideoView*) [segue destinationViewController];
    next.group_str = [group_uri_input text];
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField{
    NSString *group_str = [group_uri_input text];
    hamcast::uri group_uri ([group_str UTF8String]);
    if (!group_uri.empty()) {
        [self performSegueWithIdentifier:@"switch_to_video" sender:self];
    }
    else 
    {
        NSLog(@"not a valid group");
    }
    return YES;
}

-(BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)orientation
{
    return (orientation == UIDeviceOrientationLandscapeLeft) ||
    (orientation == UIDeviceOrientationLandscapeRight);
}

@end
