#ifndef YSWAVFILE_IS_INCLUDED
#define YSWAVFILE_IS_INCLUDED
/* { */

#ifndef YSRESULT_IS_DEFINED
#define YSRESULT_IS_DEFINED
/*! Enum for processing result. */
typedef enum
{
	YSERR,  /*!< There were error(s). */
	YSOK    /*!< The process was successful. */
} YSRESULT;
#endif

#ifndef YSBOOL_IS_DEFINED
#define YSBOOL_IS_DEFINED
/*! Enum for boolearn. */
typedef enum
{
	YSFALSE,     /*!< False */
	YSTRUE,      /*!< True */
	YSTFUNKNOWN  /*!< Unable to tell true or false. */
} YSBOOL;
#endif

class YsWavFile
{
private:
	YSBOOL stereo;
	unsigned int bit;
	unsigned int rate;
	unsigned int sizeInBytes;

	YSBOOL isSigned;
	unsigned char *dat;

public:
	YsWavFile();
	~YsWavFile();

	void Initialize(void);

	unsigned int NTimeStep(void) const;

	YSBOOL Stereo(void) const;
	unsigned int BytePerTimeStep(void) const;
	unsigned int BitPerSample(void) const;
	unsigned int BytePerSample(void) const;
	unsigned int PlayBackRate(void) const;
	unsigned int SizeInByte(void) const;
	YSBOOL IsSigned(void) const;
	const unsigned char *DataPointer(void) const;
	const unsigned char *DataPointerAtTimeStep(unsigned int ts) const;

	YSRESULT LoadWav(const char fn[]);
	YSRESULT ConvertTo16Bit(void);
	YSRESULT ConvertTo8Bit(void);
	YSRESULT ConvertToStereo(void);
	YSRESULT ConvertToMono(void);
	YSRESULT Resample(int newRate);

	YSRESULT ConvertToSigned(void);
	YSRESULT ConvertToUnsigned(void);

private:
	int GetNumChannel(void) const;
	int GetNumSample(void) const;
	int GetNumSamplePerChannel(void) const;
	size_t GetUnitSize(void) const;
	size_t GetSamplePosition(int atIndex) const;
	int GetSignedValue(int atTimeStep,int channel) const;
	void SetSignedValue(unsigned char *savePtr,int rawSignedValue);
};


/* } */
#endif
