#ifndef __FCMEDIA_H
#define __FCMEDIA_H

#include "common.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
}
#pragma pack (push ,1)

typedef struct { /* bmfh */
	uint16_t bfType;  
	uint32_t bfSize; 
	uint16_t bfReserved1; 
	uint16_t bfReserved2; 
	uint32_t bfOffBits;
} BMP_FILE_HDR;

typedef struct { /* bmih */
	uint32_t biSize; 
	uint32_t biWidth; 
	uint32_t biHeight; 
	uint16_t biPlanes; 
	uint16_t biBitCount; 
	uint32_t biCompression; 
	uint32_t biSizeImage; 
	uint32_t biXPelsPerMeter; 
	uint32_t biYPelsPerMeter; 
	uint32_t biClrUsed; 
	uint32_t biClrImportant;
} BMP_INFO_HDR;

#pragma pack (pop)


class MediaConvert {
public:
	//H264
	static bool ParseSPS(const unsigned char *sps, int *width, int *height);	//解析psp帧

	//MJPEG
	static bool ParseJpeg(const uint8_t *buf, int len, int *w, int *h);
	
	//使用完需将指针free
	static uint8_t *CreateBMP(uint8_t *src, enum PixelFormat fmt, int width, 
							  int height, int *len, bool reverse = false);
	
	/*****************************************************************
	 * video
	 *****************************************************************/
	bool VideoDecOpen(enum CodecID ci, int w, int h, enum PixelFormat fd);
	void VideoDecClose();
	bool VideoDecFrame(uint8_t *src, int size, uint8_t *dst);
	int VideoWidth();
	int VideoHeight();
	int VideoByteCount();
	bool IsVideoDevOpen();
	
	void InitWH(){_width = _height = 0;}
	
	/*****************************************************************
	 * ima adpcm
	 *****************************************************************/
	void ImaAdpcmInitEncode(int sample, int index);
	void ImaAdpcmInitDecode(int sample, int index);
	void ImaAdpcmEncode(const unsigned char * raw, int len, unsigned char * encoded);
	void ImaAdpcmDecode(const unsigned char * raw, int len, unsigned char * decoded);
	
	/*****************************************************************
	 * G726
	 *****************************************************************/
	bool AudioDecOpen(enum CodecID codec_id, int bit_rate, int sample_rate, int channels);
	void AudioDecClose();
	bool AudioDecFrame(uint8_t *src, int16_t *dst, int *size);
	
	
	MediaConvert();
	~MediaConvert();
	
private:
	int		_en_sample;
	int		_en_index;
	int		_de_sample;
	int		_de_index;
	
	AVCodecContext	*_audio_ctx;
	
	AVCodecContext	*_video_ctx;
	struct SwsContext	*_sws_ctx;
	AVFrame	*_frame_src;
	AVFrame	*_frame_dst;
	int		_width;
	int		_height;
	int		_byte_num;
	uint8_t	*_img_buf;
	enum PixelFormat	_fmt_dst;
	
public:
	AVPicture picture;
};


#endif