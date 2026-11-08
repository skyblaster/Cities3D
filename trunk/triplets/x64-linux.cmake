# Overlay for Homebrew-on-Linux / immutable builds. Same as vcpkg's default
# x64-linux triplet, plus a FindOpenGL hint for Mesa without GLVND.
#
# Homebrew's mesa formula ships a classic libGL.so that already contains GLX.
# It does not install a separate libGLX.so. CMake's default GLVND preference
# then leaves OPENGL_glx_LIBRARY as NOTFOUND, which breaks ports such as glew
# that link ${OPENGL_glx_LIBRARY} directly.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_CMAKE_CONFIGURE_OPTIONS -DOpenGL_GL_PREFERENCE=LEGACY)
