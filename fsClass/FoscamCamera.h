//
//  FoscamCamera.h
//  foscam-ios-sdk
//
//  Created by yu dezhi on 12/11/13.
//  Copyright (c) 2013 Quatanium Co., Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FSApi.h"

@protocol FoscamDelegate
- (void)loggedIn;
- (void)loginFailed:(int)reason;
- (void)videoOpened;
- (void)videoFailedToOpen;
@end

@interface FoscamCamera : NSObject
@property (weak   , nonatomic) id <FoscamDelegate>  delegate;
@property (strong , nonatomic) NSString            *user_IP;
@property (strong , nonatomic) NSString            *user_Name;
@property (strong , nonatomic) NSString            *user_Password;
@property (assign , nonatomic) int                  user_Port;
@property (strong , nonatomic) UIImageView         *firstimageShowView;
- (id)initWithImageView:(UIImageView *)imageView connectIP:(NSString *)ip connectPort:(int)port forAdmin:(NSString *)admin withPass:(NSString *)pass autoStart:(BOOL)startNow;
- (void)startPlay;
- (void)stopPlay;
- (void)moveUp;
- (void)moveDown;
- (void)moveLeft;
- (void)moveRight;
@end


