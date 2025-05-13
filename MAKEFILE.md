# Advanced Build with Make

Each sample directory also contains a `run.sh` script for quick Bash-based builds, but this document covers instructions specifically for using `make`. BEEP-8 SDK includes a prebuilt `make` binary under `beep8-sdk/tool/make`, though you can also use any `make` already installed on your system.

---

## Prerequisites

- **GNU Make** (version 3.8 or later)

---

## Bundled Make & Environment Setup

If you do not have `make` installed, or wish to use the SDK’s bundled version, follow the instructions for your platform:

### macOS

1. **Use the bundled `make`**  
   ```bash
   source ./beeb8-env.sh
   ```  
   This prepends `./tool` to your `PATH`, ensuring the included `make` is used.  
2. **Install Xcode Command Line Tools**  
   ```bash
   xcode-select --install
   ```

### Windows MS-DOS

If you are on MS-DOS (not PowerShell or WSL):

```bat
beep8-env.bat
```

This sets up `make.exe` and `busybox` from the SDK for use in the command prompt.

---

## Build a Single Sample

Navigate to the sample directory (e.g., `pico8_example`) and run:

```bash
cd sdk/app/pico8_example
make clean       # Remove generated files
make             # Compile sources, convert assets, link, and generate ROM
make run         # Build and launch the ROM in your default web browser (e.g., Google Chrome)
```

- **`make clean`**: Deletes `obj/`, `data/export/`, and any intermediate files.  
- **`make`**: Builds the `.b8` ROM image in `obj/`.  
- **`make run`**: Builds (if needed) and opens the ROM in your default browser for execution.

---

## Build All Samples

From the `sdk/app/` directory, you can build every sample at once:

```bash
./build_all.sh
```

This script iterates through each sample subdirectory, invoking `make` for each.

---

## Makefile Structure

- **`Makefile`** in each sample  
  Sets `PROJECT` and includes `makefile.app` for common rules.  
- **`makefile.app`**  
  Defines rules to:  
  - Collect `.c`, `.cpp`, and converted `.png.cpp` files  
  - Invoke `png2c` for asset conversion  
  - Compile, assemble, link, and generate the `.b8` ROM  
- **`makefile.top`** and **`makefile.inc`**  
  Handle VPATH, toolchain prefixes, flags, and platform detection.

---

## Environment Variables & Options

These variables can be set when invoking `make` in each sample’s directory:

- **`EXPORT_LIST=1`**  
  Enable generation of an assembly listing (`.lst`) alongside the ROM:  
  ```bash
  EXPORT_LIST=1 make
  ```
- **`CFLAGS`, `LDFLAGS`, `CPPFLAGS`**  
  Customize compiler and linker flags:  
  ```bash
  make CFLAGS="-O3 -g" LDFLAGS="-specs=nano.specs"
  ```

---

## CI Integration Example

In a headless CI environment on Linux:

```bash
cd sdk/app/pico8_example
make -j6              # Parallel build
make run              # Launch in browser (or mock upload)
```

Or disable upload in CI by commenting out the `run` target and only generate the ROM:

```bash
make clean
make
```

---

*Happy building!*  
