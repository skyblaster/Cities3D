# Cities3D 0.10.0

Linux build for an immutable system, using [Homebrew](https://docs.brew.sh/Homebrew-on-Linux) only. The CMake project is in `trunk/`. The checked-in `linux` preset is a Debug build that downloads and compiles its own copies of Boost, wxWidgets, GTK, and the rest through vcpkg. The first configure is a long source build and needs several gigabytes of disk.

This path uses the same manifest as the Windows build (`trunk/vcpkg.json`, baseline `0c2a9ace1964b818b15f7ae50ba1c42404918a7b`). vcpkg does not build OpenGL. The headers and link libraries come from Homebrew's Mesa.

## Homebrew

Install Homebrew, then put it on `PATH` for this shell and for later shells:

```sh
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"
```

If the installer prints a different `brew shellenv` path, use that path. Add the same `eval` line to the shell startup file so new terminals can find `brew`.

Install the compiler, the build tools, and the OpenGL SDK:

```sh
brew install gcc cmake ninja pkgconf git curl unzip zip python3 bison flex gperf meson mesa glu
```

`gcc` from Homebrew must be the `gcc` and `g++` that `PATH` finds. `eval "$(brew shellenv)"` does that.

## vcpkg

Clone vcpkg with full git history. A shallow copy cannot check out the baseline pinned in `trunk/vcpkg.json`.

```sh
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh" -disableMetrics
export VCPKG_ROOT="$HOME/vcpkg"
export VCPKG_DEFAULT_TRIPLET=x64-linux
export CMAKE_PREFIX_PATH="$(brew --prefix):$(brew --prefix mesa):$(brew --prefix glu)"
export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
```

Keep those `export` lines in the same shell you use for CMake. Add them next to the `brew shellenv` line if you want them in every new terminal.

## Build

From a checkout of this repository:

```sh
cd trunk
cmake --preset linux
cmake --build --preset linux -j "$(nproc)"
```

CMake configures `trunk/build/linux` and writes the program, the shared libraries, and the plugins into `trunk/Cities3D/distribute`. Plugins land in `trunk/Cities3D/distribute/plugins`.

For a Release build instead of the Debug preset:

```sh
cd trunk
cmake -S . -B build/linux-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build/linux-release -j "$(nproc)"
```

If a vcpkg port stops because a command is missing, install that command with `brew install` and run the same `cmake` command again. Configure is safe to repeat.

## Run

The program looks for `plugins/`, `tex/`, and `skins/` next to the executable.

```sh
cd trunk/Cities3D/distribute
./Cities3D
```

If the loader cannot find a library, point it at the vcpkg install tree for the build you made:

```sh
export LD_LIBRARY_PATH="$PWD/../../build/linux/vcpkg_installed/x64-linux/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
./Cities3D
```

Use `build/linux-release/vcpkg_installed/x64-linux/lib` after a Release build. Leave Homebrew's `lib` off `LD_LIBRARY_PATH` so the process keeps the graphics driver's `libGL` rather than Mesa's.
