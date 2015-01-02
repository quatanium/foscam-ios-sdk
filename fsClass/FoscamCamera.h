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
- (void)videoOpenedForWidth:(int)width andHeight:(int)height;
@end

@interface FoscamCamera : NSObject
@property (weak,   nonatomic) id <FoscamDelegate>  delegate;
@property (strong, nonatomic) NSString            *user_IP;
@property (strong, nonatomic) NSString            *user_Name;
@property (strong, nonatomic) NSString            *user_Password;
@property         (nonatomic) int                  user_Port;
@property (strong, nonatomic) UIImageView         *firstImageShowView;
- (void)startPlay;
- (void)stopPlay;
- (void)moveUp;
- (void)moveDown;
- (void)moveLeft;
- (void)moveRight;
@end


