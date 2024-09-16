/*************************************************************************
    > File Name: simple_compiler.h
  > Author: Wei Sun
  > Mail:sunweiflyus@gmail.com 
  > Created Time: Fri 23 Jun 2017 03:49:20 PM CDT
  > Comments: Adpated from compiler_clang.h for LLVM used by Klee
 ************************************************************************/

#ifndef __SIMPLE_COMPILER_H__
#define __SIMPLE_COMPILER_H__

#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)

/* same as gcc, this was present in clang-2.6 so we can assume it works
 * with any version that can compile the kernel
 */
#define __UNIQUE_ID(prefix) __PASTE(__PASTE(__UNIQUE_ID_, prefix), __COUNTER__)
#define __aligned(x) __attribute__((aligned(x)))
#define __force   __attribute__((force))
#define __must_be_array(a) 0
#endif
