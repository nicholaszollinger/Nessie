echo off
rem Clean the project folder and removes all git submodule downloads.
call Source\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Tools\ProjectGenerator\premake5.lua" clean_all "%~dp0
pause