/*
 * File Name: simple_sock.h
 * Comments: adapted from include/net/sock.h
 */

#ifndef __SIMPLE_SOCK_H__
#define __SIMPLE_SOCK_H__
#include "simple_net_namespace.h"

/* Sock flags */
enum sock_flags {
	SOCK_DEAD,
	SOCK_DONE,
	SOCK_URGINLINE,
	SOCK_KEEPOPEN,
	SOCK_LINGER,
	SOCK_DESTROY,
	SOCK_BROADCAST,
	SOCK_TIMESTAMP,
	SOCK_ZAPPED,
	SOCK_USE_WRITE_QUEUE, /* whether to call sk->sk_write_space in sock_wfree */
	SOCK_DBG, /* %SO_DEBUG setting */
	SOCK_RCVTSTAMP, /* %SO_TIMESTAMP setting */
	SOCK_RCVTSTAMPNS, /* %SO_TIMESTAMPNS setting */
	SOCK_LOCALROUTE, /* route locally only, %SO_DONTROUTE setting */
	SOCK_QUEUE_SHRUNK, /* write queue has been shrunk recently */
	SOCK_MEMALLOC, /* VM depends on this socket for swapping */
	SOCK_TIMESTAMPING_RX_SOFTWARE,  /* %SOF_TIMESTAMPING_RX_SOFTWARE */
	SOCK_FASYNC, /* fasync() active */
	SOCK_RXQ_OVFL,
	SOCK_ZEROCOPY, /* buffers from userspace */
	SOCK_WIFI_STATUS, /* push wifi status to userspace */
	SOCK_NOFCS, /* Tell NIC not to do the Ethernet FCS.
		     * Will use last 4 bytes of packet sent from
		     * user-space instead.
		     */
	SOCK_FILTER_LOCKED, /* Filter cannot be changed anymore */
	SOCK_SELECT_ERR_QUEUE, /* Wake select on error queue */
	SOCK_RCU_FREE, /* wait rcu grace period in sk_destruct() */
};

struct sock_common {

	volatile unsigned char	skc_state;
	possible_net_t		skc_net;
	unsigned long	skc_flags;
};

/**
  *	struct sock - network layer representation of sockets
  *	@__sk_common: shared layout with inet_timewait_sock
  *	@sk_shutdown: mask of %SEND_SHUTDOWN and/or %RCV_SHUTDOWN
  *	@sk_userlocks: %SO_SNDBUF and %SO_RCVBUF settings
  *	@sk_lock:	synchronizer
  *	@sk_kern_sock: True if sock is using kernel lock classes
  *	@sk_rcvbuf: size of receive buffer in bytes
  *	@sk_wq: sock wait queue and async head
  *	@sk_rx_dst: receive input route used by early demux
  *	@sk_dst_cache: destination cache
  *	@sk_dst_pending_confirm: need to confirm neighbour
  *	@sk_policy: flow policy
  *	@sk_receive_queue: incoming packets
  *	@sk_wmem_alloc: transmit queue bytes committed
  *	@sk_tsq_flags: TCP Small Queues flags
  *	@sk_write_queue: Packet sending queue
  *	@sk_omem_alloc: "o" is "option" or "other"
  *	@sk_wmem_queued: persistent queue size
  *	@sk_forward_alloc: space allocated forward
  *	@sk_napi_id: id of the last napi context to receive data for sk
  *	@sk_ll_usec: usecs to busypoll when there is no data
  *	@sk_allocation: allocation mode
  *	@sk_pacing_rate: Pacing rate (if supported by transport/packet scheduler)
  *	@sk_pacing_status: Pacing status (requested, handled by sch_fq)
  *	@sk_max_pacing_rate: Maximum pacing rate (%SO_MAX_PACING_RATE)
  *	@sk_sndbuf: size of send buffer in bytes
  *	@__sk_flags_offset: empty field used to determine location of bitfield
  *	@sk_padding: unused element for alignment
  *	@sk_no_check_tx: %SO_NO_CHECK setting, set checksum in TX packets
  *	@sk_no_check_rx: allow zero checksum in RX packets
  *	@sk_route_caps: route capabilities (e.g. %NETIF_F_TSO)
  *	@sk_route_nocaps: forbidden route capabilities (e.g NETIF_F_GSO_MASK)
  *	@sk_gso_type: GSO type (e.g. %SKB_GSO_TCPV4)
  *	@sk_gso_max_size: Maximum GSO segment size to build
  *	@sk_gso_max_segs: Maximum number of GSO segments
  *	@sk_pacing_shift: scaling factor for TCP Small Queues
  *	@sk_lingertime: %SO_LINGER l_linger setting
  *	@sk_backlog: always used with the per-socket spinlock held
  *	@sk_callback_lock: used with the callbacks in the end of this struct
  *	@sk_error_queue: rarely used
  *	@sk_prot_creator: sk_prot of original sock creator (see ipv6_setsockopt,
  *			  IPV6_ADDRFORM for instance)
  *	@sk_err: last error
  *	@sk_err_soft: errors that don't cause failure but are the cause of a
  *		      persistent failure not just 'timed out'
  *	@sk_drops: raw/udp drops counter
  *	@sk_ack_backlog: current listen backlog
  *	@sk_max_ack_backlog: listen backlog set in listen()
  *	@sk_uid: user id of owner
  *	@sk_priority: %SO_PRIORITY setting
  *	@sk_type: socket type (%SOCK_STREAM, etc)
  *	@sk_protocol: which protocol this socket belongs in this network family
  *	@sk_peer_pid: &struct pid for this socket's peer
  *	@sk_peer_cred: %SO_PEERCRED setting
  *	@sk_rcvlowat: %SO_RCVLOWAT setting
  *	@sk_rcvtimeo: %SO_RCVTIMEO setting
  *	@sk_sndtimeo: %SO_SNDTIMEO setting
  *	@sk_txhash: computed flow hash for use on transmit
  *	@sk_filter: socket filtering instructions
  *	@sk_timer: sock cleanup timer
  *	@sk_stamp: time stamp of last packet received
  *	@sk_stamp_seq: lock for accessing sk_stamp on 32 bit architectures only
  *	@sk_tsflags: SO_TIMESTAMPING socket options
  *	@sk_tskey: counter to disambiguate concurrent tstamp requests
  *	@sk_zckey: counter to order MSG_ZEROCOPY notifications
  *	@sk_socket: Identd and reporting IO signals
  *	@sk_user_data: RPC layer private data
  *	@sk_frag: cached page frag
  *	@sk_peek_off: current peek_offset value
  *	@sk_send_head: front of stuff to transmit
  *	@sk_security: used by security modules
  *	@sk_mark: generic packet mark
  *	@sk_cgrp_data: cgroup data for this cgroup
  *	@sk_memcg: this socket's memory cgroup association
  *	@sk_write_pending: a write to stream socket waits to start
  *	@sk_state_change: callback to indicate change in the state of the sock
  *	@sk_data_ready: callback to indicate there is data to be processed
  *	@sk_write_space: callback to indicate there is bf sending space available
  *	@sk_error_report: callback to indicate errors (e.g. %MSG_ERRQUEUE)
  *	@sk_backlog_rcv: callback to process the backlog
  *	@sk_destruct: called at sock freeing time, i.e. when all refcnt == 0
  *	@sk_reuseport_cb: reuseport group container
  *	@sk_rcu: used during RCU grace period
  *	@sk_clockid: clockid used by time-based scheduling (SO_TXTIME)
  *	@sk_txtime_deadline_mode: set deadline mode for SO_TXTIME
  *	@sk_txtime_unused: unused txtime flags
  */
struct sock {
	struct sock_common	__sk_common;
	#define sk_net		__sk_common.skc_net
	#define sk_state	__sk_common.skc_state
	#define sk_flags	__sk_common.skc_flags

	int			sk_rcvbuf;
	int			sk_sndbuf;
	

// <M>
	//struct dst_entry	*sk_rx_dst;
	//struct dst_entry __rcu	*sk_dst_cache;
	//atomic_t		sk_omem_alloc;
	//int			sk_sndbuf;
	/* ===== cache line for TX ===== */
	//int			sk_wmem_queued;
	//refcount_t		sk_wmem_alloc;
	//unsigned long		sk_tsq_flags;
	//union {
		//struct sk_buff	*sk_send_head;
		//struct rb_root	tcp_rtx_queue;
	//};
	//struct sk_buff_head	sk_write_queue;
	//__s32			sk_peek_off;
	//int			sk_write_pending;
	__u32			sk_dst_pending_confirm;
	//u32			sk_pacing_status; /* see enum sk_pacing */
	//long			sk_sndtimeo;
	//struct timer_list	sk_timer;
	//__u32			sk_priority;
	//__u32			sk_mark;
	//unsigned long		sk_pacing_rate; /* bytes per second */
	//unsigned long		sk_max_pacing_rate;
	//struct page_frag	sk_frag;
	//netdev_features_t	sk_route_caps;
	//netdev_features_t	sk_route_nocaps;
	//netdev_features_t	sk_route_forced_caps;
	//int			sk_gso_type;
	//unsigned int		sk_gso_max_size;
	//gfp_t			sk_allocation;
	//__u32			sk_txhash;	
	
#define SK_PROTOCOL_MAX U8_MAX
	//u16			sk_gso_max_segs;
	//u8			sk_pacing_shift;
	//unsigned long	        sk_lingertime;
	//struct proto		*sk_prot_creator;
	//rwlock_t		sk_callback_lock;
	int			sk_err,
				sk_err_soft;
	//u32			sk_ack_backlog;
	//u32			sk_max_ack_backlog;
	//kuid_t			sk_uid;
	//struct pid		*sk_peer_pid;
	//const struct cred	*sk_peer_cred;
	//long			sk_rcvtimeo;
	//ktime_t			sk_stamp;
};

void SOCK_DEBUG(const struct sock *sk, const char *msg, ...)
{
}

static inline
struct net *sock_net(const struct sock *sk)
{
	return read_pnet(&sk->sk_net);
}

static inline void sock_set_flag(struct sock *sk, enum sock_flags flag)
{
	__set_bit(flag, &sk->sk_flags);
}

// <M>
static inline void sk_dst_confirm(struct sock *sk)
{
	if (!sk->sk_dst_pending_confirm)
		sk->sk_dst_pending_confirm = 1;
}

#endif
