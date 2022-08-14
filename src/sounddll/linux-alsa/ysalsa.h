#ifndef YS_ALSA_IS_INCLUDED
#define YS_ALSA_IS_INCLUDED

#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>

#include "../yswavfile/yswavfile.h"

class YsAlsaPlayer
{
private:
	class YsWavFile *oneShotWav;
	unsigned int oneShotWavPtr;
	class YsWavFile *environWav;
	unsigned int environWavPtr;

	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwParam;

	unsigned int nChannel,rate,bufSize;
	snd_pcm_uframes_t nPeriod;

	const unsigned int bytePerTimeStep;
	unsigned int bufSizeInNStep;
	unsigned int bufSizeInByte;
	unsigned char *writeBuf;


public:
	YsAlsaPlayer();
	~YsAlsaPlayer();

	void Initialize(void);
	void PlayOneShot(class YsWavFile *wavFile);
	void PlayEnviron(class YsWavFile *wavFile);
	void KeepPlaying(void);
private:
	void PopulateWriteBuffer(unsigned int &writeBufFilledInNStep,unsigned int &wavPtr,const YsWavFile *wavFile,YSBOOL loop);

public:
	void Stop(void);

	void PrintState(int errCode);
};


#endif
