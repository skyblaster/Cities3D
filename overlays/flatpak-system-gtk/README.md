# Flatpak system GTK/Cairo overlays

Empty vcpkg ports so `wxwidgets` links against the Freedesktop SDK’s GTK3/Cairo
(and their GLib) instead of embedding vcpkg’s static copies into every
`libwx_*.so`.

Without this, shared wx + static GLib aborts at process load:

```text
GLib-GObject:ERROR:.../gtype.c:...:g_type_register_static: assertion failed: (static_quark_type_flags)
```
