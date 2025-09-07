# Nessie
This is a C++20 Game Engine that I am developing to learn and experiment with!

# Compatibility
**Supported Platforms**: *Windows*

**Supported IDEs**: *The build system was made to work with Microsoft Visual Studio 2022.*

**Vulkan**: *The engine requires Vulkan to be installed. Use the following link and to install the compatible version:* [Link](https://sdk.lunarg.com/sdk/download/1.4.321.1/windows/vulkansdk-windows-X64-1.4.321.1.exe)

**HardWare**: *Currently, I am only testing on my machine, which uses an NVIDIA 3090 GPU and AMD Ryzen 9 5950X CPU. I am working on checking for 
hardware limitations with certain features, but until then, I can't guarantee it will be compatible on other machines.*

# How to Build:
Download the git repository and use the following build scripts:
- `1_Deploy.bat`: Initializes and/or updates the git submodules.
- `2_Build.bat`: Generates the solution files. From here you are good to go!
- `3_CleanBuild.bat`: Cleans up the Build and Intermediate folders and solution files. 
    - You don't have to re-deploy after cleaning; just use `2_Build.bat` to regenerate the solution.
- `4_CleanAll.bat`: Cleans the build just as above, but also deinitializes all git submodules. 
    - You will have to re-deploy (`1_Deploy.bat`) before building.

# What I am Working On:
### Immediate:
This Fall, the aim is finish rendering features and create the Editor UI!
- Renderer Refactor - Vulkan Abstraction ⌛
    - PBR Pipeline
    - Shadows
- ECS refactor - wrapping Entt.
- Editor UI - using ImGui.
- 3D Demo Scene showing off features!

### Next Year (Unless I can get to these sooner):
- Full Jolt Physics Integration
- Exporting a final executable.
- Proper Serialization - utilizing a binary format.
- UI System.
- Audio System.
- Scripting, in python (lower priority, as I will be the solo dev for this engine).