echo off
call Source\Engine\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Engine\Tools\ProjectGenerator\premake5.lua" clean_all %~dp0
pause