# Nessie
This is a C++20 Game Engine that I am developing to learn and experiment with!

# Compatibility
**Supported Platforms**: *Windows*

**Supported IDEs**: *The build system was made to work with Microsoft Visual Studio 2022.*

**Vulkan**: *The engine requires Vulkan to be installed. Use the following link to install a compatible version:* [Link](https://sdk.lunarg.com/sdk/download/1.4.304.0/windows/VulkanSDK-1.4.304.0-Installer.exe)
* Leave everything default when installing.

**HardWare**: *Currently, I am only testing on my machine, which uses an NVIDIA 3090 GPU and AMD Ryzen 9 5950X CPU. I am working on checking for 
hardware limitations with certain features, but until then, I can't guarantee it will be compatible on other machines.*

# How to Build
Double click `GenerateSolution.bat` to generate the Visual Studio Projects and Solution!

`CleanProjectFolder.bat` is used to delete the `.sln` file, and the `Build`, `Intermediate`, `Saved`, `.vs` folders. Basically a convenience
utitlity for me if I have to zip up the project for someone else. 

# What I am Working On:
### Immediate:
Over the summer, I am going to be doing some project-wide refactors, updating syntax/naming rules and splitting the code-base into smaller projects so that I can more easily maintain the system.
- Break the engine into smaller projects
- New Syntax Rules
- Refactor Math classes with SIMD operations
- Refactor Logging & Profiling code to be more robust.
- Finish basic Physics System, with demo of different types of collisions

### Fall Goals:
- Port Asset System and Graphics work from my Graphics class assignments.
- Render Thread
- Asset Thread
- Polish ECS
- Engine vs Game Applications

### Next Year (Unless I can get to these sooner):
- Proper Serialization - utilizing a binary format.
- UI
- Sound
- Scripting, in python (lower priority, as I will be the solo dev for this engine).

# Branches
### Core
I am going to use this branch to perform the syntax and project structure refactors.

### Physics Branch
**This branch should not be used yet! This is currently under-development!*

I am working toward implementing essentially "Jolt Physics Lite" - a multithreaded, 3D physics simulation. I am trying to streamline the 
implementation as much as I can and use my own core systems to support, it (Debugging, Memory allocation, Math classes, etc). I am trying to recreate the library line by line as a learning exercise - I want to be working on fun 3D platforming games, so I want to get into the nitty-gritty of physics problems and solutions.
- [Jolt Physics Github](https://github.com/jrouwe/JoltPhysics)

*(Side note: It is cool to see that we reference the same textbook that I have been using this semester)*
