#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <x-list.h>
#include <x-timer.h>

#define time_after(a,b)		\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(b) - (long)(a) < 0))
#define time_before(a,b)	time_after(b,a)

#define time_after_eq(a,b)	\
	(typecheck(unsigned long, a) && \
	 typecheck(unsigned long, b) && \
	 ((long)(a) - (long)(b) >= 0))
#define time_before_eq(a,b)	time_after_eq(b,a)


#define TVN_BITS 	6
#define TVR_BITS 	8
#define TVN_SIZE 	(1 << TVN_BITS)
#define TVR_SIZE 	(1 << TVR_BITS)
#define TVN_MASK 	(TVN_SIZE - 1)
#define TVR_MASK 	(TVR_SIZE - 1)

typedef struct tvec_s {
	struct list_head vec[TVN_SIZE];
} tvec_t;

typedef struct tvec_root_s {
	struct list_head vec[TVR_SIZE];
} tvec_root_t;

typedef struct tvec_t_base_s {
	
	struct timer_list 		*running_timer;
	unsigned long 			timer_jiffies;
	tvec_root_t 			tv1;
	tvec_t 				tv2;
	tvec_t 				tv3;
	tvec_t 				tv4;
	tvec_t 				tv5;
}tvec_base_t;

tvec_base_t boot_tvec_bases;


int  jhl_timer_num = 0;

int  init_timers_cpu(void)
{
	int j;
	tvec_base_t *base = &boot_tvec_bases;
	memset((void*)base, 0, sizeof(*base));

	for (j = 0; j < TVN_SIZE; j++) {
		INIT_LIST_HEAD(base->tv5.vec + j);
		INIT_LIST_HEAD(base->tv4.vec + j);
		INIT_LIST_HEAD(base->tv3.vec + j);
		INIT_LIST_HEAD(base->tv2.vec + j);
	}
	for (j = 0; j < TVR_SIZE; j++)
		INIT_LIST_HEAD(base->tv1.vec + j);

	base->timer_jiffies = jiffies;
	return 0;
}


void  init_timer(struct timer_list *timer)
{
	timer->entry.next = NULL;	
}

static inline void detach_timer(struct timer_list *timer)
{
	struct list_head *entry = &timer->entry;

	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = LIST_POISON2;

	jhl_timer_num--;
}



int del_timer(struct timer_list *timer)
{
	int ret = 0;
			
	if (timer_pending(timer)) {
		detach_timer(timer);
		ret = 1;
	}	
	
	return ret;
}

static void internal_add_timer(tvec_base_t *base, struct timer_list *timer)
{
	unsigned long expires = timer->expires;
	unsigned long idx = expires - base->timer_jiffies;
	struct list_head *vec;

	if (idx < TVR_SIZE) {
		int i = expires & TVR_MASK;
		vec = base->tv1.vec + i;
	} else if (idx < 1 << (TVR_BITS + TVN_BITS)) {
		int i = (expires >> TVR_BITS) & TVN_MASK;
		vec = base->tv2.vec + i;
	} else if (idx < 1 << (TVR_BITS + 2 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + TVN_BITS)) & TVN_MASK;
		vec = base->tv3.vec + i;
	} else if (idx < 1 << (TVR_BITS + 3 * TVN_BITS)) {
		int i = (expires >> (TVR_BITS + 2 * TVN_BITS)) & TVN_MASK;
		vec = base->tv4.vec + i;
	} else if ((signed long) idx < 0) {
		/*
		 * Can happen if you add a timer with expires == jiffies,
		 * or you set a timer to go off in the past
		 */
		vec = base->tv1.vec + (base->timer_jiffies & TVR_MASK);
	} else {
		int i;
		/* If the timeout is larger than 0xffffffff on 64-bit
		 * architectures then we use the maximum timeout:
		 */
		if (idx > 0xffffffffUL) {
			idx = 0xffffffffUL;
			expires = idx + base->timer_jiffies;
		}
		i = (expires >> (TVR_BITS + 3 * TVN_BITS)) & TVN_MASK;
		vec = base->tv5.vec + i;
	}
	/*
	 * Timers are FIFO:
	 */
	list_add_tail(&timer->entry, vec);

	jhl_timer_num++;
}


static int cascade(tvec_base_t *base, tvec_t *tv, int index)
{
	/* cascade all the timers from tv up one level */
	struct timer_list *timer, *tmp;
	struct list_head tv_list;

	list_replace_init(tv->vec + index, &tv_list);

	/*
	 * We are removing _all_ timers from the list, so we
	 * don't have to detach them individually.
	 */
	list_for_each_entry_safe(timer, tmp, &tv_list, entry) {		
		internal_add_timer(base, timer);
	}

	return index;
}

#define INDEX(N) ((base->timer_jiffies >> (TVR_BITS + (N) * TVN_BITS)) & TVN_MASK)

/**
 * __run_timers - run all expired timers (if any) on this CPU.
 * @base: the timer vector to be processed.
 *
 * This function cascades all vectors and executes all expired timer
 * vectors.
 */
static inline void __run_timers(tvec_base_t *base)
{
	struct timer_list *timer;
	
	while (time_after_eq(jiffies, base->timer_jiffies)) {
		struct list_head work_list;
		struct list_head *head = &work_list;
 		int index = base->timer_jiffies & TVR_MASK;

		/*
		 * Cascade timers:
		 */
		if (!index &&
			(!cascade(base, &base->tv2, INDEX(0))) &&
				(!cascade(base, &base->tv3, INDEX(1))) &&
					!cascade(base, &base->tv4, INDEX(2)))
			cascade(base, &base->tv5, INDEX(3));
		++base->timer_jiffies;
		list_replace_init(base->tv1.vec + index, &work_list);
		while (!list_empty(head)) {
			void (*fn)(unsigned long);
			unsigned long data;

			timer = list_entry(head->next,struct timer_list,entry);
 			fn = timer->function;
 			data = timer->data;			
			detach_timer(timer);
			fn(data);				
		}
	}
	
}


void run_timers(void)
{
	__run_timers(&boot_tvec_bases);
}
	


int __mod_timer(struct timer_list *timer, unsigned long expires)
{
	int ret = 0;
	if (timer_pending(timer)) {
		detach_timer(timer);
		ret = 1;
	}
	
	timer->expires = expires;
	internal_add_timer(&boot_tvec_bases, timer);
	return ret;
}

void add_timer(struct timer_list *timer)
{	
	__mod_timer(timer,timer->expires);
}

int mod_timer(struct timer_list *timer, unsigned long expires)
{	
	/*
	 * This is a common optimization triggered by the
	 * networking code - if the timer is re-modified
	 * to be the same thing then just return:
	 */
	if (timer->expires == expires && timer_pending(timer))
		return 1;

	return __mod_timer(timer, expires);
}


unsigned long ll_start_second = 0;

unsigned long ll_jiffies_second = 0;

void jiffies_init()
{
	struct timeval tv;
	gettimeofday (&tv, NULL);	
	ll_start_second = tv.tv_sec;
	
}

//unsigned long get_jiffies();

unsigned long  jiffies_get()
{
	#if 1
	unsigned long tt;
	unsigned long cc_s;
	struct timeval tv;
	gettimeofday (&tv, NULL);	

	cc_s = (tv.tv_sec - ll_start_second);

	if(cc_s - ll_jiffies_second > 2)
	{
		//set_sys_date();
		ll_jiffies_second+=2;
		ll_start_second = tv.tv_sec - ll_jiffies_second;
		
	}else
	{
		ll_jiffies_second = cc_s;	
	}

	
	if(tv.tv_usec )
		tt = (tv.tv_usec / 1000) + (ll_jiffies_second)*HZ;
	else
		tt = (ll_jiffies_second)*HZ;
	
	return tt;	
	#else
	return  get_mm_jiffies();
	#endif
}


#if 0

struct timer_list test_timer1;
struct timer_list test_timer2;

void test_ttt_fun1(unsigned long data)
{
	struct timer_list *tt =(struct timer_list*)data;
	JHL_PRINTF("test_ttt_fun1 \n");
	tt->expires = jiffies + 1200;
	add_timer(tt);
}
void test_ttt_fun2(unsigned long data)
{
	struct timer_list *tt =(struct timer_list*)data;
	JHL_PRINTF("test_ttt_fun2 \n");
	tt->expires = jiffies + 1500;
	add_timer(tt);
}

void test_ttt()
{
	init_timer(&test_timer1);
	test_timer1.data = (unsigned long)&test_timer1;
	test_timer1.expires = jiffies + 3*HZ;
	test_timer1.function = test_ttt_fun1;
	add_timer(&test_timer1);

	init_timer(&test_timer2);
	test_timer2.data = (unsigned long)&test_timer2;
	test_timer2.expires = jiffies + 4*HZ;
	test_timer2.function = test_ttt_fun2;
	add_timer(&test_timer2);
}


#endif
char WAYOS[] = 
"\033[2J\033[20H"
",ss;   ;ss:   iss.       h5h        sss.        ,s1i      :shShr,          ,rh55s;.           \n"	
"i@@A   A@@G   #@@,      1@@@h       h@@&       :M@#i    5H@@@#@@@&s      rX@@@@@@@M3          \n"
" B@#. ,@@@B  ;@@&      .M@@@M.       s@@&     ,B@M;   .&@@Xi. .sH@@3    5@@Hr, .;G@@A.        \n"
" G@@; 1@#@@; 1@@S      3@@8@@9        s@@X   ,B@M:    9@@X      .M@@r   #@@;      XBMr        \n"
" h@@h 8@9G@5 3@@r     :@@& X@@:        s@@G .H@M:    .M@@r       9@@G   X@@X;                 \n"
" ;@@9 B@ih@& &@#.     X@@i i@@X         s@@GA@M:     ;@@@:       h@@H   .3M@@BGh;             \n"
"  B@&:@# :@#,B@&     r@@8   8@@r         s#@@B:      i@@#,       h@@B     .s9H@@@#Gs          \n"
"  8@B3@&  B@S#@S     H@@&888&@@B          X@@5       ;@@@:       5@@H         .iSB@@A;        \n"
"  1@@@@5  8@@@@i    5@@BAHHHAB@@5         G@@5       .M@@r       8@@G  ;hhi       S@@B.       \n"
"  :@@@@;  s@@@M.   ,#@@:     ,@@#,        G@@5        3@@X      .M@@i  S@@A       ,@@@.       \n"
"   H@@B   ,#@@X    8@@9       9@@8        G@@5        .&@@Xi  .rH@@S   .H@@8;   .rH@@S        \n"
"   8@@8    A@@S   r@@#,       ,#@@r       X@@S          5H@@#M@@@As     .3B@@@M#@@@&s         \n"
"   ;55;    i55,   ;ssi         ;ss;       iss:            ;s5S5s:          ;s5SShr,           \n"
;


