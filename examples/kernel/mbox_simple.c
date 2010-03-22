/*
 * 程序清单：邮箱例程
 *
 * 这个程序会创建2个动态线程，一个静态的邮箱对象，其中一个线程往邮箱中发送邮件，
 * 一个线程往邮箱中收取邮件。
 */
#include <rtthread.h>
#include "tc_comm.h"

/* 指向线程控制块的指针 */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;

static struct rt_mailbox mb;
static char mb_pool[128];
static char mb_str1[] = "I'm a mail!";
static char mb_str2[] = "this is another mail!";

/* 线程1入口 */
static void thread1_entry(void* parameter)
{
	unsigned char* str;

	while (1)
	{
		rt_kprintf("thread1: try to recv a mail\n");
		if (rt_mb_recv(&mb, (rt_uint32_t*)&str, RT_WAITING_FOREVER) == RT_EOK)
		{
			rt_kprintf("thread1: get a mail from mailbox, the content:%s\n", str);

			rt_thread_delay(100);
		}
	}
}

/* 线程2入口 */
static void thread2_entry(void* parameter)
{
	rt_uint8_t count;

	count = 0;
	while (1)
	{
		count ++;
		if (count & 0x1)
		{
			rt_mb_send(&mb, (rt_uint32_t)&mb_str1[0]);
		}
		else
		{
			rt_mb_send(&mb, (rt_uint32_t)&mb_str2[0]);
		}

		rt_thread_delay(200);
	}
}

int mbox_simple_init()
{
	/* 初始化一个mailbox */
	rt_mb_init(&mb, "mbt", &mb_pool[0], 128 / 4, RT_IPC_FLAG_FIFO);

	/* 创建线程1 */
	tid1 = rt_thread_create("t1",
		thread1_entry, RT_NULL, /* 线程入口是thread1_entry, 入口参数是RT_NULL */
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid1 != RT_NULL)
		rt_thread_startup(tid1);
	else
		tc_stat(TC_STAT_END | TC_STAT_FAILED);

	/* 创建线程2 */
	tid2 = rt_thread_create("t2",
		thread2_entry, RT_NULL, /* 线程入口是thread2_entry, 入口参数是RT_NULL */
		THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
	if (tid2 != RT_NULL)
		rt_thread_startup(tid2);
	else
		tc_stat(TC_STAT_END | TC_STAT_FAILED);

	return 0;
}

#ifdef RT_USING_TC
static void _tc_cleanup()
{
	/* 调度器上锁，上锁后，将不再切换到其他线程，仅响应中断 */
	rt_enter_critical();

	/* 删除线程 */
	if (tid1 != RT_NULL && tid1->stat != RT_THREAD_CLOSE)
		rt_thread_delete(tid1);
	if (tid2 != RT_NULL && tid2->stat != RT_THREAD_CLOSE)
		rt_thread_delete(tid2);

	/* 调度器解锁 */
	rt_exit_critical();

	/* 设置TestCase状态 */
	tc_done(TC_STAT_PASSED);
}

int _tc_mbox_simple()
{
	/* 设置TestCase清理回调函数 */
	tc_cleanup(_tc_cleanup);
	mbox_simple_init();

	/* 返回TestCase运行的最长时间 */
	return 100;
}
/* 输出函数命令到finsh shell中 */
FINSH_FUNCTION_EXPORT(_tc_mbox_simple, a simple mailbox example);
#else
/* 用户应用入口 */
int rt_application_init()
{
	mbox_simple_init();

	return 0;
}
#endif