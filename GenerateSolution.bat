echo off
call Source\Engine\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Engine\Tools\ProjectGenerator\premake5.lua" vs2022 %~dp0
pause