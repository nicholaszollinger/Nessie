// © 2021 NVIDIA Corporation

#if defined(_WIN32)

// Ensure windows is included at this point.
#if ((NRI_ENABLE_D3D11_SUPPORT == 0) && (NRI_ENABLE_D3D12_SUPPORT == 0))
    #include <windows.h>
#endif
#    undef LoadLibrary

const char* VULKAN_LOADER_NAME = "vulkan-1.dll";

Library* LoadSharedLibrary(const char* path) {
    return (Library*)LoadLibraryA(path);
}

void* GetSharedLibraryFunction(Library& library, const char* name) {
    return (void*)GetProcAddress((HMODULE)&library, name);
}

void UnloadSharedLibrary(Library& library) {
    FreeLibrary((HMODULE)&library);
}
#elif defined(__linux__) || defined(__APPLE__)
#    include <dlfcn.h>

#    if defined(__linux__)
const char* VULKAN_LOADER_NAME = "libvulkan.so";
#    elif defined(__APPLE__)
const char* VULKAN_LOADER_NAME = "libvulkan.dylib";
#    endif

Library* LoadSharedLibrary(const char* path) {
    return (Library*)dlopen(path, RTLD_NOW);
}

void* GetSharedLibraryFunction(Library& library, const char* name) {
    return dlsym((void*)&library, name);
}

void UnloadSharedLibrary(Library& library) {
    dlclose((void*)&library);
}
#else
#    error unknown platform
#endif