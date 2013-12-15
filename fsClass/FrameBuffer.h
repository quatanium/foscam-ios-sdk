//
//  FrameBuffer.h
//  fsIPC4
//
//  Created by foscam on 12-11-30.
//  Copyright 2012 foscam. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "FSApi.h"

typedef struct tagFrame
{
	char		magic[4];
	int     	type;
	long long	pts;
	int			length;
	bool		isKey;
	char		data[0];
} ATTRIBUTE_PACKED StreamFrame;

@interface FrameBuffer : NSObject {
	char	*frameBuf;
	int		bufferSize;
	int		writelen;
	int		tFrame;
	MUTEX	mutex;
    int     bError;
}

- (void)createBuf:(int)size;
- (void)releaseBuf;
- (void)writeFrame:(int)type :(char*)frame :(int)len :(long long)pts :(bool)isKey;
- (BOOL)readFrame:(int*)type :(char*)frame :(int*)len :(long long *)pts;
@end
