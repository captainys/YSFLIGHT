SET DIR=c:\ysshare\src
SET YSFLIGHTSRCDIR=%DIR%\ysflight
SET YSCLASSDIR=%DIR%\ysclass
SET RUNTIMEDIR=%DIR%\runtime

SET YSBITMAPDIR=%DIR%\ysbitmap
SET SCNEDITDIR=%DIR%\sceneryeditor
SET YSWIN32DIR=%DIR%\yswin32
SET FSGUIDIR=%DIR%\fsguilib

SET YSJOYSTICKDIR=%DIR%\ysjoystick


rd /S /Q %YSFLIGHTSRCDIR%
rd /S /Q %YSCLASSDIR%
rd /S /Q %RUNTIMEDIR%
rd /S /Q %YSBITMAPDIR%
rd /S /Q %SCNEDITDIR%
rd /S /Q %YSWIN32DIR%
rd /S /Q %FSGUIDIR%
rd /S /Q %YSJOYSTICKDIR%

md %RUNTIMEDIR%

md %YSFLIGHTSRCDIR%
md %YSCLASSDIR%
md %YSBITMAPDIR%
md %SCNEDITDIR%
md %YSWIN32DIR%
md %FSGUIDIR%
md %YSJOYSTICKDIR%

md %YSFLIGHTSRCDIR%\src
md %YSCLASSDIR%\src
md %YSBITMAPDIR%\src
md %SCNEDITDIR%\src
md %YSWIN32DIR%\src
md %FSGUIDIR%\src
md %YSJOYSTICKDIR%\src



xcopy /S /H \src\ysflight\src\*.* %YSFLIGHTSRCDIR%\src
xcopy /S /H \src\ysclass\src\*.* %YSCLASSDIR%\src
xcopy /S /H \src\ysbitmap\src\*.* %YSBITMAPDIR%\src
xcopy /S /H \src\sceneryeditor\src\*.* %SCNEDITDIR%\src
xcopy /S /H \src\yswin32\src\*.* %YSWIN32DIR%\src
xcopy /S /H \src\fsguilib\src\*.* %FSGUIDIR%\src
xcopy /S /H \src\ysjoystick\src\*.* %YSJOYSTICKDIR%\src

xcopy /S /H ..\..\runtime\*.* %RUNTIMEDIR%
del %RUNTIMEDIR%\200*
del %RUNTIMEDIR%\*.exe
del %RUNTIMEDIR%\bundle\*.exe
del %RUNTIMEDIR%\plugin\*.dll
del %RUNTIMEDIR%\ysinstaller.def
rd %RUNTIMEDIR%\bundle
