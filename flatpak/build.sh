#!/usr/bin/env bash
# Build and install the Cities3D Flatpak (run inside the Distrobox).
#
# Always passes --force-clean so the staging app dir (build-flatpak/) can be
# rewritten. That does NOT wipe the module cache in .flatpak-builder/, so glu
# and an unchanged cities3d module still show "Cache hit".
#
# True from-scratch rebuild (also delete the module cache):
#   ./flatpak/build.sh --full-rebuild
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

FULL_REBUILD=0
for arg in "$@"; do
  case "${arg}" in
    --full-rebuild) FULL_REBUILD=1 ;;
    -h|--help)
      echo "Usage: $0 [--full-rebuild]"
      echo "  (default)  --force-clean app dir; reuse .flatpak-builder module cache"
      echo "  --full-rebuild  also delete .flatpak-builder/ before building"
      exit 0
      ;;
    *)
      echo "Unknown option: ${arg}" >&2
      echo "Usage: $0 [--full-rebuild]" >&2
      exit 1
      ;;
  esac
done

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

if [[ "${FULL_REBUILD}" == "1" ]]; then
  echo "Full rebuild: removing .flatpak-builder/ and build-flatpak/."
  rm -rf .flatpak-builder build-flatpak
fi

# --force-clean clears build-flatpak/ (required when that dir already exists).
# Module artifacts stay cached under .flatpak-builder/ unless --full-rebuild.
# Distrobox / rootless Podman usually cannot run rofiles-fuse.
exec flatpak-builder --user --install --force-clean \
  --disable-rofiles-fuse \
  --install-deps-from=flathub \
  build-flatpak \
  flatpak/ca.skyblaster.Cities3D.yml
