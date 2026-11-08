#!/usr/bin/env bash
# Build and install the Cities3D Flatpak (run inside the Distrobox).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

if ! command -v flatpak-builder >/dev/null 2>&1; then
  echo "flatpak-builder not found. Enter the cities3d-flatpak Distrobox and install it:" >&2
  echo "  sudo dnf install -y flatpak flatpak-builder" >&2
  exit 1
fi

# Distrobox often bind-mounts the host resolv.conf. When the host rewrites that
# file (VPN, Wi-Fi, sleep), the mount can point at a deleted inode and nested
# bwrap fails with: Can't bind mount /etc/resolv.conf
if grep -q ' /etc/resolv.conf .*//deleted' /proc/self/mountinfo 2>/dev/null \
  || { [[ -e /etc/resolv.conf ]] && [[ "$(stat -c %h /etc/resolv.conf 2>/dev/null || echo 1)" == "0" ]]; }; then
  echo "Distrobox DNS mount looks stale (deleted /etc/resolv.conf inode)." >&2
  echo "Fix, then re-run ./flatpak/build.sh:" >&2
  echo "  distrobox stop cities3d-flatpak && distrobox enter cities3d-flatpak" >&2
  echo "Or recreate the box with --unshare-netns (see README.md)." >&2
  exit 1
fi

# Distrobox / rootless Podman usually cannot run rofiles-fuse (Permission denied
# on .flatpak-builder/rofiles). Hard links / copies are slower but work.
exec flatpak-builder --user --install --force-clean \
  --disable-rofiles-fuse \
  --install-deps-from=flathub \
  build-flatpak \
  flatpak/ca.skyblaster.Cities3D.yml
