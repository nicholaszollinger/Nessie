echo off
call Source\Engine\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Engine\Tools\ProjectGenerator\premake5.lua" clean %~dp0
pause