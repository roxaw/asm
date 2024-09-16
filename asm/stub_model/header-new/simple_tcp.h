/*
 * File Name: simple_tcp.h
 * Comments: adapted from include/net/tcp.h
 */

#ifndef __SIMPLE_TCP_H__
#define __SIMPLE_TCP_H__
//#define TCP_SKB_CB(__skb)›  ((struct tcp_skb_cb *)&((__skb)->cb[0]))
#define TCP_SKB_CB(__skb)	((struct tcp_skb_cb *)&((__skb)->cb[0]))

struct tcp_skb_cb {
	__u32		seq;		/* Starting sequence number	*/
	__u32		end_seq;	/* SEQ + FIN + SYN + datalen	*/
	union {
		/* Note : tcp_tw_isn is used in input path only
		 *	  (isn chosen by tcp_timewait_state_process())
		 *
		 *	  tcp_gso_segs/size are used in write queue only,
		 *	  cf tcp_skb_pcount()/tcp_skb_mss()
		 */
		__u32		tcp_tw_isn;
		struct {
			u16	tcp_gso_segs;
			u16	tcp_gso_size;
		};
	};
	__u8		tcp_flags;	/* TCP header flags. (tcp[13])	*/

	__u8		sacked;		/* State flags for SACK/FACK.	*/
#define TCPCB_SACKED_ACKED	0x01	/* SKB ACK'd by a SACK block	*/
#define TCPCB_SACKED_RETRANS	0x02	/* SKB retransmitted		*/
#define TCPCB_LOST		0x04	/* SKB is lost			*/
#define TCPCB_TAGBITS		0x07	/* All tag bits			*/
#define TCPCB_REPAIRED		0x10	/* SKB repaired (no skb_mstamp)	*/
#define TCPCB_EVER_RETRANS	0x80	/* Ever retransmitted frame	*/
#define TCPCB_RETRANS		(TCPCB_SACKED_RETRANS|TCPCB_EVER_RETRANS| \
		TCPCB_REPAIRED)

	__u8		ip_dsfield;	/* IPv4 tos or IPv6 dsfield	*/
	__u8		txstamp_ack:1,	/* Record TX timestamp for ack? */
				eor:1,		/* Is skb MSG_EOR marked? */
				unused:6;
	__u32		ack_seq;	/* Sequence number ACK'd	*/
	union {
		struct {
			/* There is space for up to 24 bytes */
			__u32 in_flight:30,/* Bytes in flight at transmit */
				  is_app_limited:1, /* cwnd not fully used? */
				  unused:1;
			/* pkts S/ACKed so far upon tx of skb, incl retrans: */
			__u32 delivered;
			/* start of send pipeline phase */
			//struct skb_mstamp first_tx_mstamp;
			/* when we reached the "delivered" count */
			//struct skb_mstamp delivered_mstamp;
		} tx;   /* only used for outgoing skbs */
		/*     union {*/
		//struct inet_skb_parm	h4;
		//#if IS_ENABLED(CONFIG_IPV6)
		//struct inet6_skb_parm	h6;
		//#endif
		/*} header;	[> For incoming skbs <]*/
	};
};

#define MAX_TCP_HEADER	(128 + MAX_HEADER)
#define MAX_TCP_OPTION_SPACE 40

/*
 * Never offer a window over 32767 without using window scaling. Some
 * poor stacks do signed 16bit maths!
 */
#define MAX_TCP_WINDOW		32767U

/* Minimal accepted MSS. It is (60+60+8) - (20+20). */
#define TCP_MIN_MSS		88U

/* The least MTU to use for probing */
#define TCP_BASE_MSS		1024

/* probing interval, default to 10 minutes as per RFC4821 */
#define TCP_PROBE_INTERVAL	600

/* Specify interval when tcp mtu probing will stop */
#define TCP_PROBE_THRESHOLD	8

/* After receiving this amount of duplicate ACKs fast retransmit starts. */
#define TCP_FASTRETRANS_THRESH 3

/* Maximal number of ACKs sent quickly to accelerate slow-start. */
#define TCP_MAX_QUICKACKS	16U

/* Maximal number of window scale according to RFC1323 */
#define TCP_MAX_WSCALE		14U

/* urg_data states */
#define TCP_URG_VALID	0x0100
#define TCP_URG_NOTYET	0x0200
#define TCP_URG_READ	0x0400

#define TCP_RETR1	3	/*
						 * This is how many retries it does before it
						 * tries to figure out if the gateway is
						 * down. Minimal RFC value is 3; it corresponds
						 * to ~3sec-8min depending on RTO.
						 */

#define TCP_RETR2	15	/*
						 * This should take at least
						 * 90 minutes to time out.
						 * RFC1122 says that the limit is 100 sec.
						 * 15 is ~13-30min depending on RTO.
						 */

#define TCP_SYN_RETRIES	 6	/* This is how many retries are done
							 * when active opening a connection.
							 * RFC1122 says the minimum retry MUST
							 * be at least 180secs.  Nevertheless
							 * this value is corresponding to
							 * 63secs of retransmission with the
							 * current initial RTO.
							 */

#define TCP_SYNACK_RETRIES 5	/* This is how may retries are done
								 * when passive opening a connection.
								 * This is corresponding to 31secs of
								 * retransmission with the current
								 * initial RTO.
								 */

#define TCP_TIMEWAIT_LEN (60*HZ) /* how long to wait to destroy TIME-WAIT
								  * state, about 60 seconds	*/
#define TCP_FIN_TIMEOUT	TCP_TIMEWAIT_LEN
/* BSD style FIN_WAIT2 deadlock breaker.
 * It used to be 3min, new value is 60sec,
 * to combine FIN-WAIT-2 timeout with
 * TIME-WAIT timer.
 */

#define TCP_DELACK_MAX	((unsigned)(HZ/5))	/* maximal time to delay before sending an ACK */
#if HZ >= 100
#define TCP_DELACK_MIN	((unsigned)(HZ/25))	/* minimal time to delay before sending an ACK */
#define TCP_ATO_MIN	((unsigned)(HZ/25))
#else
#define TCP_DELACK_MIN	4U
#define TCP_ATO_MIN	4U
#endif
#define TCP_RTO_MAX	((unsigned)(120*HZ))
#define TCP_RTO_MIN	((unsigned)(HZ/5))
#define TCP_TIMEOUT_INIT ((unsigned)(1*HZ))	/* RFC6298 2.1 initial RTO value	*/
#define TCP_TIMEOUT_FALLBACK ((unsigned)(3*HZ))	/* RFC 1122 initial RTO value, now
												 * used as a fallback RTO for the
												 * initial data transmission if no
												 * valid RTT sample has been acquired,
												 * most likely due to retrans in 3WHS.
												 */

#define TCP_RESOURCE_PROBE_INTERVAL ((unsigned)(HZ/2U)) /* Maximal interval between probes
														 * for local resources.
														 */
#define TCP_REO_TIMEOUT_MIN	(2000) /* Min RACK reordering timeout in usec */

#define TCP_KEEPALIVE_TIME	(120*60*HZ)	/* two hours */
#define TCP_KEEPALIVE_PROBES	9		/* Max of 9 keepalive probes	*/
#define TCP_KEEPALIVE_INTVL	(75*HZ)

#define MAX_TCP_KEEPIDLE	32767
#define MAX_TCP_KEEPINTVL	32767
#define MAX_TCP_KEEPCNT		127
#define MAX_TCP_SYNCNT		127

#define TCP_SYNQ_INTERVAL	(HZ/5)	/* Period of SYNACK timer */

#define TCP_PAWS_24DAYS	(60 * 60 * 24 * 24)
#define TCP_PAWS_MSL	60		/* Per-host timestamps are invalidated
								 * after this time. It should be equal
								 * (or greater than) TCP_TIMEWAIT_LEN
								 * to provide reliability equal to one
								 * provided by timewait state.
								 */
#define TCP_PAWS_WINDOW	1		/* Replay window for per-host
								 * timestamps. It must be less than
								 * minimal timewait lifetime.
								 */
/*
 *	TCP option
 */

#define TCPOPT_NOP		1	/* Padding */
#define TCPOPT_EOL		0	/* End of options */
#define TCPOPT_MSS		2	/* Segment size negotiating */
#define TCPOPT_WINDOW		3	/* Window scaling */
#define TCPOPT_SACK_PERM        4       /* SACK Permitted */
#define TCPOPT_SACK             5       /* SACK Block */
#define TCPOPT_TIMESTAMP	8	/* Better RTT estimations/PAWS */
#define TCPOPT_MD5SIG		19	/* MD5 Signature (RFC2385) */
#define TCPOPT_FASTOPEN		34	/* Fast open (RFC7413) */
#define TCPOPT_EXP		254	/* Experimental */
		/* Magic number to be after the option value for sharing TCP
		 * experimental options. See draft-ietf-tcpm-experimental-options-00.txt
		 */
#define TCPOPT_FASTOPEN_MAGIC	0xF989

/*
 *     TCP option lengths
 */

#define TCPOLEN_MSS            4
#define TCPOLEN_WINDOW         3
#define TCPOLEN_SACK_PERM      2
#define TCPOLEN_TIMESTAMP      10
#define TCPOLEN_MD5SIG         18
#define TCPOLEN_FASTOPEN_BASE  2
#define TCPOLEN_EXP_FASTOPEN_BASE  4

/* But this is what stacks really send out. */
#define TCPOLEN_TSTAMP_ALIGNED		12
#define TCPOLEN_WSCALE_ALIGNED		4
#define TCPOLEN_SACKPERM_ALIGNED	4
#define TCPOLEN_SACK_BASE		2
#define TCPOLEN_SACK_BASE_ALIGNED	4
#define TCPOLEN_SACK_PERBLOCK		8
#define TCPOLEN_MD5SIG_ALIGNED		20
#define TCPOLEN_MSS_ALIGNED		4

/* Flags in tp->nonagle */
#define TCP_NAGLE_OFF		1	/* Nagle's algo is disabled */
#define TCP_NAGLE_CORK		2	/* Socket is corked		*/
#define TCP_NAGLE_PUSH		4	/* Cork is overridden for already queued data */

/* TCP thin-stream limits */
#define TCP_THIN_LINEAR_RETRIES 6       /* After 6 linear retries, do exp. backoff */

/* TCP initial congestion window as per rfc6928 */
#define TCP_INIT_CWND		10

/* Bit Flags for sysctl_tcp_fastopen */
#define	TFO_CLIENT_ENABLE	1
#define	TFO_SERVER_ENABLE	2
#define	TFO_CLIENT_NO_COOKIE	4	/* Data in SYN w/o cookie option */

/* Accept SYN data w/o any cookie option */
#define	TFO_SERVER_COOKIE_NOT_REQD	0x200

/* Force enable TFO on all listeners, i.e., not requiring the
 * TCP_FASTOPEN socket option.
 */
#define	TFO_SERVER_WO_SOCKOPT1	0x400


	/* sysctl variables for tcp */
	extern int sysctl_tcp_timestamps;
	extern int sysctl_tcp_window_scaling;
	extern int sysctl_tcp_sack;
	extern int sysctl_tcp_fastopen;
	extern int sysctl_tcp_retrans_collapse;
	extern int sysctl_tcp_stdurg;
	extern int sysctl_tcp_rfc1337;
	extern int sysctl_tcp_abort_on_overflow;
	extern int sysctl_tcp_max_orphans;
	extern int sysctl_tcp_fack;
	extern int sysctl_tcp_reordering;
	extern int sysctl_tcp_max_reordering;
	extern int sysctl_tcp_dsack;
	extern long sysctl_tcp_mem[3];
	extern int sysctl_tcp_wmem[3];
	extern int sysctl_tcp_rmem[3];
	extern int sysctl_tcp_app_win;
	extern int sysctl_tcp_adv_win_scale;
	extern int sysctl_tcp_frto;
	extern int sysctl_tcp_low_latency;
	extern int sysctl_tcp_nometrics_save;
	extern int sysctl_tcp_moderate_rcvbuf;
	extern int sysctl_tcp_tso_win_divisor;
	extern int sysctl_tcp_workaround_signed_windows;
	extern int sysctl_tcp_slow_start_after_idle;
	extern int sysctl_tcp_thin_linear_timeouts;
	extern int sysctl_tcp_thin_dupack;
	extern int sysctl_tcp_early_retrans;
	extern int sysctl_tcp_recovery;
#define TCP_RACK_LOSS_DETECTION  0x1 /* Use RACK to detect losses */

	extern int sysctl_tcp_limit_output_bytes;
	extern int sysctl_tcp_challenge_ack_limit;
	extern int sysctl_tcp_min_tso_segs;
	extern int sysctl_tcp_min_rtt_wlen;
	extern int sysctl_tcp_autocorking;
	extern int sysctl_tcp_invalid_ratelimit;
	extern int sysctl_tcp_pacing_ss_ratio;
	extern int sysctl_tcp_pacing_ca_ratio;
	extern int tcp_memory_pressure;

	/*
	 * The next routines deal with comparing 32 bit unsigned ints
	 * and worry about wraparound (automatic with unsigned arithmetic).
	 */

static inline bool before(__u32 seq1, __u32 seq2)
{
	return (__s32)(seq1-seq2) < 0;
}
#define after(seq2, seq1)	before(seq1, seq2)

/* is s2<=s1<=s3 ? */
static inline bool between(__u32 seq1, __u32 seq2, __u32 seq3)
{
	return seq3 - seq2 >= seq1 - seq2;
}

static inline bool tcp_out_of_memory(struct sock *sk)
{
	return false;
}

/* TCP timestamps are only 32-bits, this causes a slight
 * complication on 64-bit systems since we store a snapshot
 * of jiffies in the buffer control blocks below.  We decided
 * to use only the low 32-bits of jiffies and hide the ugly
 * casts with the following macro.
 */
//#define tcp_time_stamp		((__u32)(jiffies))

/* TCP uses 32bit jiffies to save some space.
 * Note that this is different from tcp_time_stamp, which
 * historically has been the same until linux-4.13.
 */
#define tcp_jiffies32 ((u32)jiffies)


	/* Events passed to congestion control interface */
	enum tcp_ca_event {
		CA_EVENT_TX_START,	/* first transmit when no packets in flight */
		CA_EVENT_CWND_RESTART,	/* congestion window restart */
		CA_EVENT_COMPLETE_CWR,	/* end of congestion recovery */
		CA_EVENT_LOSS,		/* loss timeout */
		CA_EVENT_ECN_NO_CE,	/* ECT set, but not CE marked */
		CA_EVENT_ECN_IS_CE,	/* received CE marked IP packet */
		CA_EVENT_DELAYED_ACK,	/* Delayed ack is sent */
		CA_EVENT_NON_DELAYED_ACK,
	};

/* Information about inbound ACK, passed to cong_ops->in_ack_event() */
enum tcp_ca_ack_event_flags {
	CA_ACK_SLOWPATH		= (1 << 0),	/* In slow path processing */
	CA_ACK_WIN_UPDATE	= (1 << 1),	/* ACK updated window */
	CA_ACK_ECE		= (1 << 2),	/* ECE bit is set on ack */
};

/*
 * Interface for adding new TCP congestion control handlers
 */
#define TCP_CA_NAME_MAX	16
#define TCP_CA_MAX	128
#define TCP_CA_BUF_MAX	(TCP_CA_NAME_MAX*TCP_CA_MAX)

#define TCP_CA_UNSPEC	0

	/* Algorithm can be set on socket without CAP_NET_ADMIN privileges */
#define TCP_CONG_NON_RESTRICTED 0x1
	/* Requires ECN/ECT set on all packets */
#define TCP_CONG_NEEDS_ECN	0x2

	union tcp_cc_info;

	struct ack_sample {
		u32 pkts_acked;
		s32 rtt_us;
		u32 in_flight;
	};

struct rate_sample {
	//struct	skb_mstamp prior_mstamp; /* starting timestamp for interval */
	//u32  prior_delivered;	/* tp->delivered at "prior_mstamp" */
	//s32  delivered;		/* number of packets delivered over interval */
	//long interval_us;	/* time for tp->delivered to incr "delivered" */
	//long rtt_us;		/* RTT of last (S)ACKed packet (or -1) */
	//int  losses;		/* number of packets marked lost upon ACK */
	//u32  acked_sacked;	/* number of packets newly (S)ACKed upon ACK */
	//u32  prior_in_flight;	/* in flight before this ACK */
	//bool is_app_limited;	/* is sample from packet with bubble in pipe? */
	//bool is_retrans;	/* is sample from retransmission? */
	
	u64  prior_mstamp; /* starting timestamp for interval */
	u32  prior_delivered;	/* tp->delivered at "prior_mstamp" */
	s32  delivered;		/* number of packets delivered over interval */
	long interval_us;	/* time for tp->delivered to incr "delivered" */
	u32 snd_interval_us;	/* snd interval for delivered packets */
	u32 rcv_interval_us;	/* rcv interval for delivered packets */
	long rtt_us;		/* RTT of last (S)ACKed packet (or -1) */
	int  losses;		/* number of packets marked lost upon ACK */
	u32  acked_sacked;	/* number of packets newly (S)ACKed upon ACK */
	u32  prior_in_flight;	/* in flight before this ACK */
	bool is_app_limited;	/* is sample from packet with bubble in pipe? */
	bool is_retrans;	/* is sample from retransmission? */
	bool is_ack_delayed;	/* is this (likely) a delayed ACK? */
};

/* <M> from include/linux/types.h
 */
struct list_head {
	struct list_head *next, *prev;
};

struct tcp_congestion_ops {
	struct list_head	list;
	u32 key;
	u32 flags;

	/* initialize private data (optional) */
	void (*init)(struct sock *sk);
	/* cleanup private data  (optional) */
	void (*release)(struct sock *sk);

	/* return slow start threshold (required) */
	u32 (*ssthresh)(struct sock *sk);
	/* do new cwnd calculation (required) */
	void (*cong_avoid)(struct sock *sk, u32 ack, u32 acked);
	/* call before changing ca_state (optional) */
	void (*set_state)(struct sock *sk, u8 new_state);
	/* call when cwnd event occurs (optional) */
	void (*cwnd_event)(struct sock *sk, enum tcp_ca_event ev);
	/* call when ack arrives (optional) */
	void (*in_ack_event)(struct sock *sk, u32 flags);
	/* new value of cwnd after loss (required) */
	u32  (*undo_cwnd)(struct sock *sk);
	/* hook for packet ack accounting (optional) */
	void (*pkts_acked)(struct sock *sk, const struct ack_sample *sample);
	/* suggest number of segments for each skb to transmit (optional) */
	u32 (*tso_segs_goal)(struct sock *sk);
	/* returns the multiplier used in tcp_sndbuf_expand (optional) */
	u32 (*sndbuf_expand)(struct sock *sk);
	/* call when packets are delivered to update cwnd and pacing rate,
	 * after all the ca_state processing. (optional)
	 */
	void (*cong_control)(struct sock *sk, const struct rate_sample *rs);
	/* get info for inet_diag (optional) */
	size_t (*get_info)(struct sock *sk, u32 ext, int *attr,
			union tcp_cc_info *info);

	char		name[TCP_CA_NAME_MAX];
	struct module	*owner;
};

static inline bool tcp_ca_needs_ecn(const struct sock *sk)
{
	const struct inet_connection_sock *icsk = inet_csk(sk);

	return icsk->icsk_ca_ops->flags & TCP_CONG_NEEDS_ECN;
}

static inline void tcp_set_ca_state(struct sock *sk, const u8 ca_state)
{
	struct inet_connection_sock *icsk = inet_csk(sk);

	if (icsk->icsk_ca_ops->set_state)
		icsk->icsk_ca_ops->set_state(sk, ca_state);
	icsk->icsk_ca_state = ca_state;
}

static inline void tcp_ca_event(struct sock *sk, const enum tcp_ca_event event)
{
	const struct inet_connection_sock *icsk = inet_csk(sk);

	if (icsk->icsk_ca_ops->cwnd_event)
		icsk->icsk_ca_ops->cwnd_event(sk, event);
}

static inline unsigned int tcp_left_out(const struct tcp_sock *tp)
{
	return tp->sacked_out + tp->lost_out;
}

/* This determines how many packets are "in the network" to the best
 * of our knowledge.  In many cases it is conservative, but where
 * detailed information is available from the receiver (via SACK
 * blocks etc.) we can make more aggressive calculations.
 *
 * Use this for decisions involving congestion control, use just
 * tp->packets_out to determine if the send queue is empty or not.
 *
 * Read this equation as:
 *
 *	"Packets sent once on transmission queue" MINUS
 *	"Packets left network, but not honestly ACKed yet" PLUS
 *	"Packets fast retransmitted"
 */
static inline unsigned int tcp_packets_in_flight(const struct tcp_sock *tp)
{
	return tp->packets_out - tcp_left_out(tp) + tp->retrans_out;
}

#define TCP_INFINITE_SSTHRESH	0x7fffffff

static inline bool tcp_in_slow_start(const struct tcp_sock *tp)
{
	return tp->snd_cwnd < tp->snd_ssthresh;
}

static inline bool tcp_in_initial_slowstart(const struct tcp_sock *tp)
{
	return tp->snd_ssthresh >= TCP_INFINITE_SSTHRESH;
}

static inline bool tcp_in_cwnd_reduction(const struct sock *sk)
{
	return (TCPF_CA_CWR | TCPF_CA_Recovery) &
		(1 << inet_csk(sk)->icsk_ca_state);
}

/* If cwnd > ssthresh, we may raise ssthresh to be half-way to cwnd.
 * The exception is cwnd reduction phase, when cwnd is decreasing towards
 * ssthresh.
 */
static inline __u32 tcp_current_ssthresh(const struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);

	if (tcp_in_cwnd_reduction(sk))
		return tp->snd_ssthresh;
	else
		return max(tp->snd_ssthresh,
				((tp->snd_cwnd >> 1) +
				 (tp->snd_cwnd >> 2)));
}
/* Use define here intentionally to get WARN_ON location shown at the caller */
#define tcp_verify_left_out(tp)	WARN_ON(tcp_left_out(tp) > tp->packets_out)

/* The maximum number of MSS of available cwnd for which TSO defers
 * sending if not using sysctl_tcp_tso_win_divisor.
 */
static inline __u32 tcp_max_tso_deferred_mss(const struct tcp_sock *tp)
{
	return 3;
}

/* Returns end sequence number of the receiver's advertised window */
static inline u32 tcp_wnd_end(const struct tcp_sock *tp)
{
	return tp->snd_una + tp->snd_wnd;
}

/* We follow the spirit of RFC2861 to validate cwnd but implement a more
 * flexible approach. The RFC suggests cwnd should not be raised unless
 * it was fully used previously. And that's exactly what we do in
 * congestion avoidance mode. But in slow start we allow cwnd to grow
 * as long as the application has used half the cwnd.
 * Example :
 *    cwnd is 10 (IW10), but application sends 9 frames.
 *    We allow cwnd to reach 18 when all frames are ACKed.
 * This check is safe because it's as aggressive as slow start which already
 * risks 100% overshoot. The advantage is that we discourage application to
 * either send more filler packets or data to artificially blow up the cwnd
 * usage, and allow application-limited process to probe bw more aggressively.
 */
static inline bool tcp_is_cwnd_limited(const struct sock *sk)
{
	return true;	// do not test it


	const struct tcp_sock *tp = tcp_sk(sk);

	/* If in slow start, ensure cwnd grows to twice what was ACKed. */
	if (tcp_in_slow_start(tp))
		return tp->snd_cwnd < 2 * tp->max_packets_out;

	return tp->is_cwnd_limited;
}

/* Something is really bad, we could not queue an additional packet,
 * because qdisc is full or receiver sent a 0 window.
 * We do not want to add fuel to the fire, or abort too early,
 * so make sure the timer we arm now is at least 200ms in the future,
 * regardless of current icsk_rto value (as it could be ~2ms)
 */
static inline unsigned long tcp_probe0_base(const struct sock *sk)
{
	debug_fun();
	return 0;
}

/* Variant of inet_csk_rto_backoff() used for zero window probes */
static inline unsigned long tcp_probe0_when(const struct sock *sk,
		unsigned long max_when)
{
	debug_fun();
	return 0;
}

static inline void tcp_check_probe_timer(struct sock *sk)
{

	debug_fun();
	return;
}

static inline void tcp_init_wl(struct tcp_sock *tp, u32 seq)
{
	tp->snd_wl1 = seq;
}

static inline void tcp_update_wl(struct tcp_sock *tp, u32 seq)
{
	tp->snd_wl1 = seq;
}

static inline void tcp_prequeue_init(struct tcp_sock *tp)
{
	debug_fun();
	return;
}

static inline void tcp_slow_start_after_idle_check(struct sock *sk)
{
	debug_fun();
	return;
}

static inline int tcp_is_sack(const struct tcp_sock *tp)
{
	return tp->rx_opt.sack_ok;
}

static inline bool tcp_is_reno(const struct tcp_sock *tp)
{
	return !tcp_is_sack(tp);
}

static inline bool tcp_is_fack(const struct tcp_sock *tp)
{
	return tp->rx_opt.sack_ok & TCP_FACK_ENABLED;
}

/* from STCP */
static inline void tcp_clear_retrans_hints_partial(struct tcp_sock *tp)
{
	tp->lost_skb_hint = NULL;
}

static inline void tcp_clear_all_retrans_hints(struct tcp_sock *tp)
{
	tcp_clear_retrans_hints_partial(tp);
	tp->retransmit_skb_hint = NULL;
}

static inline struct sk_buff *tcp_send_head(const struct sock *sk)
{
	debug_fun();
	return NULL;
}


#define	TCP_ECN_OK		1
#define	TCP_ECN_QUEUE_CWR	2
#define	TCP_ECN_DEMAND_CWR	4
#define	TCP_ECN_SEEN		8

// <M>
static inline u32 tcp_highest_sack_seq(struct tcp_sock *tp)
{
	if (!tp->sacked_out)
		return tp->snd_una;
	if (tp->highest_sack == NULL)
		return tp->snd_nxt;
	return TCP_SKB_CB(tp->highest_sack)->seq;
}

static inline u32 tcp_stamp_us_delta(u64 t1, u64 t0)
{
	return max_t(s64, t1 - t0, 0);
}

static inline u32 tcp_min_rtt(const struct tcp_sock *tp)
{
	return minmax_get(&tp->rtt_min);
}

static inline u32 tcp_receive_window(const struct tcp_sock *tp)
{
	s32 win = tp->rcv_wup + tp->rcv_wnd - tp->rcv_nxt;

	if (win < 0)
		win = 0;
	return (u32) win;
}//Lisong added
#endif
