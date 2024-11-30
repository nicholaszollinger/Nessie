echo off
call Source\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Tools\ProjectGenerator\premake5.lua" clean_all %~dp0
pause