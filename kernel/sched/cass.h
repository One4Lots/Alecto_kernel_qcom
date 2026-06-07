/*SPDX-License-Identifier: GPL-2.0
* CASS COMPATIBILITY HEADER
* by deu
* i hope these stubs will be completed one by one!
*/
#ifndef _CASS_4_14_COMPAT
#define _CASS_4_14_COMPAT

#include <linux/sched.h>
#include <linux/cpuidle.h>

#ifndef arch_scale_min_freq_capacity
static inline unsigned long arch_scale_min_freq_capacity(int cpu)
{
    return arch_scale_cpu_capacity(cpu);
}
#endif
#endif /* _CASS_4_14_COMPAT */
