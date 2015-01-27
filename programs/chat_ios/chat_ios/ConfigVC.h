//
//  ConfigVC.h
//  chat_ios
//
//  Created by Raphael Hiesgen on 22.06.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface ConfigVC : UIViewController {
    
    IBOutlet UITextField *TextFieldIp;
    IBOutlet UITextField *TextFieldPort;
    IBOutlet UISwitch    *TunnelISwitch;
    
    NSString *ip;
    NSString *port;
    NSString *on;
    
}

@property (nonatomic, retain) NSString *ip;
@property (nonatomic, retain) NSString *port;
@property (nonatomic, retain) NSString *on;


@end
