echo off
forfiles /s /m *.glsl /c "%VK_SDK_PATH%\Bin\glslangValidator.exe glslangValidator.exe -V -o @fname.spv @file"
pause