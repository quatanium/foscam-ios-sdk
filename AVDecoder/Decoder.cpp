#include "StdAfx.h"
#include "Decoder.h"

CDecoder::CDecoder(void)
{
	_cvt = new MediaConvert;
	memset(&_frame, 0x0, sizeof(FRAME_P));
	_height = 0;
	_width = 0;
	
	m_bResetDecord = false;
}

CDecoder::~CDecoder(void)
{
	if (_cvt)
	{
		if (_cvt->IsVideoDevOpen()) {
			_cvt->VideoDecClose();
		}
		delete _cvt;
		_cvt = NULL;
	}
	
	if (_frame.frame) {
		free(_frame.frame);
		_frame.frame = NULL;
	}
}

void CDecoder::SetDecoder(int type, FSDECORD_CB cb, void* userdata)
{
	switch (type)
	{
	case DCD_IMAADPCM:
		m_audioCb = cb;
		GetPCMData = &CDecoder::ADPCM2PCM;
		break;
	case DCD_H264:
		m_videoCb = cb;
		GetYUV420Data = &CDecoder::H2642YUV420;
		break;
	case DCD_MJPEG:
		m_videoCb = cb;
		GetYUV420Data = &CDecoder::MJPG2YUV420;
		break;
	case DCD_G726:
		m_audioCb = cb;
		GetPCMData = &CDecoder::G726TOPCM;
		break;
	}
	m_userdata = userdata;
}

void CDecoder::UnRegistDecord(int type)
{
	switch (type)
	{
		case DCD_IMAADPCM:
			break;
		case DCD_H264:
			break;
		case DCD_MJPEG:
			break;
		case DCD_G726:
			break;
	}
	m_audioCb = NULL;
	m_userdata = NULL;
	m_videoCb = NULL;
}

void CDecoder::ResetDecord()
{
	m_bResetDecord = true;
}

char* CDecoder::H2642YUV420(char* data, int lenght)
{
	if (m_bResetDecord) {
		_width = _height = 0;
		_cvt->InitWH();
		m_bResetDecord = false;
	}
	if (0x67 == data[4])//解析SPS帧
	{
		MediaConvert::ParseSPS((unsigned char*)&(data[5]), &_width, &_height);
	}
    if(_width == 0 || _height == 0)
        return NULL;

	if (_width != _cvt->VideoWidth() || _height != _cvt->VideoHeight()) //图像大小发生改变
	{
		//重启解码器
		if (_cvt->IsVideoDevOpen()) {
			_cvt->VideoDecClose();
		}

		_cvt->VideoDecOpen(CODEC_ID_H264, _width, _height, PIX_FMT_YUYV422);
		//填充要解码的帧头格式
		if (_frame.frame) {
			free(_frame.frame);
		}
		_frame.length = _cvt->VideoByteCount();
		_frame.frame = (char *)malloc(_frame.length);
		_frame.width = _width;
		_frame.height = _height;
		assert(_frame.frame);

		if (m_videoCb)
		{
			if (_width == 1280) {
				_frame.width = _width/2;
				_frame.height = _height/2;
			}
			(*m_videoCb)(0, (void*)&_frame, m_userdata);
		}
	}

	if (_cvt->VideoWidth() != 0 && _cvt->VideoHeight() !=0 &&
		_cvt->VideoDecFrame((uint8_t*)data, lenght, (uint8_t*)_frame.frame)) {	//解码
		return (char*)&(_cvt->picture);

	}
	return NULL;
}

char* CDecoder::MJPG2YUV420(char* data, int lenght)
{
	return NULL;
}

char* CDecoder::ADPCM2PCM(char* data, int lenght)
{
	return NULL;
}

char* CDecoder::G726TOPCM(char* data, int lenght)
{
	return NULL;
}

char* CDecoder::GetYUV420Buffer(char* data, int lenght)
{
	return (this->*GetYUV420Data)(data, lenght);
}

char* CDecoder::GetPCMBuffer(char* data, int lenght)
{
	return (this->*GetPCMData)(data, lenght);
}