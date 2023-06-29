#ifndef FSUTIL_IS_INCLUDED
#define FSUTIL_IS_INCLUDED
/* { */

#include "fsdef.h"

YSBOOL FsTestKeyWordMatch(const YsString &str,YSSIZE_T ac,const YsString av[]);

double FsNumerical1stDifferentiation(const double &t1,const double &v1,const double &t2,const double &v2,const double &t3,const double &v3,const double &t);
double FsNumerical2ndDifferentiation(const double &t1,const double &v1,const double &t2,const double &v2,const double &t3,const double &v3,const double &t);

int FsGetRandomBetween(int min, int max);


double FsGetLevelTurnRadius(const double v,const double bank);


void FsCompressNumberInString(char str[]);


const char *FsTrueFalseString(YSBOOL tf);


void FsGetTodayFileString(YsString &str);
void FsGetTodayTimeString(YsString &str);


class FsAutoCloseFile
{
public:
	FsAutoCloseFile();
	~FsAutoCloseFile();

	FILE *Fp();
	FILE *Fopen(const wchar_t fn[],const char mode[]);
	void Fclose(void);

protected:
	FILE *filePtr;
};


class FsInternationalText
{
private:
	YsString languageCode;
	YsWString msg;
public:
	void CleanUp(void);
	void SetText(const char lang[],const wchar_t msg[]);
	YSBOOL MatchLanguage(const char lang[]) const;
	const char *GetLanguageCode(void) const;
	const wchar_t *GetText(void) const;
};


unsigned int FsGetDate(void);

const char *FsRegionIdToString(int id);

YSRESULT FsGetJoulePerSecond(double &dat,const char in[]);
YSRESULT FsGetLength(double &dat,const char in[]);
YSRESULT FsGetArea(double &dat,const char in[]);
YSRESULT FsGetWeight(double &dat,const char in[]);
YSRESULT FsGetForce(double &dat,const char in[]);
YSRESULT FsGetSpeed(double &dat,const char in[]);
YSRESULT FsGetAngle(double &dat,const char in[]);
YSRESULT FsGetBool(YSBOOL &dat,const char in[]);
YSRESULT FsGetNonDimensional(double &dat,const char in[]);
YSRESULT FsGetVec3(YsVec3 &vec,YSSIZE_T ac,const char * const av[]);
YSRESULT FsGetVec3(YsVec3 &vec,YSSIZE_T ac,const YsString av[]);
YSRESULT FsGetAtt3(YsAtt3 &att,YSSIZE_T ac,const char * const av[]);
YSRESULT FsGetAtt3(YsAtt3 &att,YSSIZE_T ac,const YsString av[]);
YSRESULT FsGetWeaponOfChoice(FSWEAPONTYPE &woc,const char str[]);
YSRESULT FsGetJoulePerSecond(double &power,const char in[]);

/* } */
#endif
