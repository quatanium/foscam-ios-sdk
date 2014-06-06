//
//  main.m
//  foscam-ios-sdk
//
//  Created by Eric Chen on 6/6/14.
//  Copyright (c) 2014 Quatanium Co., Ltd. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#if !(TARGET_IPHONE_SIMULATOR)
#import "FSApi.h"
#endif

int main(int argc, char *argv[])
{
    @autoreleasepool {
#if !(TARGET_IPHONE_SIMULATOR)
        Init();
#endif
        int retVal = UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
#if !(TARGET_IPHONE_SIMULATOR)
        Uninit();
#endif
        return retVal;
    }
}
