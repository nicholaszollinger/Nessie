echo off
rem Updates the submodules as necessary.
call Source\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Tools\ProjectGenerator\premake5.lua" update_submodules "%~dp0
pause