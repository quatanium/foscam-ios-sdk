#ifndef _FS_API_H
#define _FS_API_H

//#include <jni.h>
#include "FSApiStatusID.h"
#include "types.h"

//extern JavaVM* g_jvm_Status;

#ifdef __cplusplus
extern "C" {
#endif


#define MAX_DEV_LIST	100
typedef struct tagFSIPCDevInfo
{
	int devType; //0=MJ 1=H264
	char devName[64];
	char ip[32];
	char mac[20];
	int webPort;
	int mediaPort;
	char uid[64];
} FSIPCDevInfo;


typedef void (*FSIPC_STATUS_CALLBACK)(int statusID, int chid);
typedef void (*FSIPC_SEARCH_DEV_CALLBACK)(int devType, char* devName, char* ip, char* mac, int webPort, int mediaPort, char* uid);
typedef void (*FSIPC_AV_CALLBACK)(int frameType, char* data, int dataLen, int isKeyFrame, int width, int height, int chId);
typedef void (*FSIPC_ALARM_CALLBACK)(int alarmType);


extern void Init();
extern void Uninit();

extern int getStatusId(int chId);
extern int searchDev(int chId);
extern int getDevList(FSIPCDevInfo* pDevList, int cnt);


extern int usrLogIn(int devType, const char* ip, const char* userName, const char* password, int streamType, int webPort, int mediaPort, char* uid,int chId);
extern int usrLogOut(int chId);
extern int ChangeUserInfo(char* OldUserName, char* NewUserName, char* OldPassWord, char* NewPassWord, int chId);

// ptz
extern int ptzMoveUp(int chId);
extern int ptzMoveDown(int chId);
extern int ptzMoveLeft(int chId);
extern int ptzMoveRight(int chId);
extern int ptzMoveTopLeft(int chId);
extern int ptzMoveTopRight(int chId);
extern int ptzMoveBottomLeft(int chId);
extern int ptzMoveBottomRight(int chId);
extern int ptzStopRun(int chId);
extern int zoomIn(int chId);
extern int zoomOut(int chId);
extern int zoomStop(int chId);

//
extern int startVideoStream(int chId);
extern int getVideoStreamData(int* pFrameType, char* frameBuf, int* pDataLen, int* pIsKeyFrame, int *pVideoWidth, int* pVideoHeight ,int chId);
extern int stopVideoStream(int chId);
extern int startAudioStream(int chId);
extern int getAudioStreamData(char* frameBuf, int* pDataLen,int chId);
extern int stopAudioStream(int chId);
extern int startTalk(int chId);
extern int stopTalk(int chId);
extern int sendTalkFrame(char* data, unsigned int dataLen,int chId);
extern int snapPic(char* dir,int chId);

//
extern int startAlarmMonitor(FSIPC_ALARM_CALLBACK cbk, void* pContext,int chId);
extern int startRecord(char* dir, char* fileName, int chId);
extern int stopRecord(int chId);


//avi play
extern void InitAvi();
extern void UninitAvi();
extern int startAviPlay(int* type, char* fileName, unsigned int* pVideoWidth, unsigned int* pVideoHeight, unsigned int* pTotalFrame, unsigned int* pTotalTime);
extern int stopAviPlay(void);
extern int seekToPosAvi(unsigned int time, unsigned int* pActualTime);
extern int getAVIFrame(int *pFrameType, char* frameBuf, int *pBufSize, int* pIsKeyFrame, unsigned char* extraData, int* pExtraDataSize);
extern int GetVideoWidth();
extern int GetVideoHeight();
extern int GetdwMaxBytesPerSec();
extern int GetMicroSecPerFrame();
extern void* GetAviFileHandle();
    
#ifdef __cplusplus
}
#endif

#endif
