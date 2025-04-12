#include <stdio.h>
#include <beep8.h>

// Entry point for the BEEP-8 application.
// This function is called once when the program starts.
int main(void) {
  // Prints a debug message to the BEEP-8 system log panel (not visible to end users).
  //
  // Note:
  // On BEEP-8, printf() does not output to the screen.
  // Instead, it writes to the developer-visible log console on the right side of the emulator UI.
  printf("hello BEEP-8\n");

  // Immediately exits. This program does not draw anything or run persistently.
  // It serves as the simplest possible example to demonstrate building and running a BEEP-8 app.
  return 0;
}
