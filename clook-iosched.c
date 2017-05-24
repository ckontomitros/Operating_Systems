/*
 * elevator noop
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
static sector_t thesi=-1;
struct clook_data {
	struct list_head queue;
};

static void clook_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
{
	list_del_init(&next->queuelist);
}

static int clook_dispatch(struct request_queue *q, int force)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char 	dieuthinsi;
	if (!list_empty(&nd->queue)) {
	  struct request *rq;
	  rq = list_entry(nd->queue.next, struct request, queuelist);
	  thesi=rq->__sector;	
	  list_del_init(&rq->queuelist);
	  if(rq_data_dir(rq) == READ)
	    dieuthinsi = 'R';
	  else
	    dieuthinsi = 'W';
	  printk("[CLOOK] dsp %c %lu \n", dieuthinsi, rq->__sector);
	  elv_dispatch_sort(q, rq);
	  return 1;
	}
	return 0;
}

static void clook_add_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;
	char 	dieuthinsi;
	struct list_head *curr = NULL;
	int flag=0;
	int sec;
	struct request *cur=NULL;
	list_for_each(curr, &nd->queue){
	  cur = list_entry(curr, struct request,queuelist);
	  if(rq->__sector>thesi&&cur->__sector>thesi){
	     if(rq->__sector<cur->__sector){
	       flag=1;
	       break;
	     }
	  }
	  else if(rq->__sector>thesi&&cur->__sector<thesi){
	     list_add_tail(&rq->queuelist,curr);
	     flag=1;
	     break;
	  }
	  else{
	    if(rq->__sector<cur->__sector){
	      flag=1;
	      break;
	    }
	      
	  }
	}
	if(flag==0){
	  list_add_tail(&rq->queuelist, &nd->queue);
	}
	else
	  list_add_tail(&rq->queuelist,curr);
	if(rq_data_dir(rq) == READ)
		dieuthinsi = 'R';
	else
		dieuthinsi = 'W';
	printk("[CLOOK] add %c %lu\n", dieuthinsi, rq->__sector);
}

static struct request *
clook_former_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.prev == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
clook_latter_request(struct request_queue *q, struct request *rq)
{
	struct clook_data *nd = q->elevator->elevator_data;

	if (rq->queuelist.next == &nd->queue)
		return NULL;
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

static int clook_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct clook_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;

	INIT_LIST_HEAD(&nd->queue);

	spin_lock_irq(q->queue_lock);
	q->elevator = eq;
	spin_unlock_irq(q->queue_lock);
	return 0;
}

static void clook_exit_queue(struct elevator_queue *e)
{
	struct clook_data *nd = e->elevator_data;

	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

static struct elevator_type elevator_clook = {
	.ops = {
		.elevator_merge_req_fn		= clook_merged_requests,
		.elevator_dispatch_fn		= clook_dispatch,
		.elevator_add_req_fn		= clook_add_request,
		.elevator_former_req_fn		= clook_former_request,
		.elevator_latter_req_fn		= clook_latter_request,
		.elevator_init_fn		= clook_init_queue,
		.elevator_exit_fn		= clook_exit_queue,
	},
	.elevator_name = "clook",
	.elevator_owner = THIS_MODULE,
};

static int __init clook_init(void)
{
	return elv_register(&elevator_clook);
}

static void __exit clook_exit(void)
{
	elv_unregister(&elevator_clook);
}

module_init(clook_init);
module_exit(clook_exit);


MODULE_AUTHOR("Kontomitros Axelos Athanasiadis");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Clo-ok IO scheduler");
