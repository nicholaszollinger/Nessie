echo off
rem Compiles slang shaders, with the vert entry point = "vertMain", and frag entry point = "fragMain"
forfiles /s /m *.slang /c "%VK_SDK_PATH%\Bin\slangc.exe slangc.exe -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vertMain -entry fragMain -o @fname.spv @file"