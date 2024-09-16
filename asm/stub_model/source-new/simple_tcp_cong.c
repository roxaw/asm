#ifndef simple_tcp_cong
#define simple_tcp_cong
#include "share.h"
//#include "smack.h"

//static int num_packets = 1;
//module_param(num_packets, int, 0644);
//MODULE_PARM_DESC(num_packets, "number of consecutive packets");

/* Simple linear search, don't expect many entries! */
static struct tcp_congestion_ops *tcp_ca_find(const char *name)
{
	return NULL;
}

/* Must be called with rcu lock held */
static const struct tcp_congestion_ops *__tcp_ca_find_autoload(const char *name)
{

	return NULL;
}

/* Simple linear search, not much in here. */
struct tcp_congestion_ops *tcp_ca_find_key(u32 key)
{

	return NULL;
}

/*
 * Attach new congestion control algorithm to the list
 * of available options.
 */
int tcp_register_congestion_control(struct tcp_congestion_ops *ca)
{
	return 0;
}
EXPORT_SYMBOL_GPL(tcp_register_congestion_control);

/*
 * Remove congestion control algorithm, called from
 * the module's remove function.  Module ref counts are used
 * to ensure that this can't be done till all sockets using
 * that method are closed.
 */
void tcp_unregister_congestion_control(struct tcp_congestion_ops *ca)
{
	return;
}
EXPORT_SYMBOL_GPL(tcp_unregister_congestion_control);

u32 tcp_ca_get_key_by_name(const char *name, bool *ecn_ca)
{
	return 0;
}
EXPORT_SYMBOL_GPL(tcp_ca_get_key_by_name);

char *tcp_ca_get_name_by_key(u32 key, char *buffer)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(tcp_ca_get_name_by_key);

/* Assign choice of congestion control. */
void tcp_assign_congestion_control(struct sock *sk)
{
	return ;
}

void tcp_init_congestion_control(struct sock *sk)
{
	const struct inet_connection_sock *icsk = inet_csk(sk);

	tcp_sk(sk)->prior_ssthresh = 0;
	if (icsk->icsk_ca_ops->init)
		icsk->icsk_ca_ops->init(sk);

	/*if (tcp_ca_needs_ecn(sk))*/
		/*INET_ECN_xmit(sk);*/
	/*else*/
		/*INET_ECN_dontxmit(sk);*/
}

/* Manage refcounts on socket close. */
void tcp_cleanup_congestion_control(struct sock *sk)
{
	return;
}

static void tcp_reinit_congestion_control(struct sock *sk,
					  const struct tcp_congestion_ops *ca)
{
	struct inet_connection_sock *icsk = inet_csk(sk);

	tcp_cleanup_congestion_control(sk);
	icsk->icsk_ca_ops = ca;
	icsk->icsk_ca_setsockopt = 1;
	memset(icsk->icsk_ca_priv, 0, sizeof(icsk->icsk_ca_priv));

	if (sk->sk_state != TCP_CLOSE)
		tcp_init_congestion_control(sk);

	return;
}


/* Used by sysctl to change default congestion control */
int tcp_set_default_congestion_control(const char *name)
{
	return 0;
}

/* Set default value from kernel configuration at bootup */
static int __init tcp_congestion_default(void)
{
	return 0;
}

/* Build string with list of available congestion control values */
void tcp_get_available_congestion_control(char *buf, size_t maxlen)
{
	return ;
}

/* Get current default congestion control */
void tcp_get_default_congestion_control(char *name)
{
	return;
}

/* Built list of non-restricted congestion control values */
void tcp_get_allowed_congestion_control(char *buf, size_t maxlen)
{
	return;
}

/* Change list of non-restricted congestion control */
int tcp_set_allowed_congestion_control(char *val)
{
	return 0;
}

/* Change congestion control for socket */
// revised by wsun, directly pass CA, to comment the code to lookup the name//
int tcp_set_congestion_control(struct sock *sk, const struct tcp_congestion_ops *ca)
{
	tcp_reinit_congestion_control(sk, ca);
	return 0;
}

// <M>
/* set number of consecutive packets for aggregation*/
//static void tcp_set_number_packets(int n)
//{
//	num_packets = n;
//	printf("Number of packets: %d", num_packets);
//}


/* Slow start is used when congestion window is no greater than the slow start
 * threshold. We base on RFC2581 and also handle stretch ACKs properly.
 * We do not implement RFC3465 Appropriate Byte Counting (ABC) per se but
 * something better;) a packet is only considered (s)acked in its entirety to
 * defend the ACK attacks described in the RFC. Slow start processes a stretch
 * ACK of degree N as if N acks of degree 1 are received back to back except
 * ABC caps N to 2. Slow start exits when cwnd grows over ssthresh and
 * returns the leftover acks to adjust cwnd in congestion avoidance mode.
 */
u32 tcp_slow_start(struct tcp_sock *tp, u32 acked)
{
	debug_fun();

	u32 cwnd = min(tp->snd_cwnd + acked, tp->snd_ssthresh);

	acked -= cwnd - tp->snd_cwnd;
	tp->snd_cwnd = min(cwnd, tp->snd_cwnd_clamp);

	return acked;
}
EXPORT_SYMBOL_GPL(tcp_slow_start);

u32 tcp_slow_start_agg(struct tcp_sock *tp, u32 acked)
{
	debug_fun();

	u32 cwnd = min(tp->snd_cwnd + acked, tp->snd_ssthresh);

	acked -= cwnd - tp->snd_cwnd;
	tp->snd_cwnd = min(cwnd, tp->snd_cwnd_clamp);

	return acked;
}


/* In theory this is tp->snd_cwnd += 1 / tp->snd_cwnd (or alternative w),
 * for every packet that was ACKed.
 */
void tcp_cong_avoid_ai(struct tcp_sock *tp, u32 w, u32 acked)
{
	debug_fun();

	// If credits accumulated at a higher w, apply them gently now.
	if (tp->snd_cwnd_cnt >= w) {
		tp->snd_cwnd_cnt = 0;
		tp->snd_cwnd++;
	}

	tp->snd_cwnd_cnt += acked;
	if (tp->snd_cwnd_cnt >= w) {
		u32 delta = tp->snd_cwnd_cnt / w;

		tp->snd_cwnd_cnt -= delta * w;
		tp->snd_cwnd += delta;
	}
	tp->snd_cwnd = min(tp->snd_cwnd, tp->snd_cwnd_clamp);
}
EXPORT_SYMBOL_GPL(tcp_cong_avoid_ai);


void tcp_cong_avoid_ai_agg(struct tcp_sock *tp, u32 w, u32 acked, bool limitedByCwnd)
{
	debug_fun();

	if (limitedByCwnd){
		if (acked > w)
		error_msg ("\x1B[31m[tcp_cong_avoid_ai_agg] acked > w !!!!! Not allowed!\x1B[0m\n");

		if (tp->snd_cwnd_cnt >= w)
			error_msg ("\x1B[31m[tcp_cong_avoid_ai_agg] cwnd_cnt >= w !!!!! Not allowed!\x1B[0m\n");
	}

	if (tp->snd_cwnd_cnt >= w) {
		tp->snd_cwnd_cnt = 0;
		tp->snd_cwnd++;
	}

	tp->snd_cwnd_cnt += acked;
	if (tp->snd_cwnd_cnt >= w) {
		u32 delta = tp->snd_cwnd_cnt / w;

		tp->snd_cwnd_cnt -= delta * w;
		tp->snd_cwnd += delta;
	}

	tp->snd_cwnd = min(tp->snd_cwnd, tp->snd_cwnd_clamp);
}


/*
 * TCP Reno congestion control
 * This is special case used for fallback as well.
 */
/* This is Jacobson's slow start and congestion avoidance.
 * SIGCOMM '88, p. 328.
 */
void tcp_reno_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	
	debug_fun();

	if (!tcp_is_cwnd_limited(sk)) {
		return;
	}

	/* In "safe" area, increase. */
	if (tcp_in_slow_start(tp)) {
		acked = tcp_slow_start(tp, acked);
		if (!acked)
			return;
	}
	/* In dangerous area, increase slowly. */
	tcp_cong_avoid_ai(tp, tp->snd_cwnd, acked);
}
EXPORT_SYMBOL_GPL(tcp_reno_cong_avoid);

//struct reno_agg{
//	u32 last_ack; 		// last acknowledgement number
//	u32 total_acked;	// total number of data packets acked
//};

void tcp_reno_cong_avoid_agg(struct sock *sk, u32 ack, u32 agg_total_acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	
	debug_fun();

	if (!tcp_is_cwnd_limited(sk)) {
		return;
	}

	/* In "safe" area, increase. */
	if (tcp_in_slow_start(tp)) {
			agg_total_acked = tcp_slow_start_agg(tp, agg_total_acked);
		if (!agg_total_acked)
			return;
	}
	/* In dangerous area, increase slowly. */
	tcp_cong_avoid_ai_agg(tp, tp->snd_cwnd, agg_total_acked, true);
}

/* Slow start threshold is half the congestion window (min 2) */
u32 tcp_reno_ssthresh(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	
	debug_fun();
	
	return max(tp->snd_cwnd >> 1U, 2U);
}
EXPORT_SYMBOL_GPL(tcp_reno_ssthresh);

u32 tcp_reno_undo_cwnd(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	
	debug_fun();
	
	return max(tp->snd_cwnd, tp->snd_ssthresh << 1);
}
EXPORT_SYMBOL_GPL(tcp_reno_undo_cwnd);

struct tcp_congestion_ops tcp_reno = {
	.flags		= TCP_CONG_NON_RESTRICTED,
	.name		= "reno",
	.owner		= THIS_MODULE,
	.ssthresh	= tcp_reno_ssthresh,
	.cong_avoid	= tcp_reno_cong_avoid,
	.undo_cwnd	= tcp_reno_undo_cwnd,
};

#endif /* ifndef simple_tcp_cong */
