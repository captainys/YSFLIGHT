SET DIR=k:\macosx
SET YSFLIGHTSRCDIR=%DIR%\ysflight
SET YSCLASSDIR=%DIR%\ysclass
SET RUNTIMEDIR=%DIR%\ysflight.app


SET YSBITMAPDIR=%DIR%\ysbitmap
SET SCNEDITDIR=%DIR%\sceneryeditor
SET FSGUIDIR=%DIR%\fsguilib

SET YSJOYSTICKDIR=%DIR%\ysjoystick

SET ZIPFILE=%DIR%\trans.zip


del %ZIPFILE%

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
md %FSGUIDIR%
md %YSJOYSTICKDIR%

md %YSFLIGHTSRCDIR%\src
md %YSCLASSDIR%\src
md %YSBITMAPDIR%\src
md %SCNEDITDIR%\src
md %FSGUIDIR%\src
md %YSJOYSTICKDIR%\src
md %YSJOYSTICKDIR%\MacOSX




xcopy /S /H \src\ysflight\src\*.* %YSFLIGHTSRCDIR%\src
xcopy /S /H \src\ysclass\src\*.* %YSCLASSDIR%\src
xcopy /S /H \src\ysbitmap\src\*.* %YSBITMAPDIR%\src
xcopy /S /H \src\sceneryeditor\src\*.* %SCNEDITDIR%\src
xcopy /S /H \src\fsguilib\src\*.* %FSGUIDIR%\src

xcopy /S /H \src\ysjoystick\src\*.* %YSJOYSTICKDIR%\src
xcopy /S /H \src\ysjoystick\MacOSX\*.* %YSJOYSTICKDIR%\MacOSX



xcopy /S ..\..\runtime\*.* %RUNTIMEDIR%
del %RUNTIMEDIR%\200*
del %RUNTIMEDIR%\*.exe
del %RUNTIMEDIR%\plugin\*.dll
del %RUNTIMEDIR%\plugin\*.exe
del %RUNTIMEDIR%\bundle\*.exe
del %RUNTIMEDIR%\ysinstaller.def
rd %RUNTIMEDIR%\bundle

mkdir %RUNTIMEDIR%\Contents
copy Info.plist %RUNTIMEDIR%\Contents

mkdir %RUNTIMEDIR%\Contents\Resources
copy APPL.icns %RUNTIMEDIR%\Contents\Resources


"C:\Program Files\WinZip\wzzip.exe" -u -p -r -x@C:\exe\wzexclude.lst %ZIPFILE% %DIR%\*.*

