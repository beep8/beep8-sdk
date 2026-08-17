// One-shot host -> ROM file receive over SCI. See upload.h.
//
// The inverse of download.cpp: the ROM writes a short "open\n" request straight
// to the SCI TX FIFO, then drains the length-prefixed reply from the RX FIFO a
// bit at a time across frames (never blocking, since the reply only arrives once
// the user has picked a file). The framing is a leading "<len>\n" header, so the
// payload is fully binary-safe (a PNG's 0x00 / 0xff bytes are fine).
#include <upload.h>
#include <b8/type.h>
#include <b8/register.h>

#define SCI_CH_UP (11)          // free SCI channel (0/1/2 vterm/savedata/http,
                                //                   12/13 download/clip)
#define UP_STALL_FRAMES (600)   // ~10s @60fps: bail if a started transfer stalls
namespace {
  int  st_state  = upload::IDLE;
  int  st_need   = -1;          // payload length (-1 = still reading header)
  int  st_got    = 0;           // payload bytes received
  int  st_hval   = 0;           // running value of the "<len>" header
  bool st_discard = false;      // payload too big for the caller buffer
  bool st_started = false;      // at least one byte has arrived
  int  st_idle    = 0;          // consecutive Poll()s with no new bytes
}

namespace {
// Both requests share one reply state machine: only one is ever outstanding.
void request(const char* cmd) {
  while (B8_FIFO_SCI_RX_LEN(SCI_CH_UP) > 0) (void)B8_FIFO_SCI_RX(SCI_CH_UP);
  for (int i = 0; cmd[i]; ++i) B8_FIFO_SCI_TX(SCI_CH_UP) = (u32)(u8)cmd[i];
  st_state = upload::WAITING; st_need = -1; st_got = 0; st_hval = 0;
  st_discard = false; st_started = false; st_idle = 0;
}
}

namespace upload {

void Begin() { request("open\n"); }

// Status probe. The reply rides the same "<len>\n" framing as a file, so it is
// drained by the ordinary Poll() -- a 1-byte payload holding the flag bits.
// A host that predates this request never answers, so callers must give up on
// their own (Poll() only times out once bytes have started arriving).
void Stat() { request("stat\n"); }

State Poll(unsigned char* buf, int cap, int* out_len) {
  if (st_state != WAITING) return (State)st_state;
  bool progressed = false;
  while (B8_FIFO_SCI_RX_LEN(SCI_CH_UP) > 0) {
    progressed = true; st_started = true;
    const u8 b = (u8)B8_FIFO_SCI_RX(SCI_CH_UP);
    if (st_need < 0) {                                 // reading "<len>\n"
      if (b == '\n') {
        if (st_hval <= 0) { st_state = CANCELLED; return (State)st_state; }
        st_need = st_hval; st_got = 0; st_discard = (st_hval > cap);
      } else if (b >= '0' && b <= '9') {
        st_hval = st_hval * 10 + (b - '0');
        if (st_hval > (1 << 24)) { st_state = ERROR; return (State)st_state; }
      } else {
        st_state = ERROR; return (State)st_state;      // junk in the header
      }
    } else {                                           // payload bytes
      if (!st_discard) buf[st_got] = (unsigned char)b;
      ++st_got;
      if (st_got >= st_need) {
        if (st_discard) { st_state = ERROR; }
        else { if (out_len) *out_len = st_need; st_state = DONE; }
        return (State)st_state;
      }
    }
  }
  // Once bytes have started flowing, a long silence means the host stopped mid
  // transfer (backgrounded tab, lost data). Give up rather than spin forever.
  if (st_started) {
    if (progressed) st_idle = 0;
    else if (++st_idle > UP_STALL_FRAMES) { st_state = ERROR; return (State)st_state; }
  }
  return WAITING;
}

} // namespace upload
