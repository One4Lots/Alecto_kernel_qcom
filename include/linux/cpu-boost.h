#ifndef _CPU_BOOST_H
#define _CPU_BOOST_H

#ifdef CONFIG_CPU_BOOST
void cpu_boost_set_refresh_rate(unsigned int fps);
#else
static inline void cpu_boost_set_refresh_rate(unsigned int fps) {}
#endif

#endif /* _CPU_BOOST_H */
