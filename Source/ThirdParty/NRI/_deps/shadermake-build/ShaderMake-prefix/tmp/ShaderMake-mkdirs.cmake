# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-src/ShaderMake")
  file(MAKE_DIRECTORY "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-src/ShaderMake")
endif()
file(MAKE_DIRECTORY
  "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/build"
  "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix"
  "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix/tmp"
  "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix/src/ShaderMake-stamp"
  "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix/src"
  "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix/src/ShaderMake-stamp"
)

set(configSubDirs Debug;Release;MinSizeRel;RelWithDebInfo)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix/src/ShaderMake-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "M:/Dev/ThirdPartyLibraries/NRI/NRI/_Build/_deps/shadermake-build/ShaderMake-prefix/src/ShaderMake-stamp${cfgdir}") # cfgdir has leading slash
endif()
