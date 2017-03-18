#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
SYSCALL_DEFINE0 (find_roots) {
    struct task_struct  * curr;
    printk("find_roots system call called by %s  %d\n",current->comm,current->pid) ;
	curr=current;
    while (curr->pid!=1){
		printk("id : %d, name : %s\n",curr->pid,curr->comm);
		curr=curr->real_parent;
    }
	printk("id : %d, name : %s\n",curr->pid,curr->comm);
    return (0) ;
}
