#ifndef simple_tcp_ipv4
#define simple_tcp_ipv4
#include "share.h"
static void __net_exit tcp_sk_exit(struct net *net)
{
	return;
}

static int __net_init tcp_sk_init(struct net *net)
{
	return 0;
}

#endif /* ifndef simple_tcp_ipv4 */
