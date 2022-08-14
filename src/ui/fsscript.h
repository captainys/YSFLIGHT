#ifndef FSSCRIPT_IS_INCLUDED
#define FSSCRIPT_IS_INCLUDED
/* { */

#include <ysclass.h>
#include <memory>
#include <chrono>

#include "fsrunloop.h"

class FsScript
{
private:
	YsArray <YsString> prog;
	YSSIZE_T programCounter;
	std::chrono::time_point <std::chrono::system_clock> waitTimer;
	std::shared_ptr <FsRunLoop> runLoopPtr;
	std::shared_ptr <FsRunLoop::GuiCanvasBase> canvasPtr;

public:
	FsScript();
	void SetYSFLIGHT(std::shared_ptr <FsRunLoop> rl,std::shared_ptr <FsRunLoop::GuiCanvasBase> cv);
	void AddLine(const YsString &str);
	void RunOneStep(void);
};


/* } */
#endif
