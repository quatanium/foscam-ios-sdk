#ifndef	__FSTYPES_H
#define __FSTYPES_H
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <memory.h>
#include <unistd.h>
#include <netdb.h>

#define	WINAPI
#define FCNET_SDK

typedef long long				WPARAM;
typedef	long long				LPARAM;
typedef void *					HFSHTTP;
typedef void *					HFSSEARCH;
typedef void *					HFSCLIENT;
typedef void *					HFSTHREAD;

typedef signed char		int8_t;
typedef unsigned char   uint8_t;
typedef short			int16_t;
typedef unsigned short  uint16_t;
typedef int				int32_t;
typedef unsigned		uint32_t;

typedef pthread_mutex_t			MUTEX;
#define INIT_MUTEX(x)			pthread_mutex_init(x, NULL)
#define RELEASE_MUTEX(x)		pthread_mutex_destroy(x)
#define LOCK_MUTEX(x)			pthread_mutex_lock(x)
#define UNLOCK_MUTEX(x)			pthread_mutex_unlock(x)

#define	THREAD_FUNC_TO			void *(*)(void *)
#define	THREAD_FUNC				void * (*func)(void *)
typedef void* (*THREADFUNC)(void*);
typedef long long				DWORD;

typedef struct MSG
{
	u_int32_t	message;
	WPARAM		wParam;
	LPARAM		lParam;
}MSG;

typedef struct tagFRAME_HEAD{
	int			type;
	long long	timeStamp;
	uint32_t    videoFrameRate;
	uint32_t	length;
	uint32_t	isKeyFrame;
	uint32_t	videoBitRate;
	uint64_t	reserve;
	unsigned char*	frame;
} FRAME_HEAD, *LPFRAME_HEAD;

typedef void (*FRAME_CB)(int type, LPFRAME_HEAD frame_head, void* udata, int utype);
typedef int (*SEARCH_CB)(int type, void *buf, int length, void *userdata);
typedef void (*MSG_CB2)(int msg, int subMsg, void *param, void *userdata, int utype);

enum
{
	NET_MSG_CONNECT,
	NET_MSG_CTRL = 8,
	NET_MSG_TALK_OPEN = 9,
	NET_MSG_TALK_CLOSE = 10,
	
};

enum
{
	NETMSG_CONNECT_SUC,
	NETMSG_CONNECT_IPORPORTERROR,
	NETMSG_CONNECT_CONNECTERROR,
	NETMSG_TALK_SUC = 20,
	NETMSG_TALK_ERROR,
	
	NETMSG_CTRL_CAPTURE = 50
};

typedef enum {
	DT_NIL = 0,
	DT_MJPEG,
	DT_H264,
	DT_UDT_UDP,
	DT_UDT_TCP,
	DT_OTHER_HX,
	DT_UDT_MUTIL_CHANNEL_TUTK
} DEVICE_TYPE_T;

typedef enum {
	MF_UNKNOW = 0x100,
	MF_YUYV422,
	MF_YUV420P,
	MF_PCM,
	MF_IMAADPCM,
	MF_H264,
	MF_MJPEG,
	MF_G726,

	MF_VIDEO,
	MF_AUDIO
} MEDIA_FORMAT_T;

typedef enum{
	VCIF_STARTVIDEO,
	VCIF_STARTAUDIO,
	VCIF_STARTTALK,
	
	VCIF_TALKDATA=4,
	
	VCIF_STOPVIDEO,
	VCIF_STOPAUDIO,
	VCIF_STOPTALK,
	VCIF_CONTROL = 12,
}VCLIENT_INTERFACE;

typedef enum{
	VCIF_CONTROL_UP,
	VCIF_CONTROL_DOWN,
	VCIF_CONTROL_LEFT,
	VCIF_CONTROL_RIGHT,
	VCIF_CONTROL_LEFTUP,
	VCIF_CONTROL_RIGHTUP,
	VCIF_CONTROL_LEFTDOWN,
	VCIF_CONTROL_RIGHTDOWN,

	VCIF_CONTROL_STOP = 9,
	VCIF_CONTROL_CAPTURE = 23,
    
    //变焦
    VC_CONTROL_ZOOMIN=52,
    VC_CONTROL_ZOOMOUT,
    VC_CONTROL_ZOOMSTOP,
    //

}VCLIENTCONTROL;

typedef struct tagNETPARAM{
	long    groupid;
	char	ip[128];
	short	port;
	char	username[20];
	char	passwd[20];
	short	streamType;
    short	webPort;
}NETPARAM, *LPNETPARAM;

#define ATTRIBUTE_PACKED __attribute__((packed))
typedef struct {
	char		id[13];
	char		name[21];
	uint32_t	ip;
	uint32_t	mask;
	uint32_t	gateway;
	uint32_t	dns;
	char		type;
	char		isP2P;
	unsigned short 		mediaPort;
	uint32_t	sys_ver;
	uint32_t	app_ver;
	uint16_t	port;
	uint8_t		dhcp_enabled;
	char		uid[33];
} ATTRIBUTE_PACKED SEARCH_S;
#endif
