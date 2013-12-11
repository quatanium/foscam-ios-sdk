#ifndef	__FSAVDECODERSDK_H
#define __FSAVDECODERSDK_H

#define HAVDECODER	void*

typedef struct {
	int			type;
	long long	ts;
	int			sec;
	int			width;
	int			height;
	int			length;
	char*		frame;
} FRAME_P, *LPFRAME_P;

#ifdef	WIN32
#ifdef	FSAVDECOD_SDK
#else
#define FSAVDECOD_SDK extern "C" __declspec(dllimport)
#endif
#else
#define FSAVDECOD_SDK
#define WINAPI
#endif
/*****************************************************************
 * 宏定义
 *****************************************************************/

typedef void (*FSDECORD_CB)(int msg, void *param, void *userdata);

typedef enum {
	DCD_UNKNOW = 0x100,
	DCD_IMAADPCM,
	DCD_H264,
	DCD_MJPEG,
	DCD_G726
} DECODE_TYPE;

/************************************************************************/
/* 结构体                                                               */
/************************************************************************/


//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C" {
#endif
	
	FSAVDECOD_SDK HAVDECODER WINAPI FS_Create_Decoder();
	FSAVDECOD_SDK void WINAPI FS_Register_Decoder(HAVDECODER hD, int type, FSDECORD_CB cb, void* userdata);
	FSAVDECOD_SDK void WINAPI FS_UnRegister_Decoder(HAVDECODER hD, int type);
	FSAVDECOD_SDK void WINAPI FS_Release_Decoder(HAVDECODER hD);
	FSAVDECOD_SDK void WINAPI FS_Reset_Decoder(HAVDECODER hD);

	FSAVDECOD_SDK char* WINAPI FS_GetYUV420Data(HAVDECODER hD, char* data, int lenght);
	FSAVDECOD_SDK char* WINAPI FS_GetPCMData(HAVDECODER hD, char* data, int lenght);
	
	

#ifdef __cplusplus
}
#endif

#endif