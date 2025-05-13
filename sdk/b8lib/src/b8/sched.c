#include <b8/os.h>
#include <b8/sched.h>

int sched_get_priority_max(int policy ){
  (void)policy;
  return 255;
}
