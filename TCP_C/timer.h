
#ifndef TIMER_H__

#define TIMER_H__ "timer.h"


/*****************************************************

对于NAT 这样老化忙,而有要频繁操作的类型.

if(timer->expires - jiffies < tm_out/2)
{
	mod_timer();
}

或者在老化时,判断老化时间时不时一致的.


nat->expires 与timer->expires 进行比较.如果没有到老化时间,则重新放入.

*******************************************************/

struct timer_list {
	struct list_head entry;	
	unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};

void  init_timer(struct timer_list * timer);
static inline void setup_timer(struct timer_list * timer,
				void (*function)(unsigned long),
				unsigned long data)
{
	timer->function = function;
	timer->data = data;
	init_timer(timer);
}



static inline int timer_pending(const struct timer_list * timer)
{
	return timer->entry.next != NULL;
}

int  init_timers_cpu(void);
void run_timers(void);

void add_timer(struct timer_list *timer);
int del_timer(struct timer_list * timer);
int __mod_timer(struct timer_list *timer, unsigned long expires);
int mod_timer(struct timer_list *timer, unsigned long expires);

unsigned long  get_mm_jiffies();
void jiffies_init();
unsigned long  jiffies_get();

#define HZ 1000

#define jiffies  jiffies_get()

#endif /*TIMER_H__*/
