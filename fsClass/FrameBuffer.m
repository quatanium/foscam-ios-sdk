//
//  FrameBuffer.m
//  fsIPC4
//
//  Created by foscam on 12-11-30.
//  Copyright 2012 foscam. All rights reserved.
//

#import "FrameBuffer.h"
#define maxvbuf 300000

@implementation FrameBuffer

- (id)init
{
	self = [super init];
	if (self) {
		frameBuf = NULL;
		tFrame = 0;
		INIT_MUTEX(&mutex);
	}
	return self;
}

- (void)dealloc
{
	RELEASE_MUTEX(&mutex);
	if (frameBuf) {
		free(frameBuf);
		frameBuf = NULL;
	}
}

- (void)createBuf:(int)size
{
	assert(size > 0);
	bufferSize = size;
	if (frameBuf) {
		free(frameBuf);
	}
    bError = false;
	tFrame = 0;
	writelen = 0;
	frameBuf = malloc(size);
}

- (void)releaseBuf
{
	if (frameBuf) {
		free(frameBuf);
		frameBuf = NULL;
	}
}

- (void)writeFrame:(int)type :(char *)frame :(int)len :(long long)pts :(bool)isKey;
{
    if (bError) {
        if (!isKey) {
            return;
        }
        bError = false;
    }
	StreamFrame ff;
	memcpy(ff.magic, "FSAV", 4);
	ff.length = len;
	ff.pts = pts;
	ff.type = type;
	ff.isKey = isKey;
	int s = sizeof(StreamFrame);
	LOCK_MUTEX(&mutex);
	if (writelen + len + s <= bufferSize) {//空间足够
		memcpy(frameBuf + writelen, (char *)&ff, s);
		memcpy(frameBuf + writelen + s, frame, len);
		writelen += (len + s);
		tFrame ++;
		UNLOCK_MUTEX(&mutex);
		return;
	} else {
		if (isKey) {//当前是关键帧，丢掉所有帧，填充当前帧
			goto writeTofirst;
		} else {
			//丢掉所有帧，剩最后一个关键帧
			char *lastIFrame = NULL;
			int	parseLen = 0;
			for (int i = 0; i < tFrame; i++) {
				StreamFrame *f= (StreamFrame *)frameBuf;
				if (strncmp(f->magic, "FSAV", 4) == 0 && parseLen + f->length + s <= writelen) {
					parseLen += f->length + s;
					if (f->isKey) {
						lastIFrame = (char *)&f;
					}
				}else {//帧序出错，丢掉所有帧
                    writelen = 0;
                    tFrame = 0;
                    UNLOCK_MUTEX(&mutex);
                    bError = true;
                    return;
				}
			}
			
			if (lastIFrame) {
				StreamFrame * f= (StreamFrame *)lastIFrame;
				memmove(frameBuf, lastIFrame, f->length+s);
				writelen = f->length + s;
				
				memcpy(frameBuf + writelen, (char *)&ff, s);
				memcpy(frameBuf + writelen + s, frame, len);
				writelen += (s + len);
				tFrame = 2;
			}else {//没有找到关键帧，丢掉所有帧
				writelen = 0;
				tFrame = 0;
                bError = true;
			}
			UNLOCK_MUTEX(&mutex);
			return;
		}
		UNLOCK_MUTEX(&mutex);
		return;
	}
	
writeTofirst:
	memcpy(frameBuf, (char *)&ff, s);
	memcpy(frameBuf + s, frame, len);
	writelen = (len + s);
	tFrame = 1;
	UNLOCK_MUTEX(&mutex);
}

- (BOOL)readFrame:(int*)type :(char*)frame :(int*)len :(long long *)pts;
{
	assert(frame && type && len && pts);
	int s = sizeof(StreamFrame);
	LOCK_MUTEX(&mutex);
	if (writelen > s && (writelen - s) < 300000 ) {
		StreamFrame *f = (StreamFrame *)frameBuf;
		if (strncmp(f->magic, "FSAV", 4) == 0)
		{
			if (f->length + s <= writelen) {
				*len = f->length;
				*pts = f->pts;
				*type = f->type;
                
				writelen -= (f->length + s);
                                
                memcpy(frame, f->data, f->length);
                memmove(frameBuf, f->data + f->length, writelen);
                
                tFrame--;
                UNLOCK_MUTEX(&mutex);
                return YES;
			}
		} else {//帧序出错，丢掉所有帧
			writelen = 0;
			tFrame = 0;
		}
	}
	UNLOCK_MUTEX(&mutex);
	return NO;
}

@end
