#ifndef __SHARE_H__
#define __SHARE_H__

#define KLEE
#define DEBUG_INFO  false 


#define __read_mostly
#define __always_inline   inline
#define HZ 1000 
// This project assumes that HZ is 1000, 
// and CUBIC calls msecs_to_jiffies and ca->delay_min in msec instead of usec  
// TODO: 
// 		if change HZ to a number higher than 1000, 
// 		then need to define constants like HZ_TO_USEC_NUM, ...
// 		and CUBIC calls usecs_to_jiffies(
#define BITS_PER_LONG  64
#include <linux/types.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
unsigned long volatile jiffies = 100;  // initial jiffies
#define __init
#define __exit
#define tcp_time_stamp   ((__u32)(jiffies))
#define BUILD_BUG_ON(x)
#define THIS_MODULE ((struct module *)0)
#define likely(x) x
#define unlikely(x) x	
#define pr_err_once(fmt, ...)
#define WARN_ON_ONCE(x) ({int val = !!(x); if (val) printf("WARN_ON detected!\n"); val;})

typedef unsigned char 		u8;
typedef unsigned short 		u16;
typedef unsigned int 		u32;
typedef char 				s8;
typedef short 				s16;
typedef int 				s32;

// <M>
// from include/uapi/linux/types.h
#ifdef __CHECKER__
#define __bitwise__ __attribute__((bitwise))
#else
#define __bitwise__
#endif
#define __bitwise __bitwise__
// from include/linux/types.h
typedef unsigned int __bitwise gfp_t;
// <M>

#ifdef __GNUC__
typedef long long s64;
typedef unsigned long long u64;
#else
typedef long long s64;
typedef unsigned long long u64;
#endif
//all below header files are original files from linux kernel
#ifdef KLEE
#include "klee.h"
#endif
#ifdef KLEE
char debug_buf[1024];
#define error_msg(msg) {sprintf(debug_buf, "\x1B[31m%s !\x1B[0m\n", msg); klee_id(debug_buf);};
#define important_msg(msg) klee_id(msg); 
#define debug_fun() if (DEBUG_INFO)  {sprintf(debug_buf, "[File] %s : [Func] %s: Calling.\n", __FILE__, __func__); klee_id(debug_buf);};
#define debug_msg(msg) if (DEBUG_INFO)   klee_id(msg); 
#define debug_format(format, ...)  { };
#else
#define error_msg(msg)	printf("\x1B[31m%s !\x1B[0m\n", msg);
#define important_msg(msg)   printf("%s", msg); 
#define debug_fun() if (DEBUG_INFO)   printf("[File] %s : [Func] %s: Calling.\n", __FILE__, __func__); 
#define debug_msg(msg) if (DEBUG_INFO)   printf("%s", msg); 
#define debug_format(format, ...)  if (DEBUG_INFO) printf(format, ##__VA_ARGS__);
#endif


#include "fls.h"
#include "__fls.h"
#include "fls64.h"
#include "time64.h"
#include "tcp_states.h"

//all below header files are changed accordingly 
#include "simple_bitops.h"
#include "simple_jiffies.h"
#include "simple_minmax.h"
#include "simple_rbtree.h"
#include "simple_sock.h"
#include "simple_compiler.h"
#include "simple_inet_ecn.h"
#include "simple_module.h"
#include "simple_export.h"
#include "simple_kernel.h"
#include "simple_ip.h"
#include "simple_inet_sock.h"
#include "simple_inet_diag.h"
#include "simple_inet_connection_sock.h"
#include "simple_skbuff.h"
#include "simple_tcp_sock.h"
#include "simple_tcp2.h"
#include "simple_tcp.h"

/* This original do_div() is not C FUNCTION and is assemble language which klee cannot support;
 * Convert it to glibc implmenation, also this function is only called without any symbolic variables;
 */

//#define do_div(n, base) div(&n, base)

#define do_div(n, base) {debug_msg("Long division\n"); n = n/(base);}


/*
 * Before uncommenting this function, comment #include <stdlib.h> 
 */
//u32
//div (u64* numer, int denom)
//{
 
  //u64 res  = *numer / denom;
  //u32 rem = *numer % denom;
  ////printf("res:%llu\n",res);
  //*numer = res;
  ////printf("number:%llu\n",*numer);
  //return rem;
//}

#if BITS_PER_LONG == 64
/**
 * div_u64_rem - unsigned 64bit divide with 32bit divisor with remainder
 *
 * This is commonly provided by 32bit archs to provide an optimized 64bit
 * divide.
 */
static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/*#elif BITS_PER_LONG == 32*/
//#ifndef div_u64_rem
//#define div_u64_rem
//static inline u64 div_u64_rem(u64 dividend, u32 divisor, u32 *remainder)
//{
////	*remainder = do_div(dividend, divisor); // defalut is 64 bit
	//return dividend;
//}
/*#endif*/
#endif /* BITS_PER_LONG */

/**
 * div_u64 - unsigned 64bit divide with 32bit divisor
 *
 * This is the most common 64bit divide and should be used if possible,
 * as many 32bit archs can optimize this variant better than a full 64bit
 * divide.
 */
//#ifndef div_u64
//#define div_u64
static inline u64 div_u64(u64 dividend, u32 divisor)
{
	u32 remainder;
	return div_u64_rem(dividend, divisor, &remainder);
}
//#endif

/**
 * div64_u64 - unsigned 64bit divide with 64bit divisor
 * @dividend:	64bit dividend
 * @divisor:	64bit divisor
 *
 * This implementation is a modified version of the algorithm proposed
 * by the book 'Hacker's Delight'.  The original source and full proof
 * can be found here and is available for use without restriction.
 *
 * 'http://www.hackersdelight.org/HDcode/newCode/divDouble.c.txt'
 */
u64 div64_u64(u64 dividend, u64 divisor)
{
	u32 high = divisor >> 32;
	u64 quot;

	if (high == 0) {
		quot = div_u64(dividend, divisor);
	} else {
		int n = 1 + fls(high);
		quot = div_u64(dividend >> n, divisor >> n);

		if (quot != 0)
			quot--;
		if ((dividend - quot * divisor) >= divisor)
			quot++;
	}

	return quot;
}


#include "simple_tcp_cong.c"
#include "simple_tcp.c"
#include "simple_tcp_ipv4.c"



#endif

