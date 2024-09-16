/*************************************************************************
    > File Name: simple_net_namespace.h
  > Author: Wei Sun
  > Mail:sunweiflyus@gmail.com 
  > Created Time: Fri 23 Jun 2017 11:50:51 PM CDT
  > Comments: 
 ************************************************************************/

#ifndef __SIMPLE_NET_NAMESPACE_H__
#define __SIMPLE_NET_NAMESPACE_H__
#include "simple_netns_ipv4.h"

#define CONFIG_NET_NS 1
struct net{
struct netns_ipv4 ipv4;
};

typedef struct {
#ifdef CONFIG_NET_NS
	struct net *net;
#endif
} possible_net_t;

static inline struct net *read_pnet(const possible_net_t *pnet)
{
#ifdef CONFIG_NET_NS
	return pnet->net;
#else
	//return &init_net;
#endif
}
#define __net_init
#define __net_exit

#endif
