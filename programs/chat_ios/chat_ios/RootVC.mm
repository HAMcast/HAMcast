//
//  RootVC.m
//  chat_ios
//
//  Created by Raphael Hiesgen on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "RootVC.h"

@interface RootVC ()

@end

@implementation RootVC

@synthesize group;
@synthesize nick;
@synthesize bgTask;

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
    [TextFieldNick becomeFirstResponder]; // get keys
//    [UIApplication sharedApplication].idleTimerDisabled = YES; // prevent standby
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

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if([[segue identifier] isEqualToString:@"ChatSegue"]){
        NSLog(@"segue to chat");
        ChatVC *chat = (ChatVC *) [segue destinationViewController];
        
        UIBarButtonItem *newBackButton = [[UIBarButtonItem alloc] initWithTitle: @"Disconnect" style: UIBarButtonItemStyleBordered target: nil action: nil];
//        NSString *btnLocalized = NSLocalizedString(@"HOLA", nil);
//        NSLog(@"localized btn is named: %@", btnLocalized);
//        UIBarButtonItem *newBackButton = [[UIBarButtonItem alloc] initWithTitle:btnLocalized style: UIBarButtonItemStyleBordered target: nil action: nil];
        [[self navigationItem] setBackBarButtonItem: newBackButton];
        
        chat.nick  = nick;
        chat.group = group;
        
    }
//    else if ([[segue identifier] isEqualToString:@"ConfigSegue"]) {
//        NSLog(@"segue to config");
////        ConfigurationViewController *cvc = (ConfigurationViewController *) [segue destinationViewController];
//
//          UIBarButtonItem *newBackButton = [[UIBarButtonItem alloc] initWithTitle: @"Back" style: UIBarButtonItemStyleBordered target: nil action: nil];
////        NSString *btnLocalized = NSLocalizedString(@"BACK", nil);
////        UIBarButtonItem *newBackButton = [[UIBarButtonItem alloc] initWithTitle:btnLocalized style: UIBarButtonItemStyleBordered target: nil action: nil];
//        
//        [[self navigationItem] setBackBarButtonItem: newBackButton];
//        
//        // set variables in new view ...
//        
//    }
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField{
    NSLog(@"vc:textFieldShouldReturn");
    group = [TextFieldGroup text];
    nick  = [TextFieldNick text];
    
    hamcast::uri u([group UTF8String]);
    if(u.host_is_ipv4addr()) {
        NSLog(@"found ipv4 host: \"%@\"",group);
    }
    
    if(u.host_is_ipv6addr()) {
        NSLog(@"found ipv6 host: \"%@\"",group);
    }
    
    if(nick != nil && ![nick isEqualToString:@""]) {
        NSLog(@"found valid nick: \"%@\"", nick);
    }
    
    if(nick != nil && ![nick isEqualToString:@""] && (u.host_is_ipv4addr() || u.host_is_ipv6addr())) {
        NSLog(@"trying to perform segue");
        [self performSegueWithIdentifier:@"ChatSegue" sender:self];
    }
    //    [textField resignFirstResponder]; // this would hide keys
    return YES;
}

//- (void)applicationWillResignActive:(UIApplication *)application
//{
//    NSLog(@"rv: applicationWillResignActive");
//}
//
//- (void)applicationWillTerminate:(UIApplication *)application
//{
//    NSLog(@"rv: applicationWillTerminate");
//}
//
//- (void)applicationDidEnterBackground:(UIApplication *)application
//{
//    exit(1);
//    
//    std::cout << "blub -> leabving to  background" << std::cout;
////    bgTask = [application beginBackgroundTaskWithExpirationHandler:^{
////        NSLog(@"rv: applicationDidEnterBackground: beginBackgroundTaskWithExpirationHandler");
////        // Clean up any unfinished task business by marking where you.
////        // stopped or ending the task outright.
////        [application endBackgroundTask:bgTask];
////        bgTask = UIBackgroundTaskInvalid;
////    }];
////    
////    // Start the long-running task and return immediately.
////    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
////        
////        // Do the work associated with the task, preferably in chunks.
////        NSLog(@"rv: applicationDidEnterBackground: dispatch async");
////        
////        [application endBackgroundTask:bgTask];
////        bgTask = UIBackgroundTaskInvalid;
////    });
//}
//
//- (void)applicationDidBecomeActive:(UIApplication *)application
//{
//    NSLog(@"rv: applicationDidBecomeActive");
//}
//
//+ (void)applicationWillResignActive:(UIApplication *)application
//{
//    NSLog(@"rv: applicationWillResignActive");
//}

@end
