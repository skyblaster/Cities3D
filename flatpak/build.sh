#!/usr/bin/env bash
# Build and install the Cities3D Flatpak (run inside the Distrobox).
#
# Always passes --force-clean so the staging app dir (build-flatpak/) can be
# rewritten. That does NOT wipe the module cache in .flatpak-builder/, so glu
# and an unchanged cities3d module still show "Cache hit".
#
# True from-scratch rebuild (also delete the module cache):
#   ./flatpak/build.sh --full-rebuild
#
# Parallelism (i5-1245U = 12 threads; override if needed):
#   ./flatpak/build.sh --jobs 12
#   JOBS=8 ./flatpak/build.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

FULL_REBUILD=0
JOBS="${JOBS:-}"
USE_CCACHE=1
args=()
while [[ $# -gt 0 ]]; do
  case "$1" in
    --full-rebuild) FULL_REBUILD=1 ;;
    --jobs)
      shift
      JOBS="${1:-}"
      if [[ -z "${JOBS}" ]]; then
        echo "ERROR: --jobs requires a number" >&2
        exit 1
      fi
      ;;
    --jobs=*)
      JOBS="${1#--jobs=}"
      ;;
    --no-ccache) USE_CCACHE=0 ;;
    -h|--help)
      echo "Usage: $0 [--full-rebuild] [--jobs N] [--no-ccache]"
      echo "  (default)  --force-clean app dir; reuse .flatpak-builder module cache"
      echo "  --full-rebuild  also delete .flatpak-builder/ before building"
      echo "  --jobs N        parallel jobs for flatpak-builder / cmake / vcpkg"
      echo "                  (default: nproc; or set JOBS=N)"
      echo "  --no-ccache     disable flatpak-builder ccache"
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Usage: $0 [--full-rebuild] [--jobs N] [--no-ccache]" >&2
      exit 1
      ;;
  esac
  shift
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

if [[ -z "${JOBS}" ]]; then
  JOBS="$(nproc)"
fi
if ! [[ "${JOBS}" =~ ^[1-9][0-9]*$ ]]; then
  echo "ERROR: jobs must be a positive integer (got: ${JOBS})" >&2
  exit 1
fi

cpus="$(nproc)"
mem_kb="$(awk '/MemAvailable:/ {print $2}' /proc/meminfo 2>/dev/null || echo 0)"
echo "cities3d: host nproc=${cpus} MemAvailable_kB=${mem_kb} → flatpak-builder --jobs=${JOBS}"

if [[ "${FULL_REBUILD}" == "1" ]]; then
  echo "Full rebuild: removing .flatpak-builder/ and build-flatpak/."
  rm -rf .flatpak-builder build-flatpak
fi

args=(
  --user --install --force-clean
  --disable-rofiles-fuse
  --install-deps-from=flathub
  --jobs="${JOBS}"
)
if [[ "${USE_CCACHE}" == "1" ]]; then
  args+=(--ccache)
fi
# Visible inside the cities3d module sandbox for vcpkg + cmake --build.
args+=(--env=CITIES3D_BUILD_JOBS="${JOBS}")

# --force-clean clears build-flatpak/ (required when that dir already exists).
# Module artifacts stay cached under .flatpak-builder/ unless --full-rebuild.
# Distrobox / rootless Podman usually cannot run rofiles-fuse.
exec flatpak-builder "${args[@]}" \
  build-flatpak \
  flatpak/ca.skyblaster.Cities3D.yml
