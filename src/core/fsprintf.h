#ifndef FSPRINTF_IS_INCLUDED
#define FSPRINTF_IS_INCLUDED
/* { */

template <class StrClass>
YSRESULT YsItoA(StrClass &str,int n)
{
	if(0==n)
	{
		str.SetLength(0);
		str.Append('0');
		return YSOK;
	}

	StrClass reverse;
	const int sgn=(n>=0 ? 1 : -1);
	if(n<0)
	{
		n=-n;
	}

	while(0<n)
	{
		reverse.Append('0'+n%10);
		n/=10;
	}

	str.SetLength(0);
	if(sgn<0)
	{
		str.Append('-');
	}

	for(int i=(int)reverse.Strlen()-1; i>=0; i--)
	{
		str.Append(reverse[i]);
	}

	return YSOK;
}

template <class StrClass>
YSRESULT YsFormatFirstInteger(StrClass &str,int n)
{
	StrClass numberStr;
	YsItoA(numberStr,n);

	for(int i=0; i<=str.Strlen()-2; i++)
	{
		if(str[i]=='%' && str[i+1]=='d')
		{
			StrClass newStr;

			newStr.Set(i,str.Txt());
			newStr.Append(numberStr);
			newStr.Append(str.Txt()+i+2);

			str=newStr;
			return YSOK;
		}
	}

	return YSERR;
}

template <class StrClass,class CHARTYPE>
YSRESULT YsFormatFirstString(StrClass &str,const CHARTYPE repl[])
{
	for(int i=0; i<=str.Strlen()-2; i++)
	{
		if(str[i]=='%' && str[i+1]=='s')
		{
			StrClass newStr;

			newStr.Set(i,str.Txt());
			if(NULL!=repl)
			{
				for(int j=0; 0!=repl[j]; ++j)
				{
					newStr.Append(repl[j]);
				}
			}
			newStr.Append(str.Txt()+i+2);

			str=newStr;
			return YSOK;
		}
	}

	return YSERR;
}


/* } */
#endif
