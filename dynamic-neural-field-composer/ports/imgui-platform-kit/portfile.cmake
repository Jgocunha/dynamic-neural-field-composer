# To move to a newer imgui-platform-kit, change REF to its commit, set SHA512 to 0 and
# install once: vcpkg's error message prints the real SHA512 to paste here. Update
# "version-date" in vcpkg.json to that commit's date.
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Jgocunha/imgui-platform-kit
    REF 7e079db016b7328f5db4b5f4ad10996949afccd7
    SHA512 d86085d0fa77d7099ae897b60b1241c29547cf14684190dd75c652a19f373be2b38e85ca5cef0d42feb6763b1c25cef5dd4f90dca7f07e9377fc15bd58d7b488
    HEAD_REF main
)

# The library exports no symbols, so it can only be linked statically.
vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}/imgui-platform-kit"
    OPTIONS -DIMGUI_PLATFORM_KIT_BUILD_TESTS=OFF
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include" "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
