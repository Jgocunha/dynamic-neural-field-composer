# To move to a newer imgui-platform-kit, change REF to its commit, set SHA512 to 0 and
# install once: vcpkg's error message prints the real SHA512 to paste here. Update
# "version-date" in vcpkg.json to that commit's date.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Jgocunha/imgui-platform-kit
    REF 27ada9152ea705e683de2b31c5986c536e5354e5
    SHA512 4b334ab7bcdaebf0729b8729e92f2e7203350fb2df6481c0feda5412e7a1a2c0fe934f11a9cd0600c71f78fd00b277173a8e2f835267673b542cb1569aec2b84
    HEAD_REF main
)

# The library exports no symbols, so it can only be linked statically.
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

# The kit's CMakeLists stops unless VCPKG_ROOT is set in the environment, and vcpkg
# clears that variable for port builds on Windows. Kit 2.1.0 and later accept vcpkg's
# toolchain instead, so this line can go once REF is at one of them.
set(ENV{VCPKG_ROOT} "${VCPKG_ROOT_DIR}")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/imgui-platform-kit"
    OPTIONS -DIMGUI_PLATFORM_KIT_BUILD_TESTS=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
