#ifndef FSNETUTIL_IS_INCLUDED
#define FSNETUTIL_IS_INCLUDED
/* { */
////////////////////////////////////////////////////////////
inline void FsSetInt(unsigned char *dat,int a)
{
	unsigned int *b;
	b=(unsigned int *)(&a);
	dat[0]=(*b)&255;
	dat[1]=((*b)>>8)&255;
	dat[2]=((*b)>>16)&255;
	dat[3]=((*b)>>24)&255;
}

inline int FsGetInt(const unsigned char *dat)
{
	int *a;
	unsigned int b;

	b=dat[0]+(dat[1]<<8)+(dat[2]<<16)+(dat[3]<<24);

	a=(int *)(&b);

	return *a;
}

inline void FsSetUnsignedInt(unsigned char *dat,unsigned int a)
{
	dat[0]=a&255;
	dat[1]=(a>>8)&255;
	dat[2]=(a>>16)&255;
	dat[3]=(a>>24)&255;
}

inline unsigned int FsGetUnsignedInt(const unsigned char *dat)
{
	return dat[0]+(dat[1]<<8)+(dat[2]<<16)+(dat[3]<<24);
}

inline void FsSetShort(unsigned char *dat,short a)
{
	unsigned short *b;
	b=(unsigned short *)(&a);
	dat[0]=(*b)&255;
	dat[1]=((*b)>>8)&255;
}

inline short FsGetShort(const unsigned char *dat)
{
	short *a;
	unsigned short b;

	b=dat[0]+(dat[1]<<8);

	a=(short *)(&b);

	return *a;
}

inline void FsSetUnsignedShort(unsigned char *dat,unsigned short a)
{
	dat[0]=a&255;
	dat[1]=(a>>8)&255;
}

inline unsigned short FsGetUnsignedShort(const unsigned char *dat)
{
	return dat[0]+(dat[1]<<8);
}

inline void FsSetFloat(unsigned char *dat,float a)
{
// Assume Intel CPU
	float *b;
	b=(float *)(dat);
	(*b)=a;
}

inline float FsGetFloat(const unsigned char *dat)
{
	return *((float *)dat);
}

inline void FsSetChar(unsigned char *dat,char a)
{
	((char *)dat)[0]=a;
}

inline int FsGetChar(const unsigned char *dat)
{
	return ((char *)dat)[0];
}

////////////////////////////////////////////////////////////


inline void FsPushInt(unsigned char *&dat,int a)
{
	FsSetInt(dat,a);
	dat+=4;
}

inline int FsPopInt(const unsigned char *&dat)
{
	int a;
	a=FsGetInt(dat);
	dat+=4;
	return a;
}

inline void FsPushUnsignedInt(unsigned char *&dat,unsigned int a)
{
	FsSetUnsignedInt(dat,a);
	dat+=4;
}

inline unsigned int FsPopUnsignedInt(const unsigned char *&dat)
{
	unsigned int a;
	a=FsGetUnsignedInt(dat);
	dat+=4;
	return a;
}

inline void FsPushShort(unsigned char *&dat,short a)
{
	FsSetShort(dat,a);
	dat+=2;
}

inline short FsPopShort(const unsigned char *&dat)
{
	short a;
	a=FsGetShort(dat);
	dat+=2;
	return a;
}

inline void FsPushUnsignedShort(unsigned char *&dat,unsigned short a)
{
	FsSetUnsignedShort(dat,a);
	dat+=2;
}

inline unsigned short FsPopUnsignedShort(const unsigned char *&dat)
{
	unsigned short a;
	a=FsGetShort(dat);
	dat+=2;
	return a;
}

inline void FsPushFloat(unsigned char *&dat,float a)
{
	FsSetFloat(dat,a);
	dat+=4;
}

inline float FsPopFloat(const unsigned char *&dat)
{
	float x;
	x=FsGetFloat(dat);
	dat+=4;
	return x;
}

inline void FsPushChar(unsigned char *&dat,char a)
{
	((char *)dat)[0]=a;
	dat++;
}

inline int FsPopChar(const unsigned char *&dat)
{
	char a;
	a=((char *)dat)[0];
	dat++;
	return a;
}

inline void FsPushUnsignedChar(unsigned char *&dat,char a)
{
	dat[0]=a;
	dat++;
}

inline int FsPopUnsignedChar(const unsigned char *&dat)
{
	unsigned char a;
	a=dat[0];
	dat++;
	return a;
}

/* } */
#endif
