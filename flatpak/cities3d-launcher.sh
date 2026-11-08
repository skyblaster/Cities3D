#!/bin/sh
# Cities3D loads plugins/tex/skins relative to cwd, and writes
# cities3d-last-rule.txt into cwd. /app is read-only in Flatpak, so run from a
# writable per-user data dir and symlink the immutable game tree from /app.
set -e

APP_ROOT="/app/share/cities3d"
DATA_ROOT="${XDG_DATA_HOME:-${HOME}/.local/share}/cities3d"

mkdir -p "${DATA_ROOT}"

# Refresh links to packaged assets/binaries (safe if already correct).
for item in Cities3D \
  DatabaseCore.so ModelCore.so RulesCore.so GLCore.so UICore.so \
  plugins tex skins maps locale sounds help; do
  if [ -e "${APP_ROOT}/${item}" ]; then
    ln -sfn "${APP_ROOT}/${item}" "${DATA_ROOT}/${item}"
  fi
done

export LD_LIBRARY_PATH="${DATA_ROOT}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
cd "${DATA_ROOT}"
exec ./Cities3D "$@"
