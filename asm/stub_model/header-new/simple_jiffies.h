/*
 * File Name: simple_jiffies.h
 * Comments: adapted from include/linux/jiifies.h and linux/kernel/time/time.c
 */

#ifndef __SIMPLE_JIFFIES_H__
#define __SIMPLE_JIFFIES_H__
#define MAX_JIFFY_OFFSET ((LONG_MAX >> 1)-1)
#define LONG_MAX ((long)(~0UL>>1))

unsigned int jiffies_to_usecs(const unsigned long j)
{
	/*
	 * Hz usually doesn't go much further MSEC_PER_SEC.
	 * jiffies_to_usecs() and usecs_to_jiffies() depend on that.
	 */
	BUILD_BUG_ON(HZ > USEC_PER_SEC);

#if !(USEC_PER_SEC % HZ)
	return (USEC_PER_SEC / HZ) * j;
#else
# if BITS_PER_LONG == 32
	return (HZ_TO_USEC_MUL32 * j) >> HZ_TO_USEC_SHR32;
# else
	return (j * HZ_TO_USEC_NUM) / HZ_TO_USEC_DEN;
# endif
#endif
}
//EXPORT_SYMBOL(jiffies_to_usecs);


#if !(USEC_PER_SEC % HZ)
static inline unsigned long _usecs_to_jiffies(const unsigned int u)
{
	return (u + (USEC_PER_SEC / HZ) - 1) / (USEC_PER_SEC / HZ);
}
#else
static inline unsigned long _usecs_to_jiffies(const unsigned int u)
{
	return (USEC_TO_HZ_MUL32 * u + USEC_TO_HZ_ADJ32)
		>> USEC_TO_HZ_SHR32;
}
#endif
unsigned long __usecs_to_jiffies(const unsigned int u)
{
	if (u > jiffies_to_usecs(MAX_JIFFY_OFFSET))
		return MAX_JIFFY_OFFSET;
	return _usecs_to_jiffies(u);
}
//EXPORT_SYMBOL(__usecs_to_jiffies);


/**
 * usecs_to_jiffies: - convert microseconds to jiffies
 * @u:	time in microseconds
 *
 * conversion is done as follows:
 *
 * - 'too large' values [that would result in larger than
 *   MAX_JIFFY_OFFSET values] mean 'infinite timeout' too.
 *
 * - all other values are converted to jiffies by either multiplying
 *   the input value by a factor or dividing it with a factor and
 *   handling any 32-bit overflows as for msecs_to_jiffies.
 *
 * usecs_to_jiffies() checks for the passed in value being a constant
 * via __builtin_constant_p() allowing gcc to eliminate most of the
 * code, __usecs_to_jiffies() is called if the value passed does not
 * allow constant folding and the actual conversion must be done at
 * runtime.
 * the HZ range specific helpers _usecs_to_jiffies() are called both
 * directly here and from __msecs_to_jiffies() in the case where
 * constant folding is not possible.
 */
static __always_inline unsigned long usecs_to_jiffies(const unsigned int u)
{
	if (__builtin_constant_p(u)) {
		if (u > jiffies_to_usecs(MAX_JIFFY_OFFSET))
			return MAX_JIFFY_OFFSET;
		return _usecs_to_jiffies(u);
	} else {
		return __usecs_to_jiffies(u);
	}
}

/*
 * Convert jiffies to milliseconds and back.
 *
 * Avoid unnecessary multiplications/divisions in the
 * two most common HZ cases:
 */
unsigned int jiffies_to_msecs(const unsigned long j)
{
	return (MSEC_PER_SEC / HZ) * j; // only for HZ ==1000
}

// only consider HZ=1000 cases
/*
 * HZ is equal to or smaller than 1000, and 1000 is a nice round
 * multiple of HZ, divide with the factor between them, but round
 * upwards:
 */
static inline unsigned long _msecs_to_jiffies(const unsigned int m)
{
	return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
}

unsigned long __msecs_to_jiffies(const unsigned int m)
{
	/*
	 * Negative value, means infinite timeout:
	 */
	if ((int)m < 0)
		return MAX_JIFFY_OFFSET;
	return _msecs_to_jiffies(m);
}

static __always_inline unsigned long msecs_to_jiffies(const unsigned int m)
{
	if (__builtin_constant_p(m)) {
		if ((int)m < 0)
			return MAX_JIFFY_OFFSET;
		return _msecs_to_jiffies(m);
	} else {
		return __msecs_to_jiffies(m);
	}
}

#endif
