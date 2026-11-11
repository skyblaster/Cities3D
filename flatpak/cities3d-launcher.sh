#!/bin/sh
# Cities3D resolves plugins/tex/skins relative to cwd and next to the executable.
# /app is read-only in Flatpak, so:
#   - run with cwd in a writable XDG data dir (symlinked game assets)
#   - exec the real binary under /app (do not symlink it; /proc/self/exe would
#     otherwise resolve back into read-only /app for path probes)
# Crash breadcrumbs now go to FilePath::UserDataDir() in C++, not cwd.
#
# wxStandardPaths::GetUserDataDir() is ~/.Cities3D on Linux (and Flatpak
# --persist=.Cities3D). Keep DATA_ROOT on that path so launcher symlinks and
# FilePath::EnumPlugins/EnumDataDirs search the same tree.
set -e

APP_ROOT="/app/share/cities3d"
DATA_ROOT="${HOME}/.Cities3D"

mkdir -p "${DATA_ROOT}"

for item in plugins tex skins maps locale sounds help data; do
  if [ -e "${APP_ROOT}/${item}" ]; then
    ln -sfn "${APP_ROOT}/${item}" "${DATA_ROOT}/${item}"
  fi
done

# APP_ROOT has libwx/libGLU copies; /app/lib keeps the glu module install.
export LD_LIBRARY_PATH="${APP_ROOT}:/app/lib${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
cd "${DATA_ROOT}"
exec "${APP_ROOT}/Cities3D" "$@"
