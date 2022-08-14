

#ifndef FSOPTION_IS_INCLUDED
#define FSOPTION_IS_INCLUDED
/* { */

class FsOption
{
private:
	static YsString languageStringOverrideFromCommandLine;
	static YsString languageStringOverrideFromOption;

public:
	static void SetLanguageStringOverrideFromCommandLine(const char langStr[]);
	static void SetLanguageStringOverrideFromOption(const char langStr[]);
	static YsString GetLanguageString(void);

	FsOption();
	void SetDefault();
	YSRESULT Load(const wchar_t fn[]);
	YSRESULT Save(const wchar_t fn[]);

	YSRESULT GetFullScreenResolution(int &x,int &y,int &depth) const;

	int scrnMode;
	YSBOOL rememberWindowSize;
	YSBOOL sound;
	YSBOOL openingDemo;
	YSBOOL useTaskBarIcon;
	YSBOOL alwaysOnTop;

	YSBOOL useMapPreview;

	enum LOCALIZATIONTYPE
	{
		FORCEENGLISH,
		AUTOMATIC,
		SPECIFYFILE
	} languageType;
	YsWString languageFile;

	YSBOOL backPicture;

	YsString fontName;
	int fontHeight;

	YSBOOL intelGPUSucks;
	YSBOOL myD3dDriverSucks;
};


/* } */
#endif
