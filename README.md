# BEEP-8 SDK

**BEEP-8** is a virtual retro game console inspired by PICO-8. Designed primarily for developing C/C++ applications, it runs on a 2MHz ARM v4T emulator and is optimized for vertical smartphone displays. The SDK provides both low-level hardware APIs and a PICO-8-like high-level API to ease the transition for developers familiar with PICO-8.

---

## Features

- **Emulated CPU**: ARM v4T running at a fixed 2 MHz.
- **Memory**: 2 MB of RAM.
- **Display**: A 128×240 pixel vertical screen with a fixed 16-color palette (PICO-8–compatible).
- **Graphics Engine (PPU)**:
  - Supports sprites, background layers, and primitive drawing (rectangles, lines, circles, polygons).
  - Allows manipulation of color palettes, drawing depth (z-coordinate via `setz()`), and clipping.
- **Audio Engine (APU)**:
  - Namco C30–like sound synthesis with 16-channel wavetable and 2-channel noise.
  - Real-time audio processing implemented in JavaScript using the WebAudio API.
- **Custom RTOS (`b8OS`)**:
  - Lightweight real-time operating system with threading, semaphores, system calls, and interrupt management.
  - Designed specifically for BEEP-8 to handle multitasking and hardware interfacing.
- **Peripheral I/O**:
  - Supports keyboard, mouse, and touch inputs via a built-in Human Interface (HIF) module.
  - Additional modules for serial communication (SCI), timer functions (TMR), and more.
- **Development Environment**:
  - Build your application in C/C++ using a familiar, PICO-8–like API (provided in the `pico8` namespace).
  - Source code and headers are organized for clarity and ease of use.
- **Browser-Based Execution**:
  - The emulator runs entirely in the web browser (Chrome, Safari, Edge, etc.), making distribution and testing convenient.

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
  The core is a custom RTOS that schedules threads on an emulated ARM v4T CPU running at 2MHz. All system calls (thread creation, semaphores, sleep, and exit) are handled via a dedicated SVC dispatch mechanism.

- **Graphics (PPU):**  
  The PPU emulation supports drawing sprites, rectangles, lines, and polygons. It manages a 4bpp VRAM arranged into sprite and background banks and supports palette management and clipping.

- **Audio (APU):**  
  A JavaScript-based emulation of a Namco C30–like sound engine that synthesizes audio in real time, including both wavetable and noise channels.

- **I/O Systems:**  
  Input handling (keyboard, mouse, touch) is managed by the HIF module, while peripheral devices such as SCI (serial communication), TMR (timers), and Virtual I/O are integrated into a cohesive framework.

- **High-Level API (pico8 namespace):**  
  Provides a set of functions that mimic the original PICO-8 API—such as `cls()`, `spr()`, `map()`, and more—allowing developers to leverage familiar constructs while taking advantage of native C/C++ performance.

---

## API Documentation

For full API details, please refer to the header files in the `include/` directory. Key headers include:
- **pico8.h**: Contains the PICO-8–like API for graphics, sound, and input.
- **Other headers**: Define low-level functions for PPU, APU, and the RTOS (`b8OS`).

---

## Sample Application

The following is a minimal "Hello World" sample demonstrating a BEEP-8 application:

```c
#include <stdio.h>
#include <beep8.h>

int main(void) {
  // Print debug output; note that on BEEP-8, printf() output is sent to the
  // emulator's debug console (visible on the side) rather than the screen.
  printf("hello BEEP-8\n");
  return 0;
}
```

> **Note:** Although this sample immediately exits after printing, it serves as the simplest example for compiling and running a BEEP-8 application.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

## Contact

- **Email:** beep8.official@gmail.com  
- **Website:** https://beep8.org

---

*Happy coding with BEEP-8!*
