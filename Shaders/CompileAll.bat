echo off
set ExitCode=0;

call "%~dp0CompileGLSL.bat"
if errorlevel 1 goto Error_CompileGLSL

call "%~dp0CompileSlang.bat"
if errorlevel 1 goto Error_CompileSlang

rem Success:
pause
EXIT /B 0

:Error_CompileGLSL
echo on
echo ERROR: Failed to compile GLSL shaders!
set ExitCode=999
pause
EXIT /B !ExitCode!

:Error_CompileSlang
echo on
echo ERROR: Failed to compile Slang shaders!
set ExitCode=999
pause
EXIT /B !ExitCode!