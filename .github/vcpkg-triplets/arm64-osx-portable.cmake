# arm64-osx with a fixed minimum macOS version, used by the release workflow. Without
# VCPKG_OSX_DEPLOYMENT_TARGET every dependency is built for the runner's own macOS, and
# the release binaries refuse to start on anything older. 13.3 is the oldest macOS whose
# libc++ provides std::format for floating-point values, which the public headers use.
# Keep it equal to MACOSX_DEPLOYMENT_TARGET in .github/workflows/release.yml.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET 13.3)
