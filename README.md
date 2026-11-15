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

## Install (Windows)

Download `Install_Cities3D_v0.10.0.exe` from the [GitHub Releases](https://github.com/skyblaster/Cities3D/releases) page and run it.

The setup is 64-bit and requires Windows 10 or later. It installs for all users under Program Files, adds a Start menu shortcut, and includes the Visual C++ runtime DLLs the game needs. Tilesets, sounds, skins, and languages are the same choices as the 0.9.7 installer. Cities Online is not part of this build. `COPYING` and `THIRD-PARTY-LICENSES.txt` are installed next to the program.

## Build the Windows installer

Install [NSIS 3](https://nsis.sourceforge.io/), then build the Release preset from `trunk/` (`cmake --build --preset release`, with `VCPKG_ROOT` set). From the repository root:

```powershell
.\trunk\Cities3D\installer\build-windows-installer.ps1
```

That writes `trunk/Cities3D/installer/build/Install_Cities3D_v0.10.0.exe` and copies the same file into your Downloads folder. The script installs a temporary copy, checks the file layout, and removes it.
