/*************************************************************************
    > File Name: simple_rbtree.h
  > Author: Wei Sun
  > Mail:sunweiflyus@gmail.com 
  > Created Time: Fri 23 Jun 2017 10:54:28 PM CDT
  > Comments: 
 ************************************************************************/

#ifndef __SIMPLE_RBTREE_H__
#define __SIMPLE_RBTREE_H__

struct rb_node {
	unsigned long  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
    /* The alignment might seem pointless, but allegedly CRIS needs it */

struct rb_root {
	struct rb_node *rb_node;
};


#endif
