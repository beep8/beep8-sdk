# BEEP-8 SDK

**BEEP-8** is a virtual retro game console designed for developing C/C++ applications. It runs on an emulated ARM v4T CPU at a fixed 2 MHz and is optimized for vertical smartphone displays. The SDK provides a two-layer architecture: you can access the hardware (H/W) directly using low-level APIs, or you can use a high-level C/C++ library for simpler, more abstracted development.

---

## Features

- **Emulated CPU:** ARM v4T running at a fixed 2 MHz.
  - Based on an architecture originating in the mid-1990s, the ARM v4T design provides a solid foundation for retro-style computing.
  - Compilable using GNU ARM GCC and supports C++20.
- **Memory:** 1 MB of RAM.
- **Display:** A 128×240 pixel vertical screen with a fixed 16-color palette.
- **Graphics Engine (PPU):**
  - Supports sprites, background layers, and primitive drawing (rectangles, lines, circles, polygons).
  - Provides control over color palettes, drawing depth (via `setz()`), and clipping.
- **Audio Engine (APU):**
  - Internally supports up to 16 channels; however, the initial configuration is set to 8 channels for simplicity.
  - Real-time sound synthesis similar to a Namco C30–style sound generator.
  - Implemented using the WebAudio API for high-performance audio processing.
- **Custom RTOS (`b8OS`):**
  - A lightweight real-time operating system handling threading, semaphores, system calls, and interrupt management.
  - Its design allows developers to focus on game creation without worrying about OS-level details.
- **Peripheral I/O:**
  - Fully supports keyboard, mouse, and touch inputs via a dedicated HIF module.
  - Additional modules provide serial communication (SCI), timer functions (TMR), and more.
- **Development Environment:**
  - Access the hardware directly through low-level APIs or use the higher-level C/C++ library that abstracts these details.
  - All C/C++ source code is fully open.
- **Browser-Based Execution:**
  - The emulator runs in major browsers (Chrome, Safari, Firefox, Edge) on PC, iPhone, iPad, Android, and virtually any device.
  - WebGL is employed to guarantee a consistent 60fps performance.
- **Distribution:**
  - Completed games are delivered as a single ROM file.
  - Anyone can publish their games on the BEEP-8 portal site.
- **Cost:**
  - The SDK is completely free to use, and game releases are free as well.

---

## Quick Start

1. **Clone the Repository:**
   ```bash
   git clone git@github.com:beep8/beep8-sdk.git
   ```

2. **Build a Sample Application:**
   Navigate to the SDK directory and run make:
   ```bash
   cd beep8-sdk
   make
   ```

3. **Run the Emulator:**
   Open the emulator by navigating to a URL similar to the following in your browser:
   ```bash
   https://beep8.org/b8/beep8.html?b8rom=your_app.b8
   ```
   > **Note:** In BEEP-8, output from `printf()` is not rendered on the screen—it is directed to the debug console (the log panel on the right side of the emulator UI).

---

## Architecture Overview

BEEP-8 comprises several key modules:

- **CPU & RTOS (b8OS):**  
  The heart of BEEP-8 is a custom RTOS that schedules threads on an emulated ARM v4T CPU operating at 2 MHz. You can either write code that directly interacts with the hardware registers or leverage the high-level C/C++ library to abstract hardware details. This dual-layer architecture offers both flexibility and ease of use.

- **Graphics (PPU):**  
  The PPU emulation supports drawing sprites, shapes, and polygons. It manages a 4bpp VRAM organized into sprite and background banks, offering full control over palettes and depth for drawing operations.

- **Audio (APU):**  
  The APU provides real-time sound synthesis with an initial configuration of 8 channels (while the hardware supports up to 16), emulating a Namco C30–style sound generator through WebAudio.

- **I/O Systems:**  
  Input is handled via a dedicated HIF module for keyboard, mouse, and touch. Additional peripherals include modules for serial communication, timers, and other virtual I/O.

- **Development Environment:**  
  All C/C++ source code is provided openly, and you can use GNU ARM GCC as well as C++20 for development. Use either low-level hardware access or a high-level API (in the `pico8` namespace) based on your preference.

---

## API Documentation

For complete API details, please refer to the header files in the `include/` directory. Key headers include:
- **pico8.h:** Defines the high-level API for graphics, sound, and input.
- **Other headers:** Provide low-level interfaces for the PPU, APU, and RTOS (`b8OS`).

---

## Sample Application

Below is a minimal "Hello World" sample that demonstrates a BEEP-8 application:

```c
#include <stdio.h>
#include <beep8.h>

int main(void) {
  // Print debug output; on BEEP-8, printf() output is sent to the
  // emulator's debug console (visible on the side) rather than on-screen.
  printf("hello BEEP-8\n");
  return 0;
}
```

> **Note:** Although this sample exits immediately after printing, it serves as the simplest example to illustrate how to compile and run a BEEP-8 application.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

## Contact

- **Email:** [beep8.official@gmail.com](mailto:beep8.official@gmail.com)
- **Website:** [https://beep8.org](https://beep8.org)

---

*Happy coding with BEEP-8!*
