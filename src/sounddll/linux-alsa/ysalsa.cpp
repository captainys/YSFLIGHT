#include <stdio.h>
#include "ysalsa.h"

YsAlsaPlayer::YsAlsaPlayer() : 
	oneShotWav(NULL),
	oneShotWavPtr(0),
	environWav(NULL),
	environWavPtr(0),
	handle(NULL),
	hwParam(NULL),
	nChannel(0),
	rate(0),
	nPeriod(0),
	bufSize(0),
	bytePerTimeStep(2) // 2 bytes for 16-bit mono stream per time step
{
	int res=snd_pcm_open(&handle,"default",SND_PCM_STREAM_PLAYBACK,SND_PCM_NONBLOCK);
	if(0>res)
	{
		printf("Cannot open PCM device.\n");
		handle=NULL;
		return;
	}

	snd_pcm_hw_params_alloca(&hwParam);
	snd_pcm_hw_params_any(handle,hwParam);
	snd_pcm_hw_params_set_access(handle,hwParam,SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle,hwParam,SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(handle,hwParam,1);

	unsigned int request=8000;//22050;
	int dir;  // What's dir?
	snd_pcm_hw_params_set_rate_near(handle,hwParam,&request,&dir);

	if(0>snd_pcm_hw_params(handle,hwParam))
	{
		printf("Cannot set hardward parameters.\n");
		return;
	}

	snd_pcm_hw_params_get_channels(hwParam,&nChannel);
	snd_pcm_hw_params_get_rate(hwParam,&rate,&dir);
	snd_pcm_hw_params_get_period_size(hwParam,&nPeriod,&dir);
	snd_pcm_hw_params_get_buffer_size(hwParam,(snd_pcm_uframes_t *)&bufSize);
	printf("%d channels, %d Hz, %d periods, %d frames buffer.\n",
		   nChannel,rate,(int)nPeriod,(int)bufSize);

	bufSizeInNStep=nPeriod*4;
	bufSizeInByte=bufSizeInNStep*bytePerTimeStep;
	writeBuf=new unsigned char [bufSizeInByte];
	

	snd_pcm_prepare(handle);
	if(0==snd_pcm_wait(handle,1000))
	{
		printf("snd_pcm_wait timed out.\n");
		printf("There may be no sound.\n");
	}
	snd_pcm_start(handle);

	Initialize();
}

YsAlsaPlayer::~YsAlsaPlayer()
{
	snd_pcm_close(handle);
	delete [] writeBuf;
}

void YsAlsaPlayer::Initialize(void)
{
	oneShotWav=NULL;
	oneShotWavPtr=0;
	environWav=NULL;
	environWavPtr=0;
}

void YsAlsaPlayer::PlayOneShot(class YsWavFile *wavFile)
{
	if(NULL!=wavFile)
	{
		wavFile->Resample(rate);
		wavFile->ConvertToMono();
		wavFile->ConvertTo16Bit();
		wavFile->ConvertToSigned();
	}

	if(oneShotWav!=wavFile)
	{
		oneShotWav=wavFile;
		oneShotWavPtr=0;
		if(NULL!=handle)
		{
			snd_pcm_drop(handle);
			snd_pcm_prepare(handle);
			snd_pcm_wait(handle,1);
		}
	}
}

void YsAlsaPlayer::PlayEnviron(class YsWavFile *wavFile)
{
	if(NULL!=wavFile)
	{
		wavFile->Resample(rate);
		wavFile->ConvertToMono();
		wavFile->ConvertTo16Bit();
		wavFile->ConvertToSigned();
	}

	if(environWav!=wavFile)
	{
		environWav=wavFile;
		environWavPtr=0;
		if(NULL!=handle && (NULL==oneShotWav || oneShotWavPtr>=oneShotWav->NTimeStep()))
		{
			snd_pcm_drop(handle);
			snd_pcm_prepare(handle);
			snd_pcm_wait(handle,1);
		}
	}
}

void YsAlsaPlayer::KeepPlaying(void)
{
	if(NULL!=handle)
	{
		const int nAvail=(unsigned int)snd_pcm_avail_update(handle);
		if(nPeriod<nAvail)
		{
			unsigned int writeBufFilledInNStep=0;
			if(NULL!=oneShotWav)
			{
				unsigned int wavPtr=oneShotWavPtr;
				PopulateWriteBuffer(writeBufFilledInNStep,wavPtr,oneShotWav,YSFALSE);
			}
			else if(NULL!=environWav)
			{
				unsigned int wavPtr=environWavPtr;
				PopulateWriteBuffer(writeBufFilledInNStep,wavPtr,environWav,YSTRUE);
			}

			int nWritten=snd_pcm_writei(handle,writeBuf,bufSizeInNStep);

			if(nWritten==-EAGAIN)
			{
			}
			else if(-EPIPE==nWritten || -EBADFD==nWritten)
			{
				snd_pcm_prepare(handle);
				snd_pcm_wait(handle,1);
				printf("ALSA: Recover from underrun\n");
			}
			else if(0>nWritten)
			{
				PrintState(-nWritten);
			}
			else if(0<nWritten)
			{
				if(NULL!=oneShotWav)
				{
					oneShotWavPtr+=nWritten;
					if(oneShotWav->NTimeStep()<=oneShotWavPtr)
					{
						nWritten=oneShotWavPtr-oneShotWav->NTimeStep();
						oneShotWav=NULL;
						oneShotWavPtr=0;
					}
					else
					{
						nWritten=0;
					}
				}
				if(NULL!=environWav)
				{
					environWavPtr+=nWritten;
					while(environWavPtr>=environWav->NTimeStep())
					{
						environWavPtr-=environWav->NTimeStep();
					}
				}
			}
		}
		else
		{
			snd_pcm_state_t state=snd_pcm_state(handle);
			if(SND_PCM_STATE_RUNNING!=state)
			{
				PrintState(0);
				snd_pcm_prepare(handle);
				snd_pcm_wait(handle,1);
				printf("ALSA: Recover from state\n");
			}
		}
	}
}

void YsAlsaPlayer::PopulateWriteBuffer(unsigned int &writeBufFilledInNStep,unsigned int &wavPtr,const YsWavFile *wavFile,YSBOOL loop)
{
	while(writeBufFilledInNStep<bufSizeInNStep)
	{
		const unsigned char *dataPtr=wavFile->DataPointerAtTimeStep(wavPtr);
		const int ptrInByte=wavPtr*bytePerTimeStep;
		const int wavByteLeft=wavFile->SizeInByte()-ptrInByte;
		const int writeBufLeftInByte=(bufSizeInNStep-writeBufFilledInNStep)*bytePerTimeStep;
		
		int nByteToWrite;
		if(wavByteLeft<writeBufLeftInByte)
		{
			nByteToWrite=wavByteLeft;
		}
		else
		{
			nByteToWrite=writeBufLeftInByte;
		}

		for(int i=0; i<nByteToWrite; i++)
		{
			writeBuf[writeBufFilledInNStep*bytePerTimeStep+i]=dataPtr[i];
		}
		writeBufFilledInNStep+=nByteToWrite/bytePerTimeStep;
		wavPtr+=nByteToWrite/bytePerTimeStep;
		if(wavFile->NTimeStep()<=wavPtr)
		{
			if(YSTRUE!=loop)
			{
				break;
			}
			wavPtr=0;
		}
	}
}

void YsAlsaPlayer::Stop(void)
{
	Initialize();
	if(NULL!=handle)
	{
		snd_pcm_drop(handle);
	}
}

void YsAlsaPlayer::PrintState(int errCode)
{
	if(0==errCode)
	{
	}
	else if(EBADFD==errCode)
	{
		printf("EBADFD\n");
	}
	else if(EPIPE==errCode)
	{
		printf("EPIPE\n");
	}
	else if(ESTRPIPE==errCode)
	{
		printf("ESTRPIPE\n");
	}
	else if(EAGAIN==errCode)
	{
		printf("EAGAIN\n");
	}
	else
	{
		printf("Unknown error.\n");
	}

	snd_pcm_state_t state=snd_pcm_state(handle);
	switch(state)
	{
	case SND_PCM_STATE_OPEN:
		printf("SND_PCM_STATE_OPEN\n");
		break;
	case SND_PCM_STATE_SETUP:
		printf("SND_PCM_STATE_SETUP\n");
		break;
	case SND_PCM_STATE_PREPARED:
		printf("SND_PCM_STATE_PREPARED\n");
		break;
	case SND_PCM_STATE_RUNNING:
		printf("SND_PCM_STATE_RUNNING\n");
		break;
	case SND_PCM_STATE_XRUN:
		printf("SND_PCM_STATE_XRUN\n");
		break;
	case SND_PCM_STATE_DRAINING:
		printf("SND_PCM_STATE_DRAINING\n");
		break;
	case SND_PCM_STATE_PAUSED:
		printf("SND_PCM_STATE_PAUSED\n");
		break;
	case SND_PCM_STATE_SUSPENDED:
		printf("SND_PCM_STATE_SUSPENDED\n");
		break;
	case SND_PCM_STATE_DISCONNECTED:
		printf("SND_PCM_STATE_DISCONNECTED\n");
		break;
	}
}
