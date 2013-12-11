/*
 *  FCCommon.cpp
 *  plugin_test
 *
 *  Created by foscam on 1/3/12.
 *  Copyright 2012 foscam. All rights reserved.
 *
 */
#include "stdafx.h"
#include "common.h"

#ifdef _DEBUG
#include <MMSystem.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif
void EnBase64(const unsigned char chasc[3],unsigned char chuue[4])
{
	int i,k=2;
	unsigned char t=0;
	for(i=0;i<3;i++)
	{
		*(chuue+i)=*(chasc+i)>>k;
		*(chuue+i)|=t;
		t=*(chasc+i)<<(8-k);
		t>>=2;
		k+=2;
	}
	*(chuue+3)=*(chasc+2)&63;
	
	for(i=0;i<4;i++)
	{
		if( *(chuue+i)==0 ) *(chuue+i)+=65;
		else if( (*(chuue+i)>0) && ( *(chuue+i) <= 25) ) *(chuue+i)+=65;
		else if((*(chuue+i)>=26)&&(*(chuue+i)<=51)) *(chuue+i)+=71;
		else if((*(chuue+i)>=52)&&(*(chuue+i)<=61)) *(chuue+i)-=4;
		else if(*(chuue+i)==62) *(chuue+i)=43;
		else if(*(chuue+i)==63) *(chuue+i)=47;
	}
}

int Base64Convert(const unsigned char *srcData,unsigned char *desData,int srcSize)
{
	int ProcessCnt;
	int RemainCnt;
	int i;	
	
	
	ProcessCnt = srcSize/3;
	RemainCnt  = srcSize % 3;
	
	ProcessCnt += (RemainCnt)?1:0;
	
	for(i=0;i<ProcessCnt;i++)
		EnBase64(i*3+srcData,i*4+desData);
	
	if(RemainCnt==2){				
		desData[ProcessCnt*4-1]='=';	
		desData[ProcessCnt*4-2]='=';					
	}else if(RemainCnt==1){
		desData[ProcessCnt*4-1]='=';			
	}
	
	desData[ProcessCnt*4]='\0';
	return ProcessCnt*4;
}

char *CmdEncode(const char *src, int len, char *dst)
{
	int ch;
	int l, h;
	
	for (int i = 0; i < len; i++) {
		ch = src[i];
		ch += i % 0x5D - 0x21;
		if (ch > 0x5D) {
			ch -= 0x5D;
		}
		h = ch / 10;
		l = ch % 10;
		
		if (l != 9 || h <= 3) {
			ch = l * 10 + h;
		}
		
		ch += 0x21;
		dst[i] = ch;
	}
	
	return dst;
}

int GetGolombValue(const unsigned char *s, int offset, unsigned *value)
{
	int a=0;a++;
	int len = -1;
	for (int b = 0; b == 0; len++, offset++) {
		if (offset >= 8) {
			s += offset / 8;
			offset %= 8;
		}
		b = ((*s) >> (7 - offset)) & 0x1;
	}

	if (value) {
		*value = (1 << len) - 1;
		unsigned t = 0;
		for (int i = 0; i < len; i++, offset++) {
			if (offset >= 8) {
				s += offset / 8;
				offset %= 8;
			}
			t <<= 1;
			t += ((*s) >> (7 - offset)) & 0x1;
		}
		*value += t;
	}

	return len * 2 + 1;
}

char *CmdDecode(const char *src, int len, char *dst)
{
	int ch;
	int l, h;
	
	for (int i = 0; i < len; i++) {
		ch = src[i];
		ch -= 0x21;
		h = ch / 10;
		l = ch % 10;
		if (l != 9 || h <= 3) {
			h += l;
			l = h - l;
			h -= l;
		}
		
		ch = h * 10 + l;
		ch -= i % 0x5D;
		if (ch < 0) {
			ch += 0x5D;
		}
		ch += 0x21;
		dst[i] = ch;
	}
	
	return dst;
}