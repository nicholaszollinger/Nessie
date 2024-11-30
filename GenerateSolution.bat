echo off
call Source\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Tools\ProjectGenerator\premake5.lua" vs2022 %~dp0
pause