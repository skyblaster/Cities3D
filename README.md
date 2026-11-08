# Cities3D 0.10.0

Linux packaging target is a **Flatpak**, built inside a **Distrobox** that uses the latest **Fedora toolbox** image. The CMake project lives in `trunk/` and still pulls Boost, wxWidgets, GLEW, and miniaudio through vcpkg (`trunk/vcpkg.json`). The Flatpak manifest is `flatpak/ca.skyblaster.Cities3D.yml`.

The host only needs Distrobox (and Podman). Compilers, SDKs, and Flatpak tooling stay in the container so an immutable host stays clean.

## Host: Distrobox

On the host (rpm-ostree / Atomic / any distro with Distrobox):

```sh
# Fedora hosts
sudo dnf install -y distrobox podman

# Or follow https://distrobox.it/ for other hosts
```

Create a Fedora 45 toolbox container (current Fedora toolbox release) and enter it:

```sh
distrobox create \
  --name cities3d-flatpak \
  --image quay.io/fedora/fedora-toolbox:45 \
  --yes

distrobox enter cities3d-flatpak
```

Use a newer `:NN` tag when Fedora ships the next toolbox image. `quay.io/fedora/fedora-toolbox` without a tag also tracks the latest toolbox release.

Home directory and the project checkout are shared with the host, so keep the repo where you usually clone it (for example `~/Cities3D`).

## Container: Flatpak tooling

Inside the Distrobox:

```sh
sudo dnf install -y flatpak flatpak-builder git curl unzip zip \
  cmake ninja-build gcc-c++ pkgconf-pkg-config \
  python3 bison flex gperf meson

flatpak remote-add --if-not-exists --user flathub \
  https://dl.flathub.org/repo/flathub.flatpakrepo

flatpak install -y --user flathub \
  org.freedesktop.Platform//26.08 \
  org.freedesktop.Sdk//26.08
```

`flatpak-builder` drives the real application build against the Freedesktop SDK. Distrobox only supplies that tool and a mutable Fedora userland.

## Build the Flatpak

From the repository root (still inside Distrobox):

```sh
cd ~/Cities3D   # or your checkout path

./flatpak/build.sh
```

Or run `flatpak-builder` yourself:

```sh
flatpak-builder --user --install --force-clean \
  --disable-rofiles-fuse \
  --install-deps-from=flathub \
  build-flatpak \
  flatpak/ca.skyblaster.Cities3D.yml
```

`--disable-rofiles-fuse` is required in Distrobox: `rofiles-fuse` needs `/dev/fuse`, which rootless Podman does not expose by default, and fails with `fusermount3: ... Permission denied` / `Failure spawning rofiles-fuse`.

The first build downloads the SDK, fetches the pinned vcpkg commit, bootstraps vcpkg, and compiles wxWidgets/GTK and the rest. Expect a long run and several gigabytes of disk under `.flatpak-builder/` and `build-flatpak/`.

`flatpak-builder` sandboxes each module **without network** unless the manifest opts in. This manifest grants `--share=network` for the `cities3d` module so vcpkg can download port archives. If you still see `Could not resolve host: github.com`, check DNS from inside Distrobox (`getent hosts github.com`) before rebuilding.

### What the manifest does

1. Downloads vcpkg at the commit pinned in `trunk/vcpkg.json` (`builtin-baseline`) during the source-fetch phase.
2. Bootstraps vcpkg, then configures `trunk/` with Ninja + the vcpkg toolchain (Release).
3. Builds Cities3D and the rule plugins.
4. Installs the executable, shared libraries, plugins, and game data under `/app/share/cities3d`, with a launcher at `/app/bin/Cities3D`.

## Run

```sh
flatpak run ca.skyblaster.Cities3D
```

LAN play needs network access; the manifest already grants `--share=network`, DRI, and PulseAudio.

## Export a bundle (optional)

```sh
flatpak build-bundle ~/.local/share/flatpak/repo \
  Cities3D.flatpak ca.skyblaster.Cities3D
```

Install that `.flatpak` on another machine with:

```sh
flatpak install --user Cities3D.flatpak
```

## Clean up

Inside Distrobox:

```sh
rm -rf build-flatpak .flatpak-builder repo
flatpak uninstall -y --user ca.skyblaster.Cities3D || true
```

On the host, remove the container when you no longer need it:

```sh
distrobox stop cities3d-flatpak
distrobox rm cities3d-flatpak
```

## Windows

Windows developers keep using the existing CMake presets in `trunk/` with Visual Studio / Ninja and `VCPKG_ROOT` (presets `default` and `release`). That path is unchanged by the Flatpak work.
