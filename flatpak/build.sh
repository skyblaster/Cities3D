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

exec flatpak-builder --user --install --force-clean \
  --install-deps-from=flathub \
  build-flatpak \
  flatpak/ca.skyblaster.Cities3D.yml
