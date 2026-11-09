# Linux triplet for Cities3D: keep most ports static, but build wxWidgets
# shared. Cities3D loads DatabaseCore/UICore/plugins as separate .so files; a
# static libwx gets linked into each of them (and into the main binary). That
# duplicates wx global state and SEGVs on the first FilePath/wx call from a
# core .so after wxApp has already started in the executable.
#
# Flatpak builds also apply overlays/flatpak-system-gtk so shared libwx links
# against the Freedesktop SDK GTK/GLib instead of embedding vcpkg's static
# copies (which abort at startup with dual-GLib g_type_register_static).
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

if(PORT STREQUAL "wxwidgets")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
    # wx 3.1.5+ defaults wxGLCanvas to EGL when libEGL is present. vcpkg's GLEW
    # is built for GLX only, so glewInit then returns GLEW_ERROR_NO_GLX_DISPLAY
    # ("No OpenGL extensions are available on this hardware") even though the
    # board still draws. Cities3D Flatpak forces GDK_BACKEND=x11, so keep the
    # GLX canvas and match GLEW.
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DwxUSE_GLCANVAS_EGL=OFF")
endif()
