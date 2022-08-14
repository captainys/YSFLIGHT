#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/soundcard.h>
#include <sys/poll.h>

#include <stdio.h>

#include <time.h>

#include "../fsairsoundenum.h"
#include "../yswavfile/yswavfile.h"
#include "yspulseaudio.h"


const YSBOOL Ysflight_SE_Signed=YSTRUE;
const int Ysflight_SE_PlayBackRate=22050;
const YSBOOL Ysflight_SE_Stereo=YSFALSE;
const int Ysflight_SE_BitPerSample=16;

YsPulseAudioContext::YsPulseAudioContext() : paMainloop(NULL), paContext(NULL)
{
	paMainloop=pa_mainloop_new();
	if(NULL!=paMainloop)
	{
		paContext=pa_context_new(
			pa_mainloop_get_api(paMainloop),
			"YSFLIGHT_PULSE_AUDIO_CONTEXT");

		if(NULL!=paContext)
		{
			pa_context_connect(
				paContext,
				NULL,
				(pa_context_flags_t)0,
				NULL);
			const time_t timeOut=5;

			time_t timeLimit=time(NULL)+timeOut;
			while(timeLimit>=time(NULL))
			{
				pa_mainloop_iterate(paMainloop,0,NULL);
				if(PA_CONTEXT_READY==pa_context_get_state(paContext))
				{
					printf("Pulse-Audio Context is ready to use.\n");
					return;
				}
			}
		}
	}
	printf("Error during setting up Pulse-Audio.\n");
	paMainloop=NULL;
	paContext=NULL;
}

YsPulseAudioContext::~YsPulseAudioContext()
{
	if(NULL!=paContext)
	{
		pa_context_disconnect(paContext);
		pa_context_unref(paContext);
	}
	if(NULL!=paMainloop)
	{
		pa_mainloop_free(paMainloop);
	}
}

void YsPulseAudioContext::KeepPlaying(void)
{
	if(NULL!=paMainloop)
	{
		while(0<pa_mainloop_iterate(paMainloop,0,NULL))
		{
		}
	}
}



YsPulseAudioWavPlayer::YsPulseAudioWavPlayer(
	YsPulseAudioContext *contextIn,
	const char idName[]) : 
	wavFile(NULL),wavPtr(0),isPlaying(YSFALSE),ysPaContext(NULL),paStream(NULL)
{
	if(NULL!=contextIn && NULL!=contextIn->paContext)
	{
		ysPaContext=contextIn;

		paSampleSpec.format=PA_SAMPLE_S16LE;
		paSampleSpec.rate=Ysflight_SE_PlayBackRate;
		paSampleSpec.channels=1;
		paStream=pa_stream_new(contextIn->paContext,idName,&paSampleSpec,NULL);

		if(NULL!=paStream)
		{
			pa_buffer_attr bufAttr=
			{
				65536, // Max length
				65536, // Target length of the buffer (playback only)
				4096,  // Pre-buffering (playback only)
				16384 , // Minimum request (playback only)
				16384, // Fragment size (recording only)
			};

			pa_stream_connect_playback(
				paStream,NULL,&bufAttr,(pa_stream_flags_t)0,NULL,NULL);
			return;
		}
	}
	printf("Error in Pulse-Audio Stream creation.\n");
}

YsPulseAudioWavPlayer::~YsPulseAudioWavPlayer()
{
	if(NULL!=paStream)
	{
		pa_stream_disconnect(paStream);
		pa_stream_unref(paStream);
	}
}

YSRESULT YsPulseAudioWavPlayer::Play(YsWavFile *wavFileIn)
{
	if(NULL!=paStream && (YSTRUE!=isPlaying || wavFileIn!=wavFile))
	{
		if(wavFileIn->IsSigned()!=Ysflight_SE_Signed)
		{
			wavFileIn->ConvertToSigned();
		}
		if(wavFileIn->BitPerSample()!=Ysflight_SE_BitPerSample)
		{
			wavFileIn->ConvertTo16Bit();
		}
		if(wavFileIn->Stereo()!=Ysflight_SE_Stereo)
		{
			wavFileIn->ConvertToMono();
		}
		if(wavFileIn->PlayBackRate()!=Ysflight_SE_PlayBackRate)
		{
			wavFileIn->Resample(Ysflight_SE_PlayBackRate);
		}

		pa_stream_flush(paStream,NULL,NULL);

		wavFile=wavFileIn;
		wavPtr=0;
		isPlaying=YSTRUE;
	}
}

YSRESULT YsPulseAudioWavPlayer::WriteData(YSBOOL loop)
{
	if(NULL!=wavFile && NULL!=paStream && YSTRUE==isPlaying)
	{
		if(PA_STREAM_READY==pa_stream_get_state(paStream))
		{
			const size_t writableSize=pa_stream_writable_size(paStream);
			const size_t sizeRemain=wavFile->SizeInByte()-wavPtr;
			const size_t writeSize=(sizeRemain<writableSize ? sizeRemain : writableSize);

			if(0<writeSize)
			{
				pa_stream_write(
					paStream,
					wavFile->DataPointer()+wavPtr,
					writeSize,
					NULL,
					0,
					PA_SEEK_RELATIVE);
				pa_stream_trigger(paStream,NULL,NULL);
				wavPtr+=writeSize;
			}

			if(YSTRUE==loop && wavFile->SizeInByte()<=wavPtr)
			{
				wavPtr=0;
			}
		}
		else
		{
			printf("Not ready.\n");
		}

		if(wavFile->SizeInByte()<=wavPtr &&
		   0<=pa_stream_get_underflow_index(paStream) &&
		   YSTRUE!=loop)
		{
			wavFile=NULL;
			isPlaying=YSFALSE;
		}
	}

	return YSOK;
}

YSRESULT YsPulseAudioWavPlayer::Stop(void)
{
	if(NULL!=paStream && YSTRUE==isPlaying)
	{
		pa_stream_flush(paStream,NULL,NULL);
		isPlaying=YSFALSE;
	}
	return YSOK;
}

