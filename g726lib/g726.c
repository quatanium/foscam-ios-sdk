#include "stdio.h"
#include "memory.h"
#include "g72x.h"

extern void g726_Encode(unsigned char *speech,char *bitstream)
{
	g726_state state_ptr;
	short temp[480];
	int i;
	//char c;

	g726_init_state(&state_ptr);
	/*for(i=0;i<480;i++)
	{
		c=*(speech+i*2);
		*(speech+i*2)=*(speech+i*2+1);
		*(speech+i*2+1)=c;
	}*/
	memcpy(temp,speech,960);
	
	for(i=0;i<120;i++)
	{
		*(bitstream+i)=(((char)(g726_16_encoder(temp[i*4],AUDIO_ENCODING_LINEAR,&state_ptr)))<<6)|(((char)(g726_16_encoder(temp[i*4+1],AUDIO_ENCODING_LINEAR,&state_ptr)))<<4)|(((char)(g726_16_encoder(temp[i*4+2],AUDIO_ENCODING_LINEAR,&state_ptr)))<<2)|(((char)(g726_16_encoder(temp[i*4+3],AUDIO_ENCODING_LINEAR,&state_ptr))));
	}
}

extern void g726_Decode(char *bitstream,unsigned char *speech)
{
	g726_state state_ptr;
	int i;
	int in;
	short temp;

	g726_init_state(&state_ptr);
	
	for(i=0;i<120;i++)
	{
		in=(int)(((*(bitstream+i))&(char)192)>>6);
		temp=g726_16_decoder(in,AUDIO_ENCODING_LINEAR,&state_ptr);
		memcpy(speech+i*8,&temp,2);
		in=(int)(((*(bitstream+i))&(char)48)>>4);
		temp=g726_16_decoder(in,AUDIO_ENCODING_LINEAR,&state_ptr);
		memcpy(speech+i*8+2,&temp,2);
		in=(int)(((*(bitstream+i))&(char)12)>>2);
		temp=g726_16_decoder(in,AUDIO_ENCODING_LINEAR,&state_ptr);
		memcpy(speech+i*8+4,&temp,2);
		in=(int)(((*(bitstream+i))&(char)3));
		temp=g726_16_decoder(in,AUDIO_ENCODING_LINEAR,&state_ptr);
		memcpy(speech+i*8+6,&temp,2);
	}
}