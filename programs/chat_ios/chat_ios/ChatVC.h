//
//  ChatVC.h
//  chat_ios
//
//  Created by Raphael Hiesgen on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "hamcast/hamcast.hpp"

@interface ChatVC : UIViewController {
    
    IBOutlet UITextField *TextFieldChatInput;
    IBOutlet UITextView  *TextViewChat;
    
    NSString *nick;
    NSString *group;
    
    hamcast::uri                u;
    hamcast::multicast_socket   sck;
    volatile BOOL               running;
    hamcast::multicast_packet   message;
    
    UIBackgroundTaskIdentifier  bgTask;
}

@property (nonatomic, retain) NSString *nick;
@property (nonatomic, retain) NSString *group;

@property (nonatomic, assign) hamcast::uri              u;
@property (nonatomic, assign) hamcast::multicast_packet message;
@property (nonatomic, assign) UIBackgroundTaskIdentifier bgTask;

@end
