# Linux triplet for Cities3D: keep most ports static, but build wxWidgets
# shared. Cities3D loads DatabaseCore/UICore/plugins as separate .so files; a
# static libwx gets linked into each of them (and into the main binary). That
# duplicates wx global state and SEGVs on the first FilePath/wx call from a
# core .so after wxApp has already started in the executable.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

if(PORT STREQUAL "wxwidgets")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
endif()
