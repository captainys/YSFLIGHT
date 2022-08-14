#include <ysclass.h>


class Source
{
public:
	YsString tag;
	YsString utf8;
};


void main(void)
{
	FILE *ifp=NULL,*ofp=NULL,*rfp=NULL;
	YsArray <Source> src;

	ifp=fopen("japanese.txt","r");
	ofp=fopen("../runtime/language/ja.uitxt","w");
	rfp=fopen("../runtime/language/en.uitxt","r");

	if(NULL!=ifp && NULL!=rfp && NULL!=ofp)
	{
		YsString str;

		while(NULL!=str.Fgets(ifp))
		{
			if(0==strncmp(str,"language/",9))
			{
				YsString tag;
				int i;
				for(i=9; i<str.Strlen() && '#'!=str[i] && ' '!=str[i] && '.'!=str[i] ; i++)
				{
					tag.Append(str[i]);
				}

				if('.'==str[i])
				{
					for(i=i; i<str.Strlen() && '#'!=str[i]; i++)
					{
					}
					if('#'==str[i])
					{
						src.Increment();
						src.GetEnd().tag=tag;
						src.GetEnd().utf8.Set((const char *)str+i+1);
					}
				}
			}
		}

		while(NULL!=str.Fgets(rfp))
		{
			YSBOOL match=YSFALSE;
			if('+'==str[0])
			{
				YsString tag;
				int i;
				for(i=1; i<str.Strlen() && '#'!=str[i] && ' '!=str[i]; i++)
				{
					tag.Append(str[i]);
				}

				for(i=0; i<src.GetN(); i++)
				{
					if(0==strcmp(tag,src[i].tag))
					{
						int j;
						for(j=0; j<str.Strlen(); j++)
						{
							if('#'==str[j])
							{
								printf("Match! %s\n",tag.Txt());
								str.SetLength(j+1);
								str.Append(src[i].utf8);
								break;
							}
						}
						break;
					}
				}
			}

			fprintf(ofp,"%s\n",str.Txt());
		}
	}

	if(NULL!=ifp)
	{
		fclose(ifp);
	}
	if(NULL!=ofp)
	{
		fclose(ofp);
	}
	if(NULL!=rfp)
	{
		fclose(rfp);
	}
}
