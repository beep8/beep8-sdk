# BEEP-8 SDK

**BEEP-8** is a virtual retro game console designed for developing C/C++ applications. It runs on an emulated ARM v4T CPU at a fixed 2 MHz and is optimized for vertical smartphone displays. The SDK provides a two-layer architecture: you can access the hardware (H/W) directly using low-level APIs, or you can use a high-level C/C++ library (PICO-8 for C/C++) for simpler, more abstracted development.

---

## Features

- **Emulated CPU:** ARM v4T running at a fixed 2 MHz.  
  - Based on an architecture originating in the mid-1990s.
  - Compilable using GNU ARM GCC and supports C++20.
- **Memory:** 1 MB of RAM.
- **Display:** A 128×240 pixel vertical screen with a fixed 16-color palette.  
  - Fully compatible with the original PICO-8 16-color palette.
- **Audio Engine (APU):**
  - Real-time sound synthesis using a Namco C30–style model.
  - Configured with **8 channels** (initial version).
- **Custom RTOS (`b8OS`):**
  - A lightweight real-time operating system handles threading, semaphores, system calls, and interrupt management.
  - Its design allows developers to focus on game creation without worrying about OS-level details.
- **Peripheral I/O:**
  - Fully supports keyboard, mouse, and **touch inputs (スマホのタッチ操作に対応)** via a dedicated HIF module.
- **Development Environment:**
  - Upper-level developers can directly control virtual hardware.
  - Alternatively, a PICO-8-like C/C++ library is available for rapid development.
  - All C/C++ source code is fully open and available for modification.
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
