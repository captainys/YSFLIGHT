#include <stdio.h>
#include <conio.h>

#ifdef WIN32
#include <windows.h>
#endif


// http://en.wikipedia.org/wiki/ANSI_escape_code


void ClearScreen(void)
{
#ifdef WIN32
	HANDLE hCon;
	COORD pos;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD a;

	pos.X=0;
	pos.Y=0;

	hCon=GetStdHandle(STD_OUTPUT_HANDLE);

	GetConsoleScreenBufferInfo(hCon,&csbi);

	FillConsoleOutputCharacter(hCon,(TCHAR)' ',csbi.dwSize.X*csbi.dwSize.Y,pos,&a);
	FillConsoleOutputAttribute(hCon,csbi.wAttributes,csbi.dwSize.X*csbi.dwSize.Y,pos,&a);

	SetConsoleCursorPosition(hCon,pos);

#else
	printf("\x1b[2J");
	printf("\x1b[0;0H");
#endif
}



int main(void)
{
	ClearScreen();
}

