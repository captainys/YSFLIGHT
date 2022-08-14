#include <ysclass.h>
#include <fs.h>
#include <fswindow.h>


#include <windows.h>



// To deal with fucking infamous axis of evil known as WinMain

extern int main(int ac,char *av[]);

int PASCAL WinMain(HINSTANCE inst,HINSTANCE /*dumb*/,LPSTR param,int /*show*/)
{
	int ac;
	char *av[128],tmp[256],prog[260];

	strcpy(prog,"Unknown");
	GetModuleFileName(inst,prog,260);

	strncpy(tmp,param,256);
	av[0]=prog;
	YsArguments(&ac,av+1,126,tmp);
	return main(ac+1,av);
}
