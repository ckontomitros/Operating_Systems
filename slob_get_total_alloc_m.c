#include <linux/kernel.h>
#include <linux/syscalls.h>
extern long *call_total; 
SYSCALL_DEFINE0(slob_get_total_alloc_m){
  return *call_total;
}
