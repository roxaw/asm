/*
 * File Name: simple_tcp_sock.h
 * Comments: adapted from include/linux/tcp.h
 */

#ifndef __SIMPLE_TCP_SOCK_H__
#define __SIMPLE_TCP_SOCK_H__

static inline struct tcphdr *tcp_hdr(const struct sk_buff *skb)
{
	return (struct tcphdr *)skb_transport_header(skb);
}

/* This defines a selective acknowledgement block. */
struct tcp_sack_block_wire {
	__be32	start_seq;
	__be32	end_seq;
};

struct tcp_sack_block {
	u32	start_seq;
	u32	end_seq;
};

struct tcp_options_received {
/*	PAWS/RTTM data	*/
	int	ts_recent_stamp;/* Time we stored ts_recent (for aging) */
	u32	ts_recent;	/* Time stamp to echo next		*/
	u32	rcv_tsval;	/* Time stamp value             	*/
	u32	rcv_tsecr;	/* Time stamp echo reply        	*/
	u16 saw_tstamp : 1,	/* Saw TIMESTAMP on last packet		*/
		tstamp_ok : 1,	/* TIMESTAMP seen on SYN packet		*/
		dsack : 1,	/* D-SACK is scheduled			*/
		wscale_ok : 1,	/* Wscale seen on SYN packet		*/
		sack_ok : 4,	/* SACK seen on SYN packet		*/
		snd_wscale : 4,	/* Window scaling received from sender	*/
		rcv_wscale : 4;	/* Window scaling to send to receiver	*/
	u8	num_sacks;	/* Number of SACK blocks		*/
	u16	user_mss;	/* mss requested by user in ioctl	*/
	u16	mss_clamp;	/* Maximal mss, negotiated at connection setup */
};

struct tcp_sock {
	/* inet_connection_sock has to be the first member of tcp_sock */
	struct inet_connection_sock	inet_conn;
	u16	tcp_header_len;	/* Bytes of tcp header to send		*/
	u16	gso_segs;	/* Max number of segs per GSO packet	*/

/*
 *	Header prediction flags
 *	0x5?10 << 16 + snd_wnd in net byte order
 */
	__be32	pred_flags;

/*
 *	RFC793 variables by their proper names. This means you can
 *	read the code and the spec side by side (and laugh ...)
 *	See RFC793 and RFC1122. The RFC writes these in capitals.
 */
	u64	bytes_received;	/* RFC4898 tcpEStatsAppHCThruOctetsReceived
						 * sum(delta(rcv_nxt)), or how many bytes
						 * were acked.
						 */
	u32	segs_in;		/* RFC4898 tcpEStatsPerfSegsIn
						 * total number of segments in.
						 */
	u32	data_segs_in;	/* RFC4898 tcpEStatsPerfDataSegsIn
						 * total number of data segments in.
						 */
 	u32	rcv_nxt;		/* What we want to receive next 	*/
	u32	copied_seq;		/* Head of yet unread data		*/
	u32	rcv_wup;		/* rcv_nxt on last window update sent	*/
 	u32	snd_nxt;		/* Next sequence we send		*/
	u32	segs_out;		/* RFC4898 tcpEStatsPerfSegsOut
						 * The total number of segments sent.
						 */
	u32	data_segs_out;	/* RFC4898 tcpEStatsPerfDataSegsOut
						 * total number of data segments sent.
						 */
	u64	bytes_sent;		/* RFC4898 tcpEStatsPerfHCDataOctetsOut
						 * total number of data bytes sent.
						 */
	u64	bytes_acked;	/* RFC4898 tcpEStatsAppHCThruOctetsAcked
						 * sum(delta(snd_una)), or how many bytes
						 * were acked.
						 */
	u32	dsack_dups;		/* RFC4898 tcpEStatsStackDSACKDups
						 * total number of DSACK blocks received
						 */					 
 	u32	snd_una;		/* First byte we want an ack for	*/
 	u32	snd_sml;		/* Last byte of the most recently transmitted small packet */
	u32	rcv_tstamp;		/* timestamp of last received ACK (for keepalives) */
	u32	lsndtime;		/* timestamp of last sent data packet (for restart window) */
	u32	last_oow_ack_time;  /* timestamp of last out-of-window ACK */
	u32	compressed_ack_rcv_nxt;

	u32	tsoffset;	/* timestamp offset */

//	struct list_head tsq_node; /* anchor in tsq_tasklet.head list */
//	struct list_head tsorted_sent_queue; /* time-sorted sent but un-SACKed skbs */

	u32	snd_wl1;	/* Sequence for window update		*/
	u32	snd_wnd;	/* The window we expect to receive	*/
	u32	max_window;	/* Maximal window ever seen from peer	*/
	u32	mss_cache;	/* Cached effective mss, not including SACKS */

	u32	window_clamp;	/* Maximal window to advertise		*/
	u32	rcv_ssthresh;	/* Current window clamp			*/

	/* Information of the most recently (s)acked skb */
	struct tcp_rack {
//		struct skb_mstamp mstamp; /* (Re)sent time of the skb */
		u64 mstamp; /* (Re)sent time of the skb */
		u32 rtt_us;  /* Associated RTT */
		u32 end_seq; /* Ending TCP sequence of the skb */
//		u8 advanced; /* mstamp advanced since last lost marking */
//		u8 reord;    /* reordering detected */
		u32 last_delivered; /* tp->delivered at last reo_wnd adj */
		u8 reo_wnd_steps;   /* Allowed reordering window */
#define TCP_RACK_RECOVERY_THRESH 16
		u8 reo_wnd_persist:5, /* No. of recovery since last adj */
		   dsack_seen:1, /* Whether DSACK seen after last adj */
		   advanced:1;	 /* mstamp advanced since last lost marking */
	} rack;
	u16	advmss;		/* Advertised MSS			*/
	u8	compressed_ack;
	u32	chrono_start;	/* Start time in jiffies of a TCP chrono */
	u32	chrono_stat[3];	/* Time in jiffies for chrono_stat stats */
//	u8	chrono_type:2,	/* current chronograph type */
//		rate_app_limited:1,  /* rate_{delivered,interval_us} limited? */
//		fastopen_connect:1, /* FASTOPEN_CONNECT sockopt */
//		unused:4;
	u8	chrono_type:2,	/* current chronograph type */
		rate_app_limited:1,  /* rate_{delivered,interval_us} limited? */
		fastopen_connect:1, /* FASTOPEN_CONNECT sockopt */
		fastopen_no_cookie:1, /* Allow send/recv SYN+data without a cookie */
		is_sack_reneg:1,    /* in recovery from loss with SACK reneg? */
		unused:2;	
	u8	nonagle     : 4,/* Disable Nagle algorithm?             */
		thin_lto    : 1,/* Use linear timeouts for thin streams */
		unused1	    : 1,
		repair      : 1,
		frto        : 1;/* F-RTO (RFC5682) activated in CA_Loss */
	u8	repair_queue;
	u8	syn_data:1,	/* SYN includes data */
		syn_fastopen:1,	/* SYN includes Fast Open option */
		syn_fastopen_exp:1,/* SYN includes Fast Open exp. option */
		syn_fastopen_ch:1, /* Active TFO re-enabling probe */
		syn_data_acked:1,/* data in SYN is acked by SYN-ACK */
		save_syn:1,	/* Save headers of SYN packet */
		is_cwnd_limited:1;/* forward progress limited by snd_cwnd? */
	u32	tlp_high_seq;	/* snd_nxt at the time of TLP retransmit. */
	
	u64	tcp_wstamp_ns;	/* departure time for next sent data packet */
	u64	tcp_clock_cache; /* cache last tcp_clock_ns() (see tcp_mstamp_refresh()) */

/* RTT measurement */
	//struct skb_mstamp tcp_mstamp; /* most recent packet received/sent */
	u64	tcp_mstamp;	/* most recent packet received/sent */ //Updated to latest kernel
	u32	srtt_us;	/* smoothed round trip time << 3 in usecs */
	u32	mdev_us;	/* medium deviation			*/
	u32	mdev_max_us;	/* maximal mdev for the last rtt period	*/
	u32	rttvar_us;	/* smoothed mdev_max			*/
	u32	rtt_seq;	/* sequence number to update rttvar	*/
	struct  minmax rtt_min;

	u32	packets_out;	/* Packets which are "in flight"	*/
	u32	retrans_out;	/* Retransmitted packets out		*/
	u32	max_packets_out;  /* max packets_out in last window */
	u32	max_packets_seq;  /* right edge of max_packets_out flight */

	u16	urg_data;	/* Saved octet of OOB data and control flags */
	u8	ecn_flags;	/* ECN status bits.			*/
	u8	keepalive_probes; /* num of allowed keep alive probes	*/
	u32	reordering;	/* Packet reordering metric.		*/
	u32	reord_seen;	/* number of data packet reordering events */
	u32	snd_up;		/* Urgent pointer		*/

/*
 *      Options received (usually on last packet, some only on SYN packets).
 */
	struct tcp_options_received rx_opt;

/*
 *	Slow start and congestion control (see also Nagle, and Karn & Partridge)
 */
 	u32	snd_ssthresh;	/* Slow start size threshold		*/
 	u32	snd_cwnd;	/* Sending congestion window		*/
	u32	snd_cwnd_cnt;	/* Linear increase counter		*/
	u32	snd_cwnd_clamp; /* Do not allow snd_cwnd to grow above this */
	u32	snd_cwnd_used;
	u32	snd_cwnd_stamp;
	u32	prior_cwnd;	/* cwnd right before starting loss recovery */
	u32	prr_delivered;	/* Number of newly delivered packets to
				 * receiver in Recovery. */
	u32	prr_out;	/* Total number of pkts sent during Recovery. */
	u32	delivered;	/* Total data packets delivered incl. rexmits */
	u32	delivered_ce;	/* Like the above but only ECE marked packets */
	u32	lost;		/* Total data packets lost incl. rexmits */
	u32	app_limited;	/* limited until "delivered" reaches this val */
//	struct skb_mstamp first_tx_mstamp;  /* start of window send phase */
//	struct skb_mstamp delivered_mstamp; /* time we reached "delivered" */
	u64	first_tx_mstamp;  /* start of window send phase */
	u64	delivered_mstamp; /* time we reached "delivered" */
	u32	rate_delivered;    /* saved rate sample: packets delivered */
	u32	rate_interval_us;  /* saved rate sample: time elapsed */

 	u32	rcv_wnd;	/* Current receiver window		*/
	u32	write_seq;	/* Tail(+1) of data held in tcp send buffer */
	u32	notsent_lowat;	/* TCP_NOTSENT_LOWAT */
	u32	pushed_seq;	/* Last pushed seq, required to talk to windows */
	u32	lost_out;	/* Lost packets			*/
	u32	sacked_out;	/* SACK'd packets			*/
	u32	fackets_out;	/* FACK'd packets			*/

	// <M>
	int num_packets; /* Number of consecutive packets for aggregation */
	u32 one_acked; /* Size of one acked */
	u32 cwnd_counter; /* For modified congestion avoidance, only increase cwnd after a RTT */
	u32 acked_counter; /* For modified congestion avoidance, only increase cwnd after a RTT */

	/* from STCP, retrans queue hinting */
	struct sk_buff* lost_skb_hint;
	struct sk_buff *retransmit_skb_hint;

	/* OOO segments go in this rbtree. Socket lock must be held. */
	struct rb_root	out_of_order_queue;
	struct sk_buff	*ooo_last_skb; /* cache rb_last(out_of_order_queue) */

	/* SACKs data, these 2 need to be together (see tcp_options_write) */
	struct tcp_sack_block duplicate_sack[1]; /* D-SACK block */
	struct tcp_sack_block selective_acks[4]; /* The SACKS themselves*/

	struct tcp_sack_block recv_sack_cache[4];

	struct sk_buff *highest_sack;   /* skb just after the highest
					 * skb with SACKed bit set
					 * (validity guaranteed only if
					 * sacked_out > 0)
					 */

	int     lost_cnt_hint;

	u32	prior_ssthresh; /* ssthresh saved at recovery start	*/
	u32	high_seq;	/* snd_nxt at onset of congestion	*/

	u32	retrans_stamp;	/* Timestamp of the last retransmit,
				 * also used in SYN-SENT to remember stamp of
				 * the first SYN. */
	u32	undo_marker;	/* snd_una upon a new recovery episode. */
	int	undo_retrans;	/* number of undoable retransmissions. */
	u32	total_retrans;	/* Total retransmits for entire connection */

	u32	urg_seq;	/* Seq of received urgent pointer */
	unsigned int		keepalive_time;	  /* time before keep alive takes place */
	unsigned int		keepalive_intvl;  /* time interval between keep alive probes */

	int			linger2;

/* Receiver side RTT estimation */
	struct {
		u32		rtt_us;
		u32		seq;
		struct skb_mstamp time;
	} rcv_rtt_est;

/* Receiver queue space */
	struct {
		int		space;
		u32		seq;
		struct skb_mstamp time;
	} rcvq_space;

/* TCP-specific MTU probe information. */
	struct {
		u32		  probe_seq_start;
		u32		  probe_seq_end;
	} mtu_probe;
	u32	mtu_info; /* We received an ICMP_FRAG_NEEDED / ICMPV6_PKT_TOOBIG
			   * while socket was owned by user.
			   */


/* TCP fastopen related information */
	struct tcp_fastopen_request *fastopen_req;
	/* fastopen_rsk points to request_sock that resulted in this big
	 * socket. Used to retransmit SYNACKs etc.
	 */
	struct request_sock *fastopen_rsk;
	u32	*saved_syn;
};

static inline void tcp_clear_options(struct tcp_options_received *rx_opt)
{
	rx_opt->tstamp_ok = rx_opt->sack_ok = 0;
	rx_opt->wscale_ok = rx_opt->snd_wscale = 0;
}

static inline struct tcp_sock *tcp_sk(const struct sock *sk)
{
	return (struct tcp_sock *)sk;
}
#endif
