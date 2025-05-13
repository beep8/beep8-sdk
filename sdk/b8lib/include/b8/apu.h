// ppu / Audio Processing Unit
#pragma once
#include <b8/type.h>

#define B8_APU_ADDR    (0xffff9000)
#define B8_APU_EXEC    _B8_REG(B8_APU_ADDR + 0x00)

#define B8_APU_CMD_NOP         (0x00)
#define B8_APU_CMD_HALT        (0xff)
#define B8_APU_CMD_ATTACK      (0x01)
#define B8_APU_CMD_ATTACKTIME  (0x02)
#define B8_APU_CMD_ATTACKAMP   (0x03)
#define B8_APU_CMD_DECAYTIME   (0x04)
#define B8_APU_CMD_SUSTAINTIME (0x05)
#define B8_APU_CMD_SUSTAINAMP  (0x06)
#define B8_APU_CMD_RELEASETIME (0x07)
#define B8_APU_CMD_SETFREQ     (0x10)
#define B8_APU_CMD_SETWAVTYPE  (0x11)
#define B8_APU_CMD_TRACKVOL    (0x13)

#define B8_APU_WAVE_SIN      (0)
#define B8_APU_WAVE_SQUARE   (1)
#define B8_APU_WAVE_TRIANGLE (2)
#define B8_APU_WAVE_SAWTOOTH (3)
#define B8_APU_WAVE_NOISE    (7)

#define B8_APU_EXEC_START  (0x1)
