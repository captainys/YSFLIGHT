SETLOCAL

cd %~dp0

SET PARTICLE_FILE=..\..\runtime\misc\particle01.png

c:\ysbin\build64\exe\Release\ysmakeblackwhite pattern1.png %PARTICLE_FILE% 128 128 128

c:\ysbin\build64\exe\Release\ysmakewhitetransparent %PARTICLE_FILE% %PARTICLE_FILE%

c:\ysbin\build64\exe\Release\ysnegatebmp %PARTICLE_FILE% %PARTICLE_FILE%



ENDLOCAL
