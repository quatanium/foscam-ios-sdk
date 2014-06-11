#include "stdafx.h"
#include "MediaConvert.h"

MediaConvert::MediaConvert()
{
	avcodec_register_all();
	
	_en_sample = 0;
	_en_index = 0;
	_de_sample = 0;
	_de_index = 0;
	
	_audio_ctx = NULL;
	_video_ctx = NULL;
	_width = 0;
	_height = 0;
}

MediaConvert::~MediaConvert()
{
}

/*
 42 00 1e e9 01 40 7b 7f e0 00 00 
 
 5 :0100 0010 
 6 :0000 0000 
 7 :0001 1110 
 8 :1110 1001 
 9 :0000 0001 
 10 :0100 0000 
 11 :0111 1011 
 12 :0111 1111 
 13 :1110 0000 
 14 :0000 0000 
 15 :0000 0000 
 */

/*
 42 00 28 e9 00 a0 0b 77 fe 00 00 
 
 5 :0100 0010 
 6 :0000 0000 
 7 :0010 1000 
 8 :1110 1001 
 9 :0000 0000 
 10 :1010 0000 
 11 :0000 1011 
 12 :0111 0111 
 13 :1111 1110 
 14 :0000 0000 
 15 :0000 0000 
 */
bool MediaConvert::ParseSPS(const unsigned char *sps, int *width, int *height)
{
	unsigned v;
	int off = 0;

	uint8_t profile_idc = *((uint8_t *)sps);
	sps += 3;
	
	off += GetGolombValue(sps, off, NULL);	//seq_parameter_set_id
	
	if (profile_idc == 100 || profile_idc == 110 || profile_idc == 122 || profile_idc == 144)
	{
		abort();	// No treatment
	}
	
	off += GetGolombValue(sps, off, NULL);	//log2_max_frame_num_minus4
	off += GetGolombValue(sps, off, &v);	//pic_order_cnt_type
	if (v == 0) 
	{
		off += GetGolombValue(sps, off, NULL);	//log2_max_pic_order_cnt_lsb_minus4
	}
	else if (v == 1)
	{
		abort();	// No treatment
	}
	
	off += GetGolombValue(sps, off, &v);	//num_ref_frames
	off += 1;	//graps_in_num_value_allowed_flag
	off += GetGolombValue(sps, off, &v);	//pic_width_in_mbs_minus1
	*width = (v + 1) * 16;	// +1 Deal needs to be done is not clear why the
	int a=0;a++;
	off += GetGolombValue(sps, off, &v);	//pic_heigth_in_map_units_minus1
	*height = (v + 1) * 16;	
	return true;
}

bool MediaConvert::ParseJpeg(const uint8_t *buf, int len, int *w, int *h)
{
	int index = 0;
	if (*((uint16_t *)buf) != 0xD8FF) {
		return false;
	}
	index += 2;
	
	uint16_t seg_len;
	bool is_find = false;
	
	for (; index < len; index++) {
		if (buf[index] == 0xFF) {
			index++;
			if (buf[index] == 0xC0) {
				is_find = true;
				break;
			}
			
			seg_len = buf[index + 1] << 8 | buf[index + 2];
			index += seg_len;
		}
	}
	
	if (!is_find) {
		return false;
	}
	
	index += 4;
	*h = buf[index] << 8 | buf[index + 1];
	*w = buf[index + 2] << 8 | buf[index + 3];
	
	return true;
}

uint8_t *MediaConvert::CreateBMP(uint8_t *src, enum PixelFormat fmt, 
								 int width, int height, int *len, bool reverse)
{
	AVFrame *frame_src = av_frame_alloc();
	AVFrame *frame_dst = av_frame_alloc();
	enum PixelFormat bmp_fmt = PIX_FMT_BGR24;
	
	int bytes = avpicture_get_size(bmp_fmt, width, height);
	*len = bytes + sizeof(BMP_FILE_HDR) + sizeof(BMP_INFO_HDR);
	uint8_t *dst_data = (uint8_t *)malloc(*len);
	memset(dst_data, 0x0, *len);
	uint8_t *img_data = dst_data + sizeof(BMP_FILE_HDR) + sizeof(BMP_INFO_HDR);
	
	bool is_cvt = false;
	if (fmt == bmp_fmt) {
		memcpy(img_data, src, bytes);
		is_cvt = true;
	}
	else {
		avpicture_fill((AVPicture *)frame_dst, img_data, bmp_fmt, width, height);
		avpicture_fill((AVPicture *)frame_src, src, fmt, width, height);
		struct SwsContext *sws = 
		sws_getContext(width, height, fmt,
					   width, height, bmp_fmt, 
					   SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if (sws) {
			int h = sws_scale(sws, (uint8_t** const)frame_src->data, frame_src->linesize, 
							  0, height, frame_dst->data, frame_dst->linesize);
			if (h == height) {
				is_cvt = true;
			}
			sws_freeContext(sws);
		}
		
		av_free(frame_src);
		av_free(frame_dst);
	}
	
	if (!is_cvt) {
		free(dst_data);
		return NULL;
	}
	
	if (reverse) {
		int line_bytes = bytes / height;
		uint8_t *buf_tmp = (uint8_t *)malloc(line_bytes);
		for (int j = 0; j < height / 2; j++) {
			memcpy(buf_tmp, img_data + line_bytes * j, line_bytes);
			memcpy(img_data + line_bytes * j, img_data + line_bytes * 
				   (height - j - 1), line_bytes);
			memcpy(img_data + line_bytes * (height - j - 1), buf_tmp, line_bytes);
		}
		free(buf_tmp);
	}
	
	BMP_FILE_HDR *fh = (BMP_FILE_HDR *)dst_data;
	BMP_INFO_HDR *bh = (BMP_INFO_HDR *)(dst_data + sizeof(BMP_FILE_HDR));
	
	fh->bfType = 0x4D42;
	fh->bfSize = sizeof(BMP_FILE_HDR) + sizeof(BMP_INFO_HDR) + bytes;
	fh->bfOffBits = sizeof(BMP_FILE_HDR) + sizeof(BMP_INFO_HDR);
	bh->biSize = sizeof(BMP_INFO_HDR);
	bh->biWidth = width;
	bh->biHeight = height;
	bh->biPlanes = 1;
	bh->biBitCount = 24;
	bh->biCompression = 0;
	bh->biSizeImage = 0;
	bh->biXPelsPerMeter = 0;
	bh->biYPelsPerMeter = 0;
	bh->biClrUsed = 0;
	bh->biClrImportant = 0;
	
	return dst_data;
}


/*****************************************************************
 * video
 *****************************************************************/

bool MediaConvert::VideoDecOpen(enum AVCodecID ci, int w, int h, enum PixelFormat fd)
{
	AVCodec *codec = avcodec_find_decoder(ci);
	if (codec == NULL) {
		goto video_dec_open_fail;
	}
	
	_video_ctx = avcodec_alloc_context3(codec);
//	_video_ctx->pix_fmt = PIX_FMT_YUYV422;
	avcodec_open2(_video_ctx, codec, NULL);
	
	_frame_src = av_frame_alloc();
	_frame_dst = av_frame_alloc();
	_width = w;
	_height = h;
	_fmt_dst = fd;
	_sws_ctx = NULL;
	
	_byte_num = avpicture_get_size(fd, w, h);
	_img_buf = (uint8_t *)av_malloc(_byte_num);
	avpicture_fill((AVPicture *)_frame_dst, _img_buf, fd, w, h);
	
	return true;
	
video_dec_open_fail:
	if (_video_ctx) {
		avcodec_close(_video_ctx);
		av_free(_video_ctx);
	}
	
	return false;
}

void MediaConvert::VideoDecClose()
{
	avcodec_close(_video_ctx);
	av_free(_video_ctx);
	_video_ctx = NULL;
	
	if (_sws_ctx) {
		sws_freeContext(_sws_ctx);
	}
	av_free(_img_buf);
	av_free(_frame_src);
	av_free(_frame_dst);
}

void yuv420p_to_yuv422(uint8_t * cy, uint8_t * cu, uint8_t * cv, uint8_t * dest, int width, int height) 
{ 
    unsigned int x, y; 
	
	
	for (y = 0; y < height; ++y) { 
		uint8_t *Y = cy + y * width; 
        uint8_t *Cb = cu + (y / 2) * (width / 2); 
		uint8_t *Cr = cv + (y / 2) * (width / 2); 
        for (x = 0; x < width; x += 2) { 
			*(dest + 0) = Y[0]; 
			*(dest + 1) = Cb[0]; 
			*(dest + 2) = Y[1]; 
            *(dest + 3) = Cr[0]; 
			dest += 4; 
			Y += 2; 
			++Cb; 
			++Cr; 
		} 
	} 
}

bool MediaConvert::VideoDecFrame(uint8_t *src, int size, uint8_t *dst)
{
	int fh = 0;
	int h = 0;
	bool ret = false;
	int _w=_width,_h=_height;
	
	AVPacket pkt;
	memset(&pkt, 0x0, sizeof(AVPacket));
	pkt.size = size;
	pkt.data = src;
	pkt.pos = -1;
	avcodec_decode_video2(_video_ctx, _frame_src, &fh, &pkt);
	if (fh > 0) {
		if (_video_ctx->pix_fmt != _fmt_dst) {
			if (_sws_ctx == NULL) {
				// Allocate RGB picture
				avpicture_alloc(&picture, PIX_FMT_RGB24, _width, _height);
				
				// Setup scaler
//				static int sws_flags =  SWS_FAST_BILINEAR;
				if (_width == 1280) {
					_w = 640;
					_h = 360;
				}
				_sws_ctx = sws_getContext(_width, _height, _video_ctx->pix_fmt,
										  _w, _h, PIX_FMT_RGB24,
										  SWS_FAST_BILINEAR, NULL, NULL, NULL);
				
				//_sws_ctx = sws_getContext(_width, _height, _video_ctx->pix_fmt, 
				//							  _width, _height, _fmt_dst, 
				//							  SWS_FAST_BILINEAR, NULL, NULL, NULL);
			//	printf(" -- sws1 \n");
			}
			h = sws_scale (_sws_ctx, _frame_src->data, _frame_src->linesize,
						   0, _video_ctx->height,
						   picture.data, picture.linesize);
			//h = sws_scale(_sws_ctx, _frame_src->data, 
			//			  _frame_src->linesize, 0, _height, 
			//			  _frame_dst->data, _frame_dst->linesize);
			/*if (h > 0) {
				memcpy(dst, _img_buf, _byte_num);
				ret = true;
			}else {
			//	printf(" -- sws2 \n");
			}*/
			ret = true;
		}
		else {
			for (int i=0, nDataLen=0; i<3; i++)
			{
				char* pYUVData = (char*)_frame_src->data[i];
				int nLength = (i==0)?_video_ctx->height:(_video_ctx->height>>1);
				int nMemLen = (i==0)?_video_ctx->width:(_video_ctx->width>>1);
				for (int j=0; j<nLength; j++)
				{
					memcpy(&dst[nDataLen], pYUVData, nMemLen);
					pYUVData += _frame_src->linesize[i];
					nDataLen += nMemLen;
				}
			}
			
			ret = true;
		}
	}
	
	return ret;
}

int MediaConvert::VideoWidth()
{
	return _width;
}

int MediaConvert::VideoHeight()
{
	return _height;
}

int MediaConvert::VideoByteCount()
{
	return _byte_num;
}

bool MediaConvert::IsVideoDevOpen()
{
	if (_video_ctx) {
		return true;
	}
	
	return false;
}

/*****************************************************************
 * G726
 *****************************************************************/

bool MediaConvert::AudioDecOpen(enum AVCodecID codec_id, int bit_rate, int sample_rate, int channels)
{
	AVCodec *codec = avcodec_find_decoder(codec_id);
	if (!codec) {
		return false;
	}
	
    _audio_ctx = avcodec_alloc_context3(codec);
	_audio_ctx->bit_rate = bit_rate;
	_audio_ctx->sample_rate = sample_rate;
	_audio_ctx->channels = channels;
	
    avcodec_open2(_audio_ctx, codec, NULL);
	
	return true;
}

void MediaConvert::AudioDecClose()
{
	avcodec_close(_audio_ctx);
	av_free(_audio_ctx);
	_audio_ctx = NULL;
}

bool MediaConvert::AudioDecFrame(uint8_t *src, int16_t *dst, int *size)
{
	AVPacket packet;
    AVFrame *decoded_frame = NULL;

	memset(&packet, 0x0, sizeof(AVPacket));
	packet.size = *size;
	packet.data = src;
	packet.pos = -1;
	*size = 0;
    
    if (!decoded_frame) {
        if (!(decoded_frame = av_frame_alloc())) {
            fprintf(stderr, "out of memory\n");
            exit(1);
        }
    } else {
        av_frame_unref(decoded_frame);
    }

	   
    avcodec_decode_audio4(_audio_ctx, decoded_frame, size, &packet);
	
	return true;
}


/*****************************************************************
 * ima adpcm
 *****************************************************************/

static int index_adjust[8] = {-1,-1,-1,-1,2,4,6,8};

static int step_table[89] = 
{
	7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
	50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,
	408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,
	2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,
	10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767
};

void MediaConvert::ImaAdpcmInitEncode(int sample, int index)
{
	_en_sample = sample;
	_en_index = index;
}

void MediaConvert::ImaAdpcmInitDecode(int sample, int index)
{
	_de_sample = sample;
	_de_index = index;
}

void MediaConvert::ImaAdpcmEncode(const unsigned char * raw, int len, unsigned char * encoded)
{
	short * pcm = (short *)raw;
	int cur_sample;
	int i;
	int delta;
	int sb;
	int code;
	len >>= 1;
	
	for (i = 0;i < len;i ++)
	{
		cur_sample = pcm[i]; // Get the current sample data
		delta = cur_sample - _en_sample; // Calculated on an incremental one, and
		if ( delta < 0 )
		{
			delta = -delta;
			sb = 8;
		}
		else 
		{
			sb = 0;
		}	// sb - The sign bit is saved
		code = 4 * delta / step_table[_en_index];	// A value of 0-7 according steptable []
		if (code>7) 
			code=7;	// It describes the amount of change in sound intensity
		
		delta = (step_table[_en_index] * code) / 4 + step_table[_en_index] / 8;	// Is followed by an order to reduce errors
		if (sb) 
			delta = -delta;
		_en_sample += delta;
		if (_en_sample > 32767)
			_en_sample = 32767;
		else if (_en_sample < -32768)
			_en_sample = -32768;
		
		_en_index += index_adjust[code];
		if (_en_index < 0) 
			_en_index = 0;
		else if (_en_index > 88) 
			_en_index = 88;
		
		if (i & 0x01)
			encoded[i >> 1] |= code | sb;
		else
			encoded[i >> 1] = (code | sb) << 4;
	}
}

void MediaConvert::ImaAdpcmDecode(const unsigned char * raw, int len, unsigned char * decoded)
{
	int i;
	int code;
	int sb;
	int delta;
	short * pcm = (short *)decoded;
	len <<= 1;
	
	for (i = 0;i < len;i ++)
	{
		if (i & 0x01)
			code = raw[i >> 1] & 0x0f;
		else
			code = raw[i >> 1] >> 4;
		if ((code & 8) != 0) 
			sb = 1;
		else 
			sb = 0;
		code &= 7;
		
		delta = (step_table[_de_index] * code) / 4 + step_table[_de_index] / 8;
		if (sb) 
			delta = -delta;
		_de_sample += delta;
		if (_de_sample > 32767)
			_de_sample = 32767;
		else if (_de_sample < -32768)
			_de_sample = -32768;
		pcm[i] = _de_sample;
		/*
		 if (_de_sample > 32767)
		 pcm[i] = 32767;
		 else if (_de_sample < -32768)
		 pcm[i] = -32768;
		 else 
		 pcm[i] = _de_sample;
		 */
		_de_index += index_adjust[code];
		if (_de_index < 0) 
			_de_index = 0;
		if (_de_index > 88) 
			_de_index = 88;
	}
}
