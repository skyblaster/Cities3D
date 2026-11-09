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
  --unshare-netns \
  --yes

distrobox enter cities3d-flatpak
```

`--unshare-netns` gives the box its own network namespace so nested `bwrap` (used by `flatpak-builder`) is not tied to the host’s bind-mounted `/etc/resolv.conf`. Without it, VPN/Wi‑Fi/sleep on the host can leave that mount pointing at a deleted inode and builds fail with `bwrap: Can't bind mount /etc/resolv.conf`.

If you already created the box without `--unshare-netns`, either recreate it with the flags above, or as a short-term fix after a DNS change:

```sh
distrobox stop cities3d-flatpak
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

On a laptop like an Intel Core i5-1245U (10 cores / 12 threads), `build.sh` passes `--jobs=$(nproc)` into `flatpak-builder` and exports `CITIES3D_BUILD_JOBS` so **vcpkg** (wxWidgets during cmake configure) and **ninja** use the same width. Inside the module it also caps jobs by `MemAvailable` (~1.8 GiB per RelWithDebInfo bulkbuild) so parallel compiles do not thrash. Override with `./flatpak/build.sh --jobs 8` or `JOBS=8 ./flatpak/build.sh`. Look for `cities3d: build jobs=…` early in the cities3d module log. `--ccache` is on by default (disable with `--no-ccache`).

`flatpak-builder` needs `--force-clean` whenever `build-flatpak/` already exists (otherwise you get `App dir 'build-flatpak' is not empty`). That flag only clears the staging app dir. The **module cache** lives in `.flatpak-builder/` and still produces `Cache hit for glu` / unchanged modules on later runs.

Partial rebuilds (this is what you want day to day):

```sh
git pull
./flatpak/build.sh          # NOT --full-rebuild
```

Expect lines like `Cache hit for glu, skipping build` and `Cache hit for cities3d, skipping build` when C++ sources are unchanged. Desktop/icon/metainfo edits only rebuild the tiny `cities3d-metadata` module. Top-level finish-args / extension tweaks re-export the app without recompiling.

- Packaging-only (icons, `.desktop`, metainfo, launcher): cache hit on `glu` + `cities3d`; rebuild `cities3d-metadata` only.
- C++ / CMake / `vcpkg.json` changes: rebuild `cities3d` (vcpkg may still reuse downloaded ports).
- Wipe everything: `./flatpak/build.sh --full-rebuild`

Or run `flatpak-builder` yourself:

```sh
printf '%s\n' "$(nproc)" > flatpak/cities3d-build-jobs
flatpak-builder --user --install --force-clean \
  --disable-rofiles-fuse \
  --install-deps-from=flathub \
  --jobs="$(nproc)" --ccache \
  build-flatpak \
  flatpak/ca.skyblaster.Cities3D.yml
```


`--disable-rofiles-fuse` is required in Distrobox: `rofiles-fuse` needs `/dev/fuse`, which rootless Podman does not expose by default, and fails with `fusermount3: ... Permission denied` / `Failure spawning rofiles-fuse`.

The first build downloads the SDK, fetches the pinned vcpkg commit plus the matching `vcpkg-glibc` tool binary, and compiles wxWidgets/GTK and the rest. Expect a long run and several gigabytes of disk under `.flatpak-builder/` and `build-flatpak/`.

`flatpak-builder` sandboxes each module **without network** unless the manifest opts in. This manifest grants `--share=network` for the `cities3d` module so vcpkg can download port archives. If you still see `Could not resolve host: github.com`, check DNS from inside Distrobox (`getent hosts github.com`) before rebuilding. A `Downloading vcpkg-glibc...` / HTTP 504 failure means the prefetched tool binary is missing from the manifest — bump the `vcpkg-tool` file source when changing the vcpkg commit, or re-run the build (bootstrap retries as a fallback).

If the build dies immediately with `bwrap: Can't bind mount /etc/resolv.conf`, stop and re-enter the Distrobox (or recreate it with `--unshare-netns` as above). That is a Distrobox/host DNS mount issue, not a Cities3D compile failure.

### What the manifest does

1. Builds Mesa **GLU** into `/app` (the Freedesktop SDK has OpenGL but not `glu.pc` / `libGLU`, which vcpkg’s GLEW port requires).
2. Downloads vcpkg at the commit pinned in `trunk/vcpkg.json` (`builtin-baseline`) and the matching `vcpkg-glibc` release binary during the source-fetch phase (so the module does not depend on a live GitHub release download inside the sandbox).
3. Uses that prefetched tool (or bootstraps with retries if it is missing), then configures `trunk/` with Ninja + the vcpkg toolchain (`RelWithDebInfo`, unstripped) using the `x64-linux-wxshared` overlay triplet (shared `libwx`, other ports static). Static wx duplicated into each `.so` crashes on the first `FilePath` call.
4. Builds Cities3D and the rule plugins; installs binaries, `libwx_*.so*`, and game data (including `data/maps.db`) under `/app/share/cities3d`.
5. Installs the launcher, desktop file, metainfo, and icons (`cities3d-metadata` module).

After pulling a cities3d / wx linkage change, rebuild with a cold module cache:

```sh
git pull
./flatpak/build.sh --full-rebuild
```

A good build must print `cities3d: verified shared libwx + system GTK/GLib linkage`, and at runtime the first stderr line must be `cities3d: build=wxshared-sysgtk-1`. If you see a GLib-GObject `g_type_register_static` abort during startup, the Flatpak still has libwx with embedded static GLib — pull and `--full-rebuild`.

Confirm shared wx uses the runtime GTK/GLib (not a bundled libglib):

```sh
flatpak run --command=sh ca.skyblaster.Cities3D -c \
  'ldd /app/share/cities3d/libwx_gtk3u_core-*.so* | grep -E "libgtk-3|libglib"; ls /app/share/cities3d/libglib* 2>/dev/null || echo "no bundled libglib (good)"'
```

## Run

Prefer running from your **host** desktop session (not only inside Distrobox), so `$DISPLAY` / Wayland come from the real graphical login:

```sh
flatpak run ca.skyblaster.Cities3D
```

Startup prints `cities3d: ...` lines on stderr. If it still dies with exit 139, the last line shows how far `OnInit` / splash got. Rebuild after pulling this branch (`./flatpak/build.sh`) so symbols stay in `/app/share/cities3d/*.so`, then:

```sh
flatpak run --command=sh --devel \
  --nosocket=wayland --socket=x11 --device=dri --env=GDK_BACKEND=x11 \
  ca.skyblaster.Cities3D
# inside the sandbox:
gdb -batch -ex run -ex bt --args /app/share/cities3d/Cities3D
```

Or on the host after a crash: `coredumpctl info -1` (frames should no longer be `n/a`).

LAN play needs network access; the manifest already grants `--share=network`, DRI, PulseAudio, and an explicit X11 socket (wxWidgets/GTK still expects X11 even when Wayland is present).

If you see `Unable to initialize GTK+, is DISPLAY set properly?` before rebuilding with that X11 finish-arg, try a one-shot override (no rebuild):

```sh
flatpak run --socket=x11 --env=GDK_BACKEND=x11 ca.skyblaster.Cities3D
# or persist:
flatpak override --user --socket=x11 --env=GDK_BACKEND=x11 ca.skyblaster.Cities3D
```

The launcher uses a writable `$XDG_DATA_HOME/Cities3D` cwd (matches `wxStandardPaths::GetUserDataDir()`, assets symlinked from `/app`). The `cities3d-last-rule.txt` breadcrumb is written under that user data `data/` subdirectory (and `~/.Cities3D/` via `--persist=.Cities3D` for wxConfig), not into read-only `/app`.


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
