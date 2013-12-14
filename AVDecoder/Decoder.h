#pragma once
#include "MediaConvert.h"
#include <iostream>
using namespace std;

class CDecoder
{
public:
	CDecoder(void);
	~CDecoder(void);

	void SetDecoder(int type, FSDECORD_CB cb, void* userdata);
	void UnRegistDecord(int type);
	
	void ResetDecord();
	
	char* GetYUV420Buffer(char* data, int lenght);
	char* GetPCMBuffer(char* data, int lenght);

protected:
	char* (CDecoder::*GetYUV420Data)(char* data, int lenght);
	char* (CDecoder::*GetPCMData)(char* data, int lenght);

	char* H2642YUV420(char* data, int lenght);
	char* MJPG2YUV420(char* data, int lenght);

	char* ADPCM2PCM(char* data, int lenght);
	char* G726TOPCM(char* data, int lenght);

private:
	FSDECORD_CB m_audioCb;
	FSDECORD_CB m_videoCb;

	/*½âÂë²ÎÊý*/
	MediaConvert *_cvt;
	FRAME_P	_frame;
	int		_height;
	int		_width;

	void* m_userdata;
	bool m_bResetDecord;
};
