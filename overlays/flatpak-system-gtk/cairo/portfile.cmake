# Provide no files — dependents must use pkg-config / system libraries from
# the Flatpak Freedesktop SDK (or host). Shipping vcpkg's static GLib/GTK
# inside every libwx_*.so causes dual-GLib aborts at process start:
#   GLib-GObject: g_type_register_static: assertion failed: (static_quark_type_flags)
set(VCPKG_POLICY_EMPTY_PACKAGE enabled)
