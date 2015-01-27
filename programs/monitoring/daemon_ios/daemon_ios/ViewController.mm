//
//  ViewController.m
//  daemon_ios
//
//  Created by Sebastian Meiling on 25.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ViewController.h"
#include <boost/asio/io_service.hpp>
#include <boost/thread.hpp>
#include <string>
#include "multicast_module.hpp"
#include "tcp_client_connection.hpp"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    int up = 1975;
    boost::asio::io_service io_service;
    std::string group = "ip://239.0.0.2:1234";
    std::string daemonid= "inetPad1";
    
    multicast_module mod(io_service,up,group,daemonid,0);
    mod.start_receive();
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

@end
