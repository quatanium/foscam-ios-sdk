/*
 *  FCCommon.h
 *  plugin_test
 *
 *  Created by foscam on 1/3/12.
 *  Copyright 2012 foscam. All rights reserved.
 *
 */

#ifndef	__FCCOMMON_H
#define	__FCCOMMON_H
#ifdef _DEBUG
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif
#define lengthof(x) (sizeof(x)/sizeof(*x))

void EnBase64(const unsigned char chasc[3],unsigned char chuue[4]);
int Base64Convert(const unsigned char *srcData,unsigned char *desData,int srcSize);
char *CmdEncode(const char *src, int len, char *dst);
char *CmdDecode(const char *src, int len, char *dst);

int GetGolombValue(const unsigned char *s, int offset, unsigned *value);

#endif