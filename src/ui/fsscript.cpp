#include "fsscript.h"



FsScript::FsScript()
{
	waitTimer=std::chrono::system_clock::now();
	programCounter=0;
}

void FsScript::SetYSFLIGHT(std::shared_ptr <FsRunLoop> rl,std::shared_ptr <FsRunLoop::GuiCanvasBase> cv)
{
	runLoopPtr=rl;
	canvasPtr=cv;
}

void FsScript::AddLine(const YsString &str)
{
	auto cleanStr=str;
	cleanStr.DeleteTailSpace();
	cleanStr.DeleteHeadSpace();
	prog.push_back(cleanStr);
}

void FsScript::RunOneStep(void)
{
	if(std::chrono::system_clock::now()<waitTimer)
	{
		return;
	}

	if(programCounter<prog.size())
	{
		const auto &testCmd=prog[programCounter];
		auto argv=testCmd.Argv();

		printf("Script>>%s\n",testCmd.c_str());
		if(0==argv.size())
		{
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("GUI:") && nullptr!=canvasPtr)
		{
			runLoopPtr->ProcessGuiCommand(testCmd.data()+4);
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("FILE:"))
		{
			runLoopPtr->ProcessFileCommand(testCmd.data()+5);
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("RAWKEY:"))
		{
			auto fsKey=FsStringToKeyCode(testCmd.data()+7);
			if(FSKEY_NULL!=fsKey)
			{
				FsPushKey(fsKey);
			}
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("FLIGHTCMD:"))
		{
			runLoopPtr->ProcessFlightCommand(testCmd.data()+10);
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("BTNFUNC:"))
		{
			runLoopPtr->ProcessButtonFunction(testCmd.data()+8);
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("WAITFOR:"))
		{
			if(0==testCmd.Subset(8).STRCMP("FLYING") &&
			   YSTRUE==runLoopPtr->Flying())
			{
				++programCounter;
			}
			else if(0==testCmd.Subset(8).STRCMP("NOT_FLYING") &&
			   YSTRUE!=runLoopPtr->Flying())
			{
				++programCounter;
			}
			else if(0==testCmd.Subset(8).STRCMP("REPLAY") &&
			        runLoopPtr->GetCurrentRunMode()==FsRunLoop::YSRUNMODE_REPLAYRECORD)
			{
				++programCounter;
			}
			else if(0==testCmd.Subset(8).STRCMP("GUI") &&
			        runLoopPtr->GetCurrentRunMode()==FsRunLoop::YSRUNMODE_MENU)
			{
				++programCounter;
			}
			else if(0==testCmd.Subset(8).STRCMP("MODAL") &&
			        nullptr!=canvasPtr->GetActiveModalDialog())
			{
				++programCounter;
			}
			else if(0==testCmd.Subset(8).STRCMP("NO_MODAL") &&
			        nullptr==canvasPtr->GetActiveModalDialog())
			{
				++programCounter;
			}
			else if(0==testCmd.Subset(8).STRCMP("NET_READY") &&
			        YSTRUE==runLoopPtr->NetReady())
			{
				++programCounter;
			}
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("SLEEP:"))
		{
			const int ms=atoi(testCmd.c_str()+6);
			waitTimer=std::chrono::system_clock::now()+std::chrono::milliseconds(ms);
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("ASSERT:"))
		{
			runLoopPtr->ProcessAssertCommand(testCmd.c_str()+7);
			++programCounter;
		}
		else if(YSTRUE==testCmd.DOESSTARTWITH("REM") || YSTRUE==testCmd.DOESSTARTWITH("#"))
		{
			++programCounter;
		}
		else
		{
			fprintf(stderr,"SCRIPT ERROR!\n");
			fprintf(stderr,"  %s\n",testCmd.c_str());
			exit(1);
		}
	}
}
