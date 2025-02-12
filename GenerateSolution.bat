echo off
rem For the Solution Directory (%~dp0) that is passed in, I am only putting a quote in front of it because when reading the
rem value in premake, it was adding an extra quote at the end. This solution works!
call Source\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Tools\ProjectGenerator\premake5.lua" vs2022 "%~dp0
pause