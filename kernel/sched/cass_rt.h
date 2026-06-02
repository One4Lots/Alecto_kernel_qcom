// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2024 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

#ifdef CONFIG_SCHED_CASS
int cass_select_task_rq_rt(struct task_struct *p, int prev_cpu, int wake_flags);

/* Use CASS. A dummy wrapper ensures the replaced function is still "used". */
static inline void *select_task_rq_rt_dummy(void)
{
	return (void *)select_task_rq_rt;
}
/* compatibility shim for 4.14 */
static int select_task_rq_rt_compat(struct task_struct *p, int prev_cpu,
				   int sd_flag, int wake_flags, int sibling_count_hint)
{
	return cass_select_task_rq_rt(p, prev_cpu, wake_flags);
}
#define select_task_rq_rt select_task_rq_rt_compat
#endif /* CONFIG_SCHED_CASS */
