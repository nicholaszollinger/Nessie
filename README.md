# Nessie
This is a C++20 Game Engine that I am developing to learn and experiment with!

# Physics Branch
**This branch should not be used yet! This is currently under-development!*
I am working toward implementing essentially "Jolt Physics Lite" - a multithreaded, 3D physics simulation. I am trying to streamline the 
implementation as much as I can and use my own core systems to support it (Debugging, Memory allocation, Math classes, etc).
[Jolt Physics Github](https://github.com/jrouwe/JoltPhysics)

*(Side note: It is cool to see that we reference the same textbook that I have been using this semester)*

# Compatibility
**Supported Platforms**: *Windows*

**Supported IDEs**: *The build system was made to work with Microsoft Visual Studio 2022.*

**Vulkan**: *The engine requires Vulkan to be installed. Use the following link to install a compatible version:* [Link](https://sdk.lunarg.com/sdk/download/1.4.304.0/windows/VulkanSDK-1.4.304.0-Installer.exe)
* Leave everything default when installing.

# How to Build
Double click `GenerateSolution.bat` to generate the Visual Studio Projects and Solution!

`CleanProjectFolder.bat` is used to delete the `.sln` file, and the `Build`, `Intermediate`, `Saved`, `.vs` folders. Basically a convenience
utitlity for me if I have to zip up the project for someone else. 