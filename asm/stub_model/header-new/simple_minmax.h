/*
 * File Name: simple_minmax.h
 * Comments: adapted from include/linux/win_minmax.h
 */

#ifndef __SIMPLE_MINMAX_H__
#define __SIMPLE_MINMAX_H__
//struct minmax{
//};

/* A single data point for our parameterized min-max tracker */
struct minmax_sample {
	u32	t;	/* time measurement was taken */
	u32	v;	/* value measured */
};
/* State for the parameterized min-max tracker */
struct minmax {
	struct minmax_sample s[3];
};
static inline u32 minmax_get(const struct minmax *m)
{
	return m->s[0].v;
}

#endif
