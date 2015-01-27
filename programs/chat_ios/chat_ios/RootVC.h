//
//  RootVC.h
//  chat_ios
//
//  Created by Raphael Hiesgen on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ChatVC.h"
#import "ConfigVC.h"
#import "hamcast/hamcast.hpp"

@interface RootVC : UIViewController {
    
    IBOutlet UITextField *TextFieldNick;
    IBOutlet UITextField *TextFieldGroup;
    
    NSString *nick;
    NSString *group;
 
    UIBackgroundTaskIdentifier  bgTask;
}

@property (nonatomic, retain) NSString *nick;
@property (nonatomic, retain) NSString *group;

@property (nonatomic, assign) UIBackgroundTaskIdentifier  bgTask;

@end
