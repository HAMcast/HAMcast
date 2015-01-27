//
//  ChatVC.m
//  chat_ios
//
//  Created by Raphael Hiesgen on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "ChatVC.h"

@interface ChatVC ()

@end

@implementation ChatVC

@synthesize u;
@synthesize nick;
@synthesize group;
@synthesize message;
@synthesize bgTask;

//to be started as a thread
- (void)receive
{
    
    while(running)
    {
        // receiving message from multicast socket
        message = sck.receive();
        
        if(!message.empty() && strcmp(reinterpret_cast<const char*>(message.data()), "") != 0)
        {
            // get NSString from c_str
            const char *in_char_p = reinterpret_cast<const char*>(message.data());
            NSString *incoming = [[NSString alloc]initWithBytes:in_char_p length:message.size() encoding:NSUTF8StringEncoding];
            
            // update TextViewChat
            if(incoming != nil) {
                NSLog(@"received \"%@\"", incoming);
                [self performSelectorOnMainThread:@selector(updateChat:) withObject:incoming waitUntilDone:YES];
            } else {
                NSLog(@"could not get NSString from multicast_packet");
            }
        } else {
            NSLog(@"received empty mcpacket");
        }
    }
}

-(void)updateChat:(NSString *)text
{
    @synchronized(TextViewChat)
    {
        TextViewChat.text = [TextViewChat.text stringByAppendingString:[@"\n" stringByAppendingString:text]];
        
//        if ([TextViewChat.text length] > 500) {
//            TextViewChat.text = TextViewChat.text 
//        }
        
        [TextViewChat scrollRangeToVisible:NSMakeRange([TextViewChat.text length], 0)];
    }
}

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
    
    //this creates a border around the TextViewChat
    [TextViewChat drawRect:[TextViewChat frame]];
    TextViewChat.layer.cornerRadius = 5;
    TextViewChat.clipsToBounds = YES;
    [TextViewChat.layer setBorderWidth:1];
    [TextViewChat.layer setBorderColor:[[UIColor lightGrayColor] CGColor]];
    
    [TextFieldChatInput becomeFirstResponder]; // get keys
    [self updateChat:[[nick stringByAppendingString:@" -> "] stringByAppendingString:group]];
    [self updateChat:@"------------"];
    
    u = hamcast::uri([group UTF8String]);
    
    NSString *login = [@">> " stringByAppendingString:nick];
    sck.send(u, [login length], [login UTF8String]);
    
    sck.join(u);
    
    running = true;
    
    [self performSelectorInBackground:@selector(receive) withObject:nil];
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

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    
    if (![[[TextFieldChatInput text] stringByReplacingOccurrencesOfString:@" " withString:@""] isEqualToString:@""] && [[TextFieldChatInput text] length] <= 160)
    {
        NSString *input = [nick stringByAppendingString:[@": " stringByAppendingString:[TextFieldChatInput text]]];
        
        
        NSLog(@"text in UITextField: \"%@\"", [TextFieldChatInput text]);
        NSLog(@"sending: \"%@\"", input);
        
        sck.send(u, [input length], [input UTF8String]);
        
        TextFieldChatInput.text=@"";
        
        return YES;
    }
    else
    {
        return NO;
    }
    
}

-(void) viewWillDisappear:(BOOL)animated {
    if ([self.navigationController.viewControllers indexOfObject:self]==NSNotFound) {
        sck.leave(u);
        //        NSString *logout = [@"<< " stringByAppendingString:nick];
        //        sck.send(u, [logout length], [logout UTF8String]);
        running = false;
    }
    [super viewWillDisappear:animated];
}

//- (void)applicationWillResignActive:(UIApplication *)application
//{
//    NSLog(@"cv: applicationWillResignActive");
//}
//
//- (void)applicationWillTerminate:(UIApplication *)application
//{
//    NSLog(@"cv: applicationWillTerminate");
//}
//
//- (void)applicationDidEnterBackground:(UIApplication *)application
//{
//    
//    exit(1);
//    
//    std::cout << "blub -> leabving to  background" << std::cout;
//    
////    bgTask = [application beginBackgroundTaskWithExpirationHandler:^{
////        NSLog(@"CV: applicationDidEnterBackground: beginBackgroundTaskWithExpirationHandler");
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
////        NSLog(@"CV: applicationDidEnterBackground: dispatch async");
////        
////        [application endBackgroundTask:bgTask];
////        bgTask = UIBackgroundTaskInvalid;
////    });
//}
//
//- (void)applicationDidBecomeActive:(UIApplication *)application
//{
//    NSLog(@"cv: applicationDidBecomeActive");
//}
//
//+ (void)applicationWillResignActive:(UIApplication *)application
//{
//    NSLog(@"cv: applicationWillResignActive");
//}

@end
