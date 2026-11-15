# Cities3D

Cross-platform Cities & Knights / Seafarers (fork of [saladyears/Cities3D](https://github.com/saladyears/Cities3D)). The CMake project lives in `trunk/` (Boost, wxWidgets, GLEW, and miniaudio via vcpkg). Linux ships as a Flatpak (`flatpak/ca.skyblaster.Cities3D.yml`).

## Install (Linux Flatpak)

Needs [Flatpak](https://flatpak.org/setup/) and Flathub (for `org.freedesktop.Platform`):

```sh
flatpak remote-add --if-not-exists --user flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo

flatpak install --user /path/to/Cities3D.flatpak
flatpak run ca.skyblaster.Cities3D
```

Download `Cities3D.flatpak` from the [GitHub Releases](https://github.com/skyblaster/Cities3D/releases) page.

## Build the Flatpak

Install `flatpak` and `flatpak-builder`, then the Freedesktop runtime/SDK used by the manifest:

```sh
flatpak remote-add --if-not-exists --user flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo

flatpak install -y --user flathub \
  org.freedesktop.Platform//26.08 \
  org.freedesktop.Sdk//26.08
```

From the repository root:

```sh
./flatpak/build.sh                 # devel (RelWithDebInfo, symbols kept)
./flatpak/build.sh --release       # Release + strip (smaller bundle)
./flatpak/build.sh --full-rebuild  # wipe .flatpak-builder module cache
```

Optional: `--jobs N`, `--no-ccache`. Use `--full-rebuild` when switching between `--devel` and `--release`.

Run the installed app:

```sh
flatpak run ca.skyblaster.Cities3D
```

Export a bundle after a successful install:

```sh
flatpak build-bundle ~/.local/share/flatpak/repo \
  Cities3D.flatpak ca.skyblaster.Cities3D
```

## Windows

Windows developers keep using the existing CMake presets in `trunk/` with Visual Studio / Ninja and `VCPKG_ROOT` (presets `default` and `release`). That path is unchanged by the Flatpak work.
