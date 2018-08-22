#ifndef _OAL_AUDIO_H_
#define _OAL_AUDIO_H_

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// audio sound chip
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// audio volume interface for 100 levels
#define USER_VOLUME_LEVEL	100
#define USER_VOLUME_DEFAULT	60

#define MAX_PCM_IO_LENGTH	2048

typedef struct{
    char isInputMode; //isInputMode : 0 for output, 1 for input
    char isInputInChannelLeft;
    unsigned short maxInputOutputSize;
    unsigned short sampleRate;
    unsigned char channelNum;
    unsigned char bitsOfsample;
}OAL_AUDIO_CFG;

// interface, note: *cfg will be changed by the hardware capbility.
void* OAL_OpenAudio(OAL_AUDIO_CFG *cfg);
void OAL_CloseAudio(void *p);

// loop call to read or write pcm data.
void OAL_InputPCM(void *p, unsigned int* data, unsigned int length);
void OAL_OutputPCM(void *p, unsigned int* data, unsigned int length);

// without instance interface
int OAL_SetVolume(int volume);
int OAL_GetVolume();
int OAL_SetMute(int onOff);
int OAL_IsHeadphoneExist();

// init
void OAL_InitAudio();
void OAL_UninitAudio();


#endif
