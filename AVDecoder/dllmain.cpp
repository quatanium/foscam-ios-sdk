// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "Decoder.h"
#ifdef WIN32

#define AUDIO_SDK extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

#endif

HAVDECODER WINAPI FS_Create_Decoder()
{
	return (HAVDECODER)(new CDecoder());
}

void WINAPI FS_Release_Decoder(HAVDECODER hD)
{
	assert(hD != NULL);
	delete (CDecoder*)hD;
}

void WINAPI FS_Register_Decoder(HAVDECODER hD, int type, FSDECORD_CB cb, void* userdata)
{
	assert(hD != NULL);
	((CDecoder*)hD)->SetDecoder(type, cb, userdata);
}

void WINAPI FS_UnRegister_Decoder(HAVDECODER hD, int type)
{
	assert(hD != NULL);
	((CDecoder*)hD)->UnRegistDecord(type);
}

void WINAPI FS_Reset_Decoder(HAVDECODER hD)
{
	assert(hD != NULL);
	((CDecoder*)hD)->ResetDecord();
}

char* WINAPI FS_GetYUV420Data(HAVDECODER hD, char* data, int lenght)
{
	assert(hD != NULL);
	CDecoder* d = (CDecoder*)hD;
	return d->GetYUV420Buffer(data, lenght);
}

char* WINAPI FS_GetPCMData(HAVDECODER hD, char* data, int lenght)
{
	return NULL;
}