/*
 * File Name: simple_kernel.h
 * Comments: adapted from include/kernel.h and tools/include/linux/kernel.h
 */

#ifndef __SIMPLE_KERNEL_H__
#define __SIMPLE_KERNEL_H__

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
/*
 * min()/max()/clamp() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define __min(t1, t2, min1, min2, x, y) ({		\
	t1 min1 = (x);					\
	t2 min2 = (y);					\
	(void) (&min1 == &min2);			\
	min1 < min2 ? min1 : min2; })

u32 min_func(u32 x, u32 y)
{
	if (x < y)
		return x;
	else
		return y;
}

u32 max_func(u32 x, u32 y)
{
	if (x > y)
		return x;
	else
		return y;
}

#define min(x, y)					\
	min_func(x,y)                  \
   /* __min(typeof(x), typeof(y),			\*/
		  //__UNIQUE_ID(min1_), __UNIQUE_ID(min2_),	\
		  /*x, y)*/

#define __max(t1, t2, max1, max2, x, y) ({		\
	t1 max1 = (x);					\
	t2 max2 = (y);					\
	(void) (&max1 == &max2);			\
	max1 > max2 ? max1 : max2; })
#define max(x, y)					\
	max_func(x,y)                       \
   /* __max(typeof(x), typeof(y),			\*/
		  //__UNIQUE_ID(max1_), __UNIQUE_ID(max2_),	\
		  /*x, y)*/

#define min3(x, y, z) min((typeof(x))min(x, y), z)
#define max3(x, y, z) max((typeof(x))max(x, y), z)

/**
 * clamp_t - return a value clamped to a given range using a given type
 * @type: the type of variable to use
 * @val: current value
 * @lo: minimum allowable value
 * @hi: maximum allowable value
 *
 * This macro does no typechecking and uses temporary variables of type
 * 'type' to make all the comparisons.
 */
#define clamp_t(type, val, lo, hi) min_t(type, max_t(type, val, lo), hi)

/**
 * clamp - return a value clamped to a given range with strict typechecking
 * @val: current value
 * @lo: lowest allowable value
 * @hi: highest allowable value
 *
 * This macro does strict typechecking of lo/hi to make sure they are of the
 * same type as val.  See the unnecessary pointer comparisons.
 */
#define clamp(val, lo, hi) min((typeof(val))max(val, lo), hi)


/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max/clamp at all, of course.
 */
#define min_t(type, x, y)				\
	__min(type, type,				\
	      __UNIQUE_ID(min1_), __UNIQUE_ID(min2_),	\
	      x, y)

#define max_t(type, x, y)				\
	__max(type, type,				\
	      __UNIQUE_ID(min1_), __UNIQUE_ID(min2_),	\
	      x, y)

#ifndef UINT_MAX
#define UINT_MAX (~0U)
#endif /* ifndef UINT_MAX */
#define WARN_ON(x) ({int val = !!(x); if (val) printf("WARN_ON detected!\n"); val;})
#endif
