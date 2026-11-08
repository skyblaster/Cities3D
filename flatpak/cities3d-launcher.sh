#!/bin/sh
# Cities3D resolves plugins/, tex/, and skins/ relative to the executable.
# Keep the working directory on the installed data tree inside the Flatpak.
cd /app/share/cities3d || exit 1
export LD_LIBRARY_PATH="/app/share/cities3d${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec ./Cities3D "$@"
