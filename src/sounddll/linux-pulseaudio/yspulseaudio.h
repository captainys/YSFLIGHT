#ifndef YSPULSEAUDIO_IS_INCLUDED
#define YSPULSEAUDIO_IS_INCLUDED
/* { */

#include <stdio.h>
#include <time.h>

#include <pulse/mainloop.h>
#include <pulse/context.h>
#include <pulse/stream.h>
#include <pulse/error.h>


class YsPulseAudioContext
{
public:
	pa_mainloop *paMainloop;
	pa_context *paContext;

	YsPulseAudioContext();
	~YsPulseAudioContext();
	void KeepPlaying(void);
};

class YsPulseAudioWavPlayer
{
private:	
	const class YsWavFile *wavFile;
	unsigned int wavPtr;
	YSBOOL isPlaying;

	YsPulseAudioContext *ysPaContext;
	pa_stream *paStream;
	pa_sample_spec paSampleSpec;

public:
	YsPulseAudioWavPlayer(
		YsPulseAudioContext *contextIn,
		const char idName[]);
	~YsPulseAudioWavPlayer();

	YSRESULT Play(YsWavFile *wavFile); // This may resample the incoming WAV.
	YSRESULT WriteData(YSBOOL loop);
	YSRESULT Stop(void);
};

/* } */
#endif
