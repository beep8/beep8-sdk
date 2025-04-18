# BEEP-8 SDK

**BEEP-8** is a virtual retro game console designed for developing C/C++ applications. It runs on an emulated ARM v4T CPU at a fixed 2 MHz and is optimized for vertical smartphone displays. The SDK provides a two-layer architecture: you can access the hardware (H/W) directly using low-level APIs, or you can use a high-level C/C++ library (PICO-8 for C/C++) for simpler, more abstracted development.

---

## Features
## System Block Diagram

![BEEP-8 Block Diagram](doc/img/BEEP-8_BLOCK.png)

- **Emulated CPU:** ARM v4T running at a fixed 2 MHz.  
  - Based on an architecture originating in the mid-1990s.  
  - Compilable using GNU ARM GCC and supports C++20.

- **Memory:** Main RAM is 1 MB and VRAM is 128 KB (shared for BG and sprite patterns, 4 bpp, 512×512).

- **ROM Limit:** Maximum size is **1024 KB** per game.

- **PPU (Pixel Processing Unit):** Handles all rendering operations including background layers, sprites, and shape drawing.  
– Drives the 128×240 pixel display using a fixed 16-color palette.  
– Shares access to 128 KB of VRAM with both BG and sprite patterns (4 bpp, 512×512 layout).

- **APU (Audio Processing Unit):** Emulates a Namco C30-style sound engine.  
– Supports 8 audio channels with real-time synthesis.  
– Provides retro-style sound effects and music playback.

- **HIF (Human Interface):** Manages keyboard, mouse, and touch input.  
– Converts browser input events into system-level signals.  
– Ideal for both PC and mobile web environments.

- **TMR (Timer Module):** Provides high-precision system timing for scheduling and interrupts.  
– Drives periodic system tasks at a consistent 60 Hz tick rate.  
– Integrated with the custom RTOS (`b8OS`) for real-time operations.


- **Custom RTOS (`b8OS`):**  
  - A lightweight real-time operating system handles threading, semaphores, system calls, and interrupt management.  
  - Its design allows developers to focus on game creation without worrying about OS-level details.

- **Peripheral I/O:**  
  - Fully supports keyboard, mouse, and **touch inputs** via a dedicated HIF module.

- **Development Environment:**  
  - Upper-level developers can directly control virtual hardware (e.g., PPU, APU, I/O registers).  
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
    BEEP‑8 applications are built by directly manipulating the virtual hardware registers in C/C++. 
    There is no predefined framework—developers can code in any style using custom libraries or modules.
    Full support up to C++20, including the STL, means that most existing C++ libraries can be
    built and run on BEEP‑8.

    At the same time, we also provide a PICO‑8–style sample library for developers familiar with PICO‑8 game development to use.
    Navigate to the SDK directory and run make:
    ```bash
    cd ./beep8-sdk/sdk/app/pico8_example
    make clean
    make
    ```

3. **Run the Emulator:**
   In your terminal, change to the example directory and run:
   
   ```bash
   cd ./beep8-sdk/sdk/app/pico8_example
   make run
   ```

   This command launches the built BEEP‑8 ROM (`b8rom`) in your default web browser. BEEP‑8 uses Google Chrome by default, so ensure Chrome is installed on your system.
   
   > **Note:** In BEEP‑8, output from `printf()` is not rendered on‑screen—it appears in the debug console (the log panel on the right side of the emulator UI).

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
