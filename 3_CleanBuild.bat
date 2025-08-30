echo off
rem Clean the project folder, deleting the Build, Saved, Intermediate, and solution file.
call Source\Tools\ProjectGenerator\Premake\premake5.exe --file="Source\Tools\ProjectGenerator\premake5.lua" clean_solution "%~dp0
pause