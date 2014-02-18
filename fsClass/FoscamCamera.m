//
//  FoscamCamera.m
//  homer
//
//  Created by yu dezhi on 12/11/13.
//  Copyright (c) 2013 Quatanium Inc. All rights reserved.
//

#import "FoscamCamera.h"
#import "avdecodersdk.h"

typedef struct AVPicture {
    uint8_t *data[4];
    int linesize[4];       // number of bytes per line
} AVPicture;

#define DEFAULTCHID     0  // default channels id
#define IS_HORIZONTAL   ([UIDevice currentDevice].orientation==UIDeviceOrientationLandscapeLeft || [UIDevice currentDevice].orientation==UIDeviceOrientationLandscapeRight)
#define BUFFER_SIZE     1280 * 720 * 3 // 24-bit

char m_frameBuf[BUFFER_SIZE];

@interface FoscamCamera()
@property (assign , nonatomic) HFSTHREAD     m_hThread;
@property (assign , nonatomic) HFSTHREAD     m_hMessageThread;
@property (assign , nonatomic) BOOL		     m_ThreadRun;
@property (assign , nonatomic) BOOL          m_MessageThreadRun;
@property (assign , nonatomic) NETPARAM	     m_CtrlParam;
@property (assign , nonatomic) int           m_iChid;
@property (assign , nonatomic) int           m_iDeviceType;
@property (assign , nonatomic) MUTEX         m_liveViewMutex;
@property (assign , nonatomic) MUTEX         m_hImageMutex;
@property (assign , nonatomic) MUTEX         m_hUpdateViewMutex;
@property (assign , nonatomic) HAVDECODER	 m_fsDecode;
@property (assign , nonatomic) FRAME_P		 m_ViewFrame;
@property (assign , nonatomic) BOOL          m_bPtzMoveLeft;
@property (assign , nonatomic) BOOL          m_bPtzMoveRight;
@property (assign , nonatomic) BOOL          m_bPtzMoveUp;
@property (assign , nonatomic) BOOL          m_bPtzMoveDown;
@property (assign , nonatomic) BOOL          m_bConnectedsuc;
@end

@implementation FoscamCamera

@synthesize delegate = _delegate;
@synthesize user_IP = _user_IP;
@synthesize user_Port = _user_Port;
@synthesize user_Name = _user_Name;
@synthesize user_Password = _user_Password;
@synthesize m_hThread = _m_hThread;
@synthesize m_hMessageThread = _m_hMessageThread;
@synthesize m_ThreadRun = _m_ThreadRun;
@synthesize m_MessageThreadRun = _m_MessageThreadRun;
@synthesize m_CtrlParam = _m_CtrlParam;
@synthesize m_iChid = _m_iChid;
@synthesize m_iDeviceType = _m_iDeviceType;
@synthesize m_liveViewMutex = _m_liveViewMutex;
@synthesize m_hImageMutex = _m_hImageMutex;
@synthesize m_hUpdateViewMutex = _m_hUpdateViewMutex;
@synthesize m_fsDecode = _m_fsDecode;
@synthesize m_ViewFrame = _m_ViewFrame;
@synthesize m_bPtzMoveLeft = _m_bPtzMoveLeft;
@synthesize m_bPtzMoveRight = _m_bPtzMoveRight;
@synthesize m_bPtzMoveUp = _m_bPtzMoveUp;
@synthesize m_bPtzMoveDown = _m_bPtzMoveDown;
@synthesize m_bConnectedsuc = _m_bConnectedsuc;

- (id)init
{
    self = [super init];
    if (self) {
        [self initParameters];
    }
    return self;
}

- (id)initWithImageView:(UIImageView *)imageView connectIP:(NSString *)ip connectPort:(int)port forAdmin:(NSString *)admin withPass:(NSString *)pass autoStart:(BOOL)startNow
{
    self = [self init];
    if (self)
    {
        self.firstimageShowView = imageView;
        self.user_IP            = ip;
        self.user_Port          = port;
        self.user_Name          = admin;
        self.user_Password      = pass;
        if (startNow)
            [self startPlay];
    }
    return self;
}

- (void)initParameters
{
    INIT_MUTEX(&_m_liveViewMutex);
    INIT_MUTEX(&_m_hImageMutex);
    INIT_MUTEX(&_m_hUpdateViewMutex);
    _m_bConnectedsuc = NO;
    memset(&_m_CtrlParam, 0x0, sizeof(NETPARAM));
}

- (void)dealloc
{
    _m_fsDecode = NULL;
    
    _m_ThreadRun = NO;
    _m_hThread = NULL;
    _m_MessageThreadRun = NO;
    _m_hMessageThread = NULL;
    
    free(&_m_CtrlParam);
    memset(&_m_CtrlParam, 0x0, sizeof(NETPARAM));
    
    RELEASE_MUTEX(&_m_liveViewMutex);
    RELEASE_MUTEX(&_m_hImageMutex);
    RELEASE_MUTEX(&_m_hUpdateViewMutex);
}

HFSTHREAD fs_createThread(THREAD_FUNC_TO, void *);

- (void)startPlay
{
    _m_iDeviceType = 1;  //0 - MJ device, 1 - H264 device
    strncpy(_m_CtrlParam.ip, self.user_IP.UTF8String, 127);
    strncpy(_m_CtrlParam.username, self.user_Name.UTF8String, 20);
    strncpy(_m_CtrlParam.passwd, self.user_Password.UTF8String, 20);
    _m_CtrlParam.streamType = 1; //0-sub stream  1-main stream
    _m_CtrlParam.port = self.user_Port;
    _m_CtrlParam.webPort = self.user_Port;
    char m_Uid[64];
    bzero(m_Uid, 64);
    _m_iChid = DEFAULTCHID;
    
    _m_ThreadRun = YES;
    _m_MessageThreadRun = YES;
    if (NULL == _m_hMessageThread) {
        _m_hMessageThread = (HFSTHREAD)fs_createThread((THREAD_FUNC_TO)&MessageThread, (__bridge void *)self);
    }
    
    if (_m_iDeviceType == 1) {
        [self initDecoder:DEFAULTCHID];
        
        usrLogIn(_m_iDeviceType, _m_CtrlParam.ip, _m_CtrlParam.username, _m_CtrlParam.passwd, _m_CtrlParam.streamType, _m_CtrlParam.port, _m_CtrlParam.webPort, m_Uid, _m_iChid);
    } else if (_m_iDeviceType == 0) {
        usrLogIn(_m_iDeviceType, _m_CtrlParam.ip, _m_CtrlParam.username, _m_CtrlParam.passwd, 0/*m_CtrlParam.streamType*/, _m_CtrlParam.webPort, _m_CtrlParam.port, m_Uid, _m_iChid);
    }
}

- (void)stopPlay
{
    if (_m_bConnectedsuc) {
        _m_bConnectedsuc = NO;
        // Should stop threads but would crash if we do, WTF FOSCAM?!

        stopVideoStream(DEFAULTCHID);
        usrLogOut(DEFAULTCHID);
    } else {
        usrLogOut(DEFAULTCHID);
    }
}

- (void)moveUp
{
    if (!_m_bConnectedsuc) {
        return;
    }
    
    if (!_m_bPtzMoveUp) {
        _m_bPtzMoveUp = YES;
        ptzMoveUp(DEFAULTCHID);
    } else {
        _m_bPtzMoveUp = NO;
        ptzStopRun(DEFAULTCHID);
    }
}

- (void)moveDown
{
    if (!_m_bConnectedsuc) {
        return;
    }
    
    if (!_m_bPtzMoveDown) {
        _m_bPtzMoveDown = YES;
        ptzMoveDown(DEFAULTCHID);
    } else {
        _m_bPtzMoveDown = NO;
        ptzStopRun(DEFAULTCHID);
    }
}

- (void)moveLeft
{
    if (!_m_bConnectedsuc) {
        return;
    }
    
    if (!_m_bPtzMoveLeft) {
        _m_bPtzMoveLeft = YES;
        ptzMoveLeft(DEFAULTCHID);
    } else {
        _m_bPtzMoveLeft = NO;
        ptzStopRun(DEFAULTCHID);
    }
}

- (void)moveRight
{
    if (!_m_bConnectedsuc) {
        return;
    }
    
    if (!_m_bPtzMoveRight) {
        _m_bPtzMoveRight = YES;
        ptzMoveRight(DEFAULTCHID);
    } else {
        _m_bPtzMoveRight = NO;
        ptzStopRun(DEFAULTCHID);
    }
}

- (void)pauseMove
{
}

#pragma mark  - FOSCAM_DATA

- (void)setViewFrame:(FRAME_P)frame viewChid:(int)chid
{
    switch (chid) {
        case DEFAULTCHID:
            memcpy(&_m_ViewFrame, &frame, sizeof(frame));
            break;
            
        default:
            break;
    }
}

void fsi_decodeCall2(int msg, void *param, void *userdata)
{
    int child = DEFAULTCHID;
	FoscamCamera *play = (__bridge FoscamCamera *)userdata;
    [play setViewFrame:*(FRAME_P*)param viewChid:child];
}

- (void)initDecoder:(int)chid
{
    if (chid == DEFAULTCHID) {
        if (!_m_fsDecode) {
            _m_fsDecode = FS_Create_Decoder();
            FS_Register_Decoder(_m_fsDecode, DCD_H264, &fsi_decodeCall2, (__bridge void *)self);
        }
    } else {
        return;
    }
}

- (void)uninitDecoder:(int)chid
{
    if (chid == DEFAULTCHID) {
        if (_m_fsDecode) {
            FS_UnRegister_Decoder(_m_fsDecode, DCD_H264);
            FS_Release_Decoder(_m_fsDecode);
            _m_fsDecode = nil;
        }
    } else {
        return;
    }
}

void MessageThread(void *lParam)
{
    FoscamCamera* obj = (__bridge FoscamCamera *)lParam;
    [obj MessageThreadWork];
}

- (void)MessageThreadWork
{
    int nStatusId = 0;
    while (_m_MessageThreadRun) {
        nStatusId = getStatusId(DEFAULTCHID);
        if (nStatusId < 0) {
            usleep(50);
        } else {
            switch (nStatusId) {
                case FS_API_STATUS_LOGIN_SUCCESS:
                {
                    _m_bConnectedsuc = YES;
                    [self.delegate loggedIn];
                    startVideoStream(DEFAULTCHID);
                    if (NULL == _m_hThread) {
                        _m_hThread = (HFSTHREAD)fs_createThread((THREAD_FUNC_TO)&MediaThread, (__bridge void *)self);
                    }
                    break;
                }
                case FS_API_STATUS_LOGIN_USR_PWD_ERROR:
                {
                    _m_bConnectedsuc = NO;
                    [self.delegate loginFailed:nStatusId];
                    break;
                }
                case FS_API_STATUS_LOGIN_ACCESS_DENY:
                {
                    _m_bConnectedsuc = NO;
                    [self.delegate loginFailed:nStatusId];
                    break;
                }
                case FS_API_STATUS_LOGIN_EXCEED_MAX_USER:
                {
                    _m_bConnectedsuc = NO;
                    [self.delegate loginFailed:nStatusId];
                    break;
                }
                case FS_API_STATUS_LOGIN_CONNECT_FAIL:
                {
                    _m_bConnectedsuc = NO;
                    [self.delegate loginFailed:nStatusId];
                    break;
                }
                case FS_API_STATUS_LOGIN_ERROR_UNKNOW:
                {
                    _m_bConnectedsuc = NO;
                    [self.delegate loginFailed:nStatusId];
                    break;
                }
                case FS_API_STATUS_OPEN_VIDEO_SUCCESS:
                {
                    [self.delegate videoOpened];
                    break;
                }
                case FS_API_STATUS_OPEN_VIDEO_CONNECTING:
                {
                    break;
                }
                case FS_API_STATUS_OPEN_VIDEO_FAIL:
                {
                    [self.delegate videoFailedToOpen];
                    break;
                }
                    
                default:
                    break;
            }
        }
    }
}

void MediaThread(void *lParam)
{
	FoscamCamera * obj = (__bridge FoscamCamera *)lParam;
	[obj ThreadWork];
}

- (void)ThreadWork
{
    LOCK_MUTEX(&_m_liveViewMutex);
    int frameType = 0;
    int dataLen = BUFFER_SIZE;
    int isKeyFrame = 0;
    int videoWidth = 0;
    int videoHeight = 0;
    
    usleep(2000);
    
    while (_m_ThreadRun) {
        if (getVideoStreamData(&frameType, m_frameBuf, &dataLen, &isKeyFrame, &videoWidth, &videoHeight, DEFAULTCHID) > 0) {
            if (dataLen < 0 || dataLen > BUFFER_SIZE) {
                continue;
            }
                    
            frameType = MF_VIDEO;
            [self fsframeCall:frameType :m_frameBuf :dataLen :DEFAULTCHID];
        }
    }
    
	UNLOCK_MUTEX(&_m_liveViewMutex);
}

- (void)fsframeCall:(int)type :(char *)framedata :(int)len :(int)chid
{
	if (type == MF_VIDEO) {
        if (chid == DEFAULTCHID) {
            if (_m_iDeviceType == 1) {
                //H264
                char *data = FS_GetYUV420Data(_m_fsDecode, framedata, len);
                if (data == NULL) {
                    return;
                }
                
                if (_m_ViewFrame.width == 0 || _m_ViewFrame.height == 0) {
                    [self uninitDecoder:chid];
                    [self initDecoder:chid];
                    return;
                }
                
                AVPicture *d = (AVPicture *)data;
                [self imageFromAVPicture:*(d) width:_m_ViewFrame.width height:_m_ViewFrame.height chid:chid];
            }
        }
    }
}

- (UIImage *)imageFromAVPicture:(AVPicture)pict width:(int)width height:(int)height chid:(int)chid
{
    LOCK_MUTEX(&_m_hImageMutex);
	CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
	CFDataRef data = CFDataCreateWithBytesNoCopy(kCFAllocatorDefault, pict.data[0], pict.linesize[0] * height, kCFAllocatorNull);
	CGDataProviderRef provider = CGDataProviderCreateWithCFData(data);
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	CGImageRef cgImage = CGImageCreate(width,
									   height,
									   8,
									   24,
									   pict.linesize[0],
									   colorSpace,
									   bitmapInfo,
									   provider,
									   NULL,
									   NO,
									   kCGRenderingIntentDefault);
	
	CGColorSpaceRelease(colorSpace);
	UIImage* image = [[UIImage alloc] initWithCGImage:cgImage];
    
	CGImageRelease(cgImage);
	CGDataProviderRelease(provider);
	CFRelease(data);
    
    [self performSelectorOnMainThread:@selector(updateView:) withObject:image waitUntilDone:NO];
    
    UNLOCK_MUTEX(&_m_hImageMutex);
	return nil;
}

- (void)updateView:(UIImage *)img
{
    LOCK_MUTEX(&_m_hUpdateViewMutex);
    self.firstimageShowView.image = img;
    UNLOCK_MUTEX(&_m_hUpdateViewMutex);
}

@end
