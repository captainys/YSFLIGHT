#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
#include <fssimplewindow.h>

#include <ysclass.h>
#include <ysport.h>
#include <fsgui.h>

#include "fsoption.h"



/*static*/ YsString FsOption::languageStringOverrideFromCommandLine;
/*static*/ YsString FsOption::languageStringOverrideFromOption;

////////////////////////////////////////////////////////////

/* static */ void FsOption::SetLanguageStringOverrideFromCommandLine(const char langStr[])
{
	languageStringOverrideFromCommandLine=langStr;
}
/* static */ void FsOption::SetLanguageStringOverrideFromOption(const char langStr[])
{
	languageStringOverrideFromOption=langStr;
}

/* static */ YsString FsOption::GetLanguageString(void)
{
	if(0<languageStringOverrideFromCommandLine.Strlen())
	{
		return languageStringOverrideFromCommandLine;
	}
	else if(0<languageStringOverrideFromOption.Strlen())
	{
		return languageStringOverrideFromOption;
	}
	else
	{
		YsLocale locale;
		return locale.GetLanguagePart();
	}
}

////////////////////////////////////////////////////////////

static const char *const optCmdBasic[]=
{
	"REM",
	"SCREENMOD",
	"SOUND",
	"OPENDEMO",
	"RMBRWSIZE",
	"USETASKBAR",
	"TOPMOSTWIN",
	"FONTNAME",
	"FONTHEIGHT",
	"USEMAPPREVIEW",
	"BACKPICTURE",
	"USELANGUAGEBMP",
	"D3DXSPRITEISSUE",
	"LANGUAGE",
	"DIRECT3DSUCKS",
	NULL
};

FsOption::FsOption()
{
	SetDefault();
}

void FsOption::SetDefault()
{
	scrnMode=0;
	sound=YSTRUE;
	openingDemo=YSTRUE;
	rememberWindowSize=YSTRUE;
	useTaskBarIcon=YSTRUE;
	alwaysOnTop=YSFALSE;
	useMapPreview=YSTRUE;

	backPicture=YSTRUE;

	intelGPUSucks=YSFALSE;
	myD3dDriverSucks=YSFALSE;

	fontName.Set("");
	fontHeight=12;

	languageType=AUTOMATIC;
}

YSRESULT FsOption::Load(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"r");
	if(fp!=NULL)
	{
		char buf[256],cmd[256];
		int ac;
		char *av[32];

		SetDefault();

		while(fgets(cmd,255,fp)!=NULL)
		{
			strncpy(buf,cmd,255);
			buf[255]=0;
			if(YsArguments(&ac,av,32,buf)==YSOK && ac>0)
			{
				int cmd;
				if(YsCommandNumber(&cmd,av[0],optCmdBasic)==YSOK)
				{
					switch(cmd)
					{
					case 0: //"REM"
						break;
					case 1: //"SCREENMOD",
						scrnMode=atoi(av[1]);
						break;
					case 2: //"SOUND"
						if(strcmp(av[1],"TRUE")==0)
						{
							sound=YSTRUE;
						}
						else
						{
							sound=YSFALSE;
						}
						break;
					case 3: //"OPENDEMO"
						if(strcmp(av[1],"TRUE")==0)
						{
							openingDemo=YSTRUE;
						}
						else
						{
							openingDemo=YSFALSE;
						}
						break;
					case 4: //"RMBRWSIZE"
						rememberWindowSize=YsStrToBool(av[1]);
						break;
					case 5: //"USETASKBAR",
						useTaskBarIcon=YsStrToBool(av[1]);
						break;
					case 6: // "TOPMOSTWIN",
						alwaysOnTop=YsStrToBool(av[1]);;
						break;
					case 7: //	"FONTNAME",
						fontName.Set(av[1]);
						break;
					case 8: //	"FONTHEIGHT",
						fontHeight=atoi(av[1]);
						break;
					case 9: //	"USEMAPPREVIEW",
						useMapPreview=YsStrToBool(av[1]);
						break;
					case 10: // 	"BACKPICTURE",
						backPicture=YsStrToBool(av[1]);
						break;
					case 11:
						// Deprecated
						break;
					case 12: // "D3DXSPRITEISSUE"
						intelGPUSucks=YsStrToBool(av[1]);
						break;
					case 13: //"LANGUAGE"
						if(0==strcmp(av[1],"AUTO"))
						{
							languageType=AUTOMATIC;
						}
						else if(0==strcmp(av[1],"ENGLISH"))
						{
							languageType=FORCEENGLISH;
						}
						else if(0==strncmp(av[1],"FILE:",5))
						{
							languageType=SPECIFYFILE;
							languageFile.SetUTF8String(av[1]+5);
						}
						break;
					case 14: // "DIRECT3DSUCKS"
						myD3dDriverSucks=YsStrToBool(av[1]);
						break;
					}
				}
				else
				{
					// Error... So, I just ignore it.
				}
			}
		}
		fclose(fp);
		return YSOK;
	}
	SetDefault();
	return YSERR;
}

YSRESULT FsOption::Save(const wchar_t fn[])
{
	FILE *fp=YsFileIO::Fopen(fn,"w");
	if(fp!=NULL)
	{
		fprintf(fp,"SCREENMOD %d\n",scrnMode);
		fprintf(fp,"RMBRWSIZE %s\n",YsBoolToStr(rememberWindowSize));
		fprintf(fp,"SOUND %s\n",YsBoolToStr(sound));
		fprintf(fp,"OPENDEMO %s\n",YsBoolToStr(openingDemo));
		fprintf(fp,"USETASKBAR %s\n",YsBoolToStr(useTaskBarIcon));
		fprintf(fp,"TOPMOSTWIN %s\n",YsBoolToStr(alwaysOnTop));
		fprintf(fp,"FONTNAME \"%s\"\n",(const char *)fontName);
		fprintf(fp,"FONTHEIGHT %d\n",fontHeight);
		fprintf(fp,"USEMAPPREVIEW %s\n",YsBoolToStr(useMapPreview));
		fprintf(fp,"BACKPICTURE %s\n",YsBoolToStr(backPicture));

		fprintf(fp,"D3DXSPRITEISSUE %s\n",YsBoolToStr(intelGPUSucks));
		fprintf(fp,"DIRECT3DSUCKS %s\n",YsBoolToStr(myD3dDriverSucks));

		switch(languageType)
		{
		case FORCEENGLISH:
			fprintf(fp,"LANGUAGE ENGLISH\n");
			break;
		case AUTOMATIC:
			fprintf(fp,"LANGUAGE AUTO\n");
			break;
		case SPECIFYFILE:
			{
				YsString utf8;
				utf8.EncodeUTF8 <wchar_t> (languageFile);
				fprintf(fp,"LANGUAGE \"FILE:%s\"\n",utf8.Txt());
			}
			break;
		}

		fclose(fp);
		return YSOK;
	}

	if(fp!=NULL)
	{
		fclose(fp);
	}
	// fsStderr.Printf("Cannot Write Option File.");
	return YSERR;
}

YSRESULT FsOption::GetFullScreenResolution(int &x,int &y,int &depth) const
{
	struct FsResDepthTable {int x,y,depth;} res[]=
	{
		{0,0,0},
		{0,0,0},
		{400,300,16},
		{512,384,16},
		{640,480,16},
		{800,600,16},
		{1024,768,16},
		{1152,864,16},
		{1200,800,16},
		{1280,1024,16},
		{1600,1200,16},
		{1920,1200,16},
	};
	int nRes=sizeof(res)/sizeof(struct FsResDepthTable);

	if(0<=scrnMode && scrnMode<nRes)
	{
		x=res[scrnMode].x;
		y=res[scrnMode].y;
		depth=res[scrnMode].depth;
		return YSOK;
	}
	return YSERR;
}

////////////////////////////////////////////////////////////

