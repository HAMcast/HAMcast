//
//  ConfigVC.m
//  chat_ios
//
//  Created by Raphael Hiesgen on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ConfigVC.h"

@interface ConfigVC ()

@end

@implementation ConfigVC

@synthesize ip;
@synthesize port;
@synthesize on;

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
    
    [TextFieldIp becomeFirstResponder]; // get keys
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad) {
        return (interfaceOrientation != UIInterfaceOrientationPortrait);
    }
    
    else {
        return (interfaceOrientation == UIInterfaceOrientationPortrait);
    }
}

-(void) viewWillDisappear:(BOOL)animated {
    if ([self.navigationController.viewControllers indexOfObject:self]==NSNotFound) {
        
        // back button was pressed.  We know this is true because self is no longer
        // in the navigation stack.

        ip = TextFieldIp.text;
        port = TextFieldPort.text;
        if( TunnelISwitch.on ) {
            on = @"enabled";
        } else {
            on = @"disabled";
        }
        
        if(![ip isEqualToString:@""] || ![port isEqualToString:@""]) {
            NSLog(@"saving ip (%@) and port (%@), tunnel is:(%@)", ip, port, on);
        } else {
            NSLog(@"fields may be empty");
        }
        
        // saves values to file
        
    }
    [super viewWillDisappear:animated];
}

@end
