/*
 * TCP CUBIC: Binary Increase Congestion control for TCP v2.3
 * Home page:
 *      http://netsrv.csc.ncsu.edu/twiki/bin/view/Main/BIC
 * This is from the implementation of CUBIC TCP in
 * Sangtae Ha, Injong Rhee and Lisong Xu,
 *  "CUBIC: A New TCP-Friendly High-Speed TCP Variant"
 *  in ACM SIGOPS Operating System Review, July 2008.
 * Available from:
 *  http://netsrv.csc.ncsu.edu/export/cubic_a_new_tcp_2008.pdf
 *
 * CUBIC integrates a new slow start algorithm, called HyStart.
 * The details of HyStart are presented in
 *  Sangtae Ha and Injong Rhee,
 *  "Taming the Elephants: New TCP Slow Start", NCSU TechReport 2008.
 * Available from:
 *  http://netsrv.csc.ncsu.edu/export/hystart_techreport_2008.pdf
 *
 * All testing results are available from:
 * http://netsrv.csc.ncsu.edu/wiki/index.php/TCP_Testing
 *
 * Unless CUBIC is enabled and congestion window is large
 * this behaves the same as the original Reno.
 */

#ifndef tcp_cubic
#define tcp_cubic
#include "share.h"

#define BICTCP_BETA_SCALE    1024	/* Scale factor beta calculation
					 * max_cwnd = snd_cwnd * beta
					 */
#define	BICTCP_HZ		10	 	/* BIC HZ 2^10 = 1024 */

/* Two methods of hybrid slow start */
#define HYSTART_ACK_TRAIN	0x1
#define HYSTART_DELAY		0x2

/* Number of delay samples for detecting the increase of delay */
#define HYSTART_MIN_SAMPLES	8
#define HYSTART_DELAY_MIN	(4U<<3)
#define HYSTART_DELAY_MAX	(16U<<3)
#define HYSTART_DELAY_THRESH(x)	clamp(x, HYSTART_DELAY_MIN, HYSTART_DELAY_MAX)

static int fast_convergence __read_mostly = 1;
static int beta __read_mostly = 717;	/* = 717/1024 (BICTCP_BETA_SCALE) */
static int initial_ssthresh __read_mostly;
static int bic_scale __read_mostly = 41;
static int tcp_friendliness __read_mostly = 1; 	

static int hystart __read_mostly = 0;	// Lisong: Do not test now
static int hystart_detect __read_mostly = HYSTART_ACK_TRAIN | HYSTART_DELAY;
static int hystart_low_window __read_mostly = 16;
static int hystart_ack_delta __read_mostly = 2;

static u32 cube_rtt_scale __read_mostly;
static u32 beta_scale __read_mostly;
static u64 cube_factor __read_mostly;

/* Note parameters that are used for precomputing scale factors are read-only */
module_param(fast_convergence, int, 0644);
MODULE_PARM_DESC(fast_convergence, "turn on/off fast convergence");
module_param(beta, int, 0644);
MODULE_PARM_DESC(beta, "beta for multiplicative increase");
module_param(initial_ssthresh, int, 0644);
MODULE_PARM_DESC(initial_ssthresh, "initial value of slow start threshold");
module_param(bic_scale, int, 0444);
MODULE_PARM_DESC(bic_scale, "scale (scaled by 1024) value for bic function (bic_scale/1024)");
module_param(tcp_friendliness, int, 0644);
MODULE_PARM_DESC(tcp_friendliness, "turn on/off tcp friendliness");
module_param(hystart, int, 0644);
MODULE_PARM_DESC(hystart, "turn on/off hybrid slow start algorithm");
module_param(hystart_detect, int, 0644);
MODULE_PARM_DESC(hystart_detect, "hybrid slow start detection mechanisms"
		 " 1: packet-train 2: delay 3: both packet-train and delay");
module_param(hystart_low_window, int, 0644);
MODULE_PARM_DESC(hystart_low_window, "lower bound cwnd for hybrid slow start");
module_param(hystart_ack_delta, int, 0644);
MODULE_PARM_DESC(hystart_ack_delta, "spacing between ack's indicating train (msecs)");

/* BIC TCP Parameters */
struct bictcp {
	u32	cnt;		/* increase cwnd by 1 after ACKs */
	u32	last_max_cwnd;	/* last maximum snd_cwnd */
	u32	last_cwnd;	/* the last snd_cwnd */
	u32	last_time;	/* time when updated last_cwnd */
	u32	bic_origin_point;/* origin point of bic function */
	u32	bic_K;		/* time to origin point
				   from the beginning of the current epoch */
	u32	delay_min;	/* min delay (msec << 3) */
	u32	epoch_start;	/* beginning of an epoch */
	u32	ack_cnt;	/* number of acks */
	u32	tcp_cwnd;	/* estimated tcp cwnd */
	u16	unused;
	u8	sample_cnt;	/* number of samples to decide curr_rtt */
	u8	found;		/* the exit point is found? */
	u32	round_start;	/* beginning of each round */
	u32	end_seq;	/* end_seq of the round */
	u32	last_ack;	/* last time when the ACK spacing is close */
	u32	curr_rtt;	/* the minimum rtt of current round */
	//newly added
	u32	bic_target;	/* save the value of bic_target */
};

static inline void bictcp_reset(struct bictcp *ca)
{
	debug_fun();
	ca->cnt = 0;
	ca->last_max_cwnd = 0;
	ca->last_cwnd = 0;
	ca->last_time = 0;
	ca->bic_origin_point = 0;
	ca->bic_K = 0;
	ca->delay_min = 0;
	ca->epoch_start = 0;
	ca->ack_cnt = 0;
	ca->tcp_cwnd = 0;
	ca->found = 0;
}

static inline u32 bictcp_clock(void)
{
	debug_fun();
	return jiffies_to_msecs(jiffies);
}

static inline void bictcp_hystart_reset(struct sock *sk)
{
	debug_fun();
	struct tcp_sock *tp = tcp_sk(sk);
	struct bictcp *ca = inet_csk_ca(sk);

	ca->round_start = ca->last_ack = bictcp_clock();
	ca->end_seq = tp->snd_nxt;
	ca->curr_rtt = ~0U;
	ca->sample_cnt = 0;
}

static void bictcp_init(struct sock *sk)
{
	debug_fun();
	struct bictcp *ca = inet_csk_ca(sk);

	bictcp_reset(ca);

	if (hystart)
		bictcp_hystart_reset(sk);

	if (!hystart && initial_ssthresh)
		tcp_sk(sk)->snd_ssthresh = initial_ssthresh;
}

static void bictcp_cwnd_event(struct sock *sk, enum tcp_ca_event event)
{
	debug_fun();
	if (event == CA_EVENT_TX_START) {
		struct bictcp *ca = inet_csk_ca(sk);
		u32 now = tcp_jiffies32;
		s32 delta;

		delta = now - tcp_sk(sk)->lsndtime;

		/* We were application limited (idle) for a while.
		 * Shift epoch_start to keep cwnd growth to cubic curve.
		 */
		if (ca->epoch_start && delta > 0) {
			ca->epoch_start += delta;
			if (after(ca->epoch_start, now))
				ca->epoch_start = now;
		}
		return;
	}
}

/* calculate the cubic root of x using a table lookup followed by one
 * Newton-Raphson iteration.
 * Avg err ~= 0.195%
 */
static u32 cubic_root(u64 a)
{
	debug_fun();
	u32 x, b, shift;
	/*
	 * cbrt(x) MSB values for x MSB values in [0..63].
	 * Precomputed then refined by hand - Willy Tarreau
	 *
	 * For x in [0..63],
	 *   v = cbrt(x << 18) - 1
	 *   cbrt(x) = (v[x] + 10) >> 6
	 */
	static const u8 v[] = {
		/* 0x00 */    0,   54,   54,   54,  118,  118,  118,  118,
		/* 0x08 */  123,  129,  134,  138,  143,  147,  151,  156,
		/* 0x10 */  157,  161,  164,  168,  170,  173,  176,  179,
		/* 0x18 */  181,  185,  187,  190,  192,  194,  197,  199,
		/* 0x20 */  200,  202,  204,  206,  209,  211,  213,  215,
		/* 0x28 */  217,  219,  221,  222,  224,  225,  227,  229,
		/* 0x30 */  231,  232,  234,  236,  237,  239,  240,  242,
		/* 0x38 */  244,  245,  246,  248,  250,  251,  252,  254,
	};

	b = fls64(a);
	if (b < 7) {
		/* a in [0..63] */
		return ((u32)v[(u32)a] + 35) >> 6;
	}

	b = ((b * 84) >> 8) - 1;
	shift = (a >> (b * 3));

	x = ((u32)(((u32)v[shift] + 10) << b)) >> 6;

	/*
	 * Newton-Raphson iteration
	 *                         2
	 * x    = ( 2 * x  +  a / x  ) / 3
	 *  k+1          k         k
	 */
	x = (2 * x + (u32)div64_u64(a, (u64)x * (u64)(x - 1)));
	x = ((x * 341) >> 10);
	return x;
}

/*
 * Compute congestion window to use.
 */
static inline void bictcp_update(struct bictcp *ca, u32 cwnd, u32 acked)
{
	debug_fun();

	debug_format("cwnd: %u, acked: %u, tcp_jiffies32: %u, ca->last_time: %u, HZ: %u\n",
						cwnd, acked, tcp_jiffies32, ca->last_time, HZ);

	u32 delta, bic_target, max_cnt;
	u64 offs, t;

	ca->ack_cnt += acked;	/* count the number of ACKed packets */

//#define BUG_FIX1
//#define BUG_FIX2
//#define BUG_FIX3

#ifdef BUG_FIX1
	//should update cwnd at least once per RTT. update cwnd even if RTT < HZ/32 = 32ms
	if (ca->last_cwnd == cwnd &&
	   (s32)(tcp_jiffies32 - ca->last_time) < min(HZ / 32, msecs_to_jiffies(ca->delay_min >> 3)))
		return;
#else
	if (ca->last_cwnd == cwnd &&
	   (s32)(tcp_jiffies32 - ca->last_time) <= HZ / 32)
		return;
#endif
	/* The CUBIC function can update ca->cnt at most once per jiffy.
	 * On all cwnd reduction events, ca->epoch_start is set to 0,
	 * which will force a recalculation of ca->cnt.
	 */
	if (ca->epoch_start && tcp_jiffies32 == ca->last_time)
		goto tcp_friendliness;

	ca->last_cwnd = cwnd;
	ca->last_time = tcp_jiffies32;

	if (ca->epoch_start == 0) {
		ca->epoch_start = tcp_jiffies32;	/* record beginning */
		ca->ack_cnt = acked;			/* start counting */
		ca->tcp_cwnd = cwnd;			/* syn with cubic */

		if (ca->last_max_cwnd <= cwnd) {
			ca->bic_K = 0;
			ca->bic_origin_point = cwnd;
		} else {
			/* Compute new K based on
			 * (wmax-cwnd) * (srtt>>3 / HZ) / c * 2^(3*bictcp_HZ)
			 */
			ca->bic_K = cubic_root(cube_factor
					       * (ca->last_max_cwnd - cwnd));
			ca->bic_origin_point = ca->last_max_cwnd;
		}
	}

	/* cubic function - calc*/
	/* calculate c * time^3 / rtt,
	 *  while considering overflow in calculation of time^3
	 * (so time^3 is done by using 64 bit)
	 * and without the support of division of 64bit numbers
	 * (so all divisions are done by using 32 bit)
	 *  also NOTE the unit of those veriables
	 *	  time  = (t - K) / 2^bictcp_HZ
	 *	  c = bic_scale >> 10
	 * rtt  = (srtt >> 3) / HZ
	 * !!! The following code does not have overflow problems,
	 * if the cwnd < 1 million packets !!!
	 */

	t = (s32)(tcp_jiffies32 - ca->epoch_start);
	t += msecs_to_jiffies(ca->delay_min >> 3);
	/* change the unit from HZ to bictcp_HZ */
	t <<= BICTCP_HZ; 
	do_div(t, HZ);

	if (t < ca->bic_K)		/* t - K */
		offs = ca->bic_K - t;
	else
		offs = t - ca->bic_K;
	
	debug_format("epoch_start: %u, delay_min: %u, offs: %llu, ca->bic_K: %u, t: %llu\n",
						ca->epoch_start, ca->delay_min,	offs, ca->bic_K, t);

	/* c/rtt * (t-K)^3 */
	delta = (cube_rtt_scale * offs * offs * offs) >> (10+3*BICTCP_HZ);
	if (t < ca->bic_K)                            /* below origin*/
		bic_target = ca->bic_origin_point - delta;
	else                                          /* above origin*/
		bic_target = ca->bic_origin_point + delta;
	
	debug_format("bic_target: %u, bic_origin_point: %u, delta: %u\n",
						bic_target, ca->bic_origin_point, delta);
	ca->bic_target = bic_target;
	
	/* cubic function - calc bictcp_cnt*/
	if (bic_target > cwnd) {
		ca->cnt = cwnd / (bic_target - cwnd);
	} else {
		ca->cnt = 100 * cwnd;              /* very small increment*/
	}

//printf("epoch_start=%u, last_max_cwnd=%u, cwnd=%u, bic_K=%u, bic_origin_point=%u, delay_min=%u, bic_target=%u, delta=%u, cnt=%u, t=%llu, offs=%llu\n", ca->epoch_start, ca->last_max_cwnd, cwnd, ca->bic_K, ca->bic_origin_point, ca->delay_min, bic_target, delta, ca->cnt, t, offs);

	/*
	 * The initial growth of cubic function may be too conservative
	 * when the available bandwidth is still unknown.
	 */
	if (ca->last_max_cwnd == 0 && ca->cnt > 20)
		ca->cnt = 20;	/* increase cwnd 5% per RTT */

tcp_friendliness:
	/* TCP Friendly */
	if (tcp_friendliness) {
		u32 scale = beta_scale;


#ifdef BUG_FIX2
		if (cwnd < ca->bic_origin_point) // to be consistent with the latest RFC
			delta = (cwnd * scale) >> 3;
		else
			delta = cwnd;
#else
		delta = (cwnd * scale) >> 3;
#endif
		while (ca->ack_cnt > delta) {		/* update tcp cwnd */
			ca->ack_cnt -= delta;
			ca->tcp_cwnd++;
		}

#ifdef BUG_FIX3
		// should use the expected cwnd in the next RTT instead of current cwnd 
		u32 tcp_cwnd_next_rtt = ca->tcp_cwnd + (ca->ack_cnt+cwnd)/delta;
		if (tcp_cwnd_next_rtt > cwnd) {	/* if bic is slower than tcp */
			delta = tcp_cwnd_next_rtt - cwnd;
#else
		if (ca->tcp_cwnd > cwnd) {	/* if bic is slower than tcp */
			delta = ca->tcp_cwnd - cwnd;
#endif
			max_cnt = cwnd / delta;
			if (ca->cnt > max_cnt)	
				ca->cnt = max_cnt;
		}
	}

	/* The maximum rate of cwnd increase CUBIC allows is 1 packet per
	 * 2 packets ACKed, meaning cwnd grows at 1.5x per RTT.
	 */
	ca->cnt = max(ca->cnt, 2U);	

}

// directly set the variables of CUBIC to some RTT after a fast recovery
// input : ca->bic_K, ca->epoch_start, ca->delay_min
// output: ca: last_max_cwnd, ca->bic_origin_point, last_cwnd, last_time, jiffies, tp->snd_cwnd,
// does not support slow start yet
// to do: which_rtt > 0. (==0 is good)
static inline u32 bictcp_update_test_init(struct tcp_sock *tp, struct bictcp *ca, u32 which_rtt)
{
	debug_fun();

	u32 delta1, delta2, bic_target1, bic_target2; //1 for begining of RTT and 2 for end of RTT
	u64 offs0, delta0, offs1, t1, offs2, t2;
	u32 return_jiffies;
	u32 cwnd_at_epoch;

	if (ca->bic_K == 0) {
		error_msg("bic_K should be greater than zero for testing cubic");
		return 0;
	}

	offs0 = ca->bic_K;
	delta0 = (cube_rtt_scale * offs0 * offs0 * offs0) >> (10+3*BICTCP_HZ);
	if (delta0 > 0){
		ca->last_max_cwnd = delta0 * BICTCP_BETA_SCALE / (BICTCP_BETA_SCALE - beta);
		tp->snd_cwnd = ca->last_max_cwnd * beta / BICTCP_BETA_SCALE;
		ca->bic_origin_point = ca->last_max_cwnd;
		cwnd_at_epoch = tp->snd_cwnd;
	} else {	// if delta0 ==0
		ca->last_max_cwnd = 2;
		tp->snd_cwnd = 2;
		ca->bic_origin_point = 2;
		cwnd_at_epoch = 2;
	}
	//printf("K %u, offs1 %llu, delta1 %u, last_max %u, cwnd %u, origin %u, epoch %u\n", 
	//	ca->bic_K, offs1, delta1, ca->last_max_cwnd, tp->snd_cwnd, ca->bic_origin_point, cwnd_at_epoch);

	t1 = msecs_to_jiffies((which_rtt*ca->delay_min) >> 3);
	//ca->last_time = ca->epoch_start + (s32)t1;		
	/* change the unit from HZ to bictcp_HZ */
	t1 <<= BICTCP_HZ;
	do_div(t1, HZ);

	if (t1 < ca->bic_K){		/* t - K */
		offs1 = ca->bic_K - t1;
		delta1 = (cube_rtt_scale * offs1 * offs1 * offs1) >> (10+3*BICTCP_HZ);
		bic_target1 = ca->bic_origin_point - delta1;
	} else {
		offs1 = t1 - ca->bic_K;
		delta1 = (cube_rtt_scale * offs1 * offs1 * offs1) >> (10+3*BICTCP_HZ);
		bic_target1 = ca->bic_origin_point + delta1;
	}
	
	tp->snd_cwnd = bic_target1;
	tp->snd_cwnd_cnt = 0;

	ca->tcp_cwnd = cwnd_at_epoch + (which_rtt)*9/17;
	ca->ack_cnt = 0;

	ca->last_cwnd = 0;
	ca->last_time = 0;

	if (ca->tcp_cwnd > tp->snd_cwnd) 	/* if bic is slower than tcp */
		tp->snd_cwnd = ca->tcp_cwnd;

	t2 = msecs_to_jiffies(((which_rtt+1)*ca->delay_min) >> 3);
	// // test ACK arriaving at end of num_of_rtt
	return_jiffies = ca->epoch_start + (s32)t2;		

	return return_jiffies;
}


// To simiplify the handling of HZ/32
// cosider only the case where the computation part is called only once at the end of RTT
// cwnd_at_rtt is the beginning cwnd of an RTT = tp->snd_cwnd
// last_ack_time is the end time of an RTT 
static inline void bictcp_update_and_tcp_cong_avoid_agg(struct tcp_sock *tp, struct bictcp *ca, u32 cwnd_at_rtt, u32 last_ack_time, u32 acked)
{
	debug_fun();

	u32 delta, max_cnt;
	u64 offs, t;

	if (ca->epoch_start == 0) {
		error_msg("epoch_start must be nonzero. That is, already initialized");
		return;
	}
	if (acked > cwnd_at_rtt) {
		error_msg("acked should be no more than current cwnd for agg cubic");
		return;
	}
	if (acked == 0) {
		error_msg("acked should be greater than zero for agg cubic");
		return;
	}


	if ((ca->last_cwnd==cwnd_at_rtt)&&((s32)(last_ack_time-ca->last_time)<=HZ/32)) {
		ca->ack_cnt += acked;	
	} else{
		ca->last_cwnd = cwnd_at_rtt; // may have some impact in proof by induct
		ca->last_time = last_ack_time;

		t = (s32)(last_ack_time - ca->epoch_start);
		t += msecs_to_jiffies(ca->delay_min >> 3);
		/* change the unit from HZ to bictcp_HZ */
		t <<= BICTCP_HZ;
		do_div(t, HZ);

		if (t < ca->bic_K){		/* t - K */
			offs = ca->bic_K - t;
			delta = (cube_rtt_scale * offs * offs * offs) >> (10+3*BICTCP_HZ);
			ca->bic_target = ca->bic_origin_point - delta;
		} else {
			offs = t - ca->bic_K;
			delta = (cube_rtt_scale * offs * offs * offs) >> (10+3*BICTCP_HZ);
			ca->bic_target = ca->bic_origin_point + delta;
		}
	
		/* cubic function - calc bictcp_cnt*/
		if (ca->bic_target > tp->snd_cwnd) {
			ca->cnt = tp->snd_cwnd / (ca->bic_target - tp->snd_cwnd);
		} else {
			ca->cnt = 100 * tp->snd_cwnd;              /* very small increment*/
		}

		// equvalent to tcp friendliness
		// inc tcp_cwnd for first ACK?
		delta = (cwnd_at_rtt*beta_scale)>>3;
		ca->ack_cnt += 1;	
		while (ca->ack_cnt > delta){
			ca->ack_cnt -= delta;
			ca->tcp_cwnd++;
		}
		// inc tcp_cwnd for rest ACKs?
		delta = (tp->snd_cwnd*beta_scale)>>3;
		ca->ack_cnt += (acked - 1);	/* count the number of ACKed packets */
		if ((ca->ack_cnt > delta) && (tp->snd_cwnd > cwnd_at_rtt)){
			ca->ack_cnt -= delta;
			ca->tcp_cwnd++;
		}

		if (ca->tcp_cwnd > tp->snd_cwnd) {	/* if bic is slower than tcp */
			delta = ca->tcp_cwnd - tp->snd_cwnd;
			max_cnt = tp->snd_cwnd / delta;
			if (ca->cnt > max_cnt)	
				ca->cnt = max_cnt;
		}
	}

	ca->cnt = max(ca->cnt, 2U);	

	// equivalent to tcp_cong_avoid_ai
	if (ca->cnt<=tp->snd_cwnd){
			if (tp->snd_cwnd_cnt >= ca->cnt){ // for the first if statement in tcp_cong_avoid_ai
				tp->snd_cwnd = tp->snd_cwnd+1;
				tp->snd_cwnd_cnt = 0;
			}
			tp->snd_cwnd_cnt = tp->snd_cwnd_cnt + acked; 
			u32 ai_delta = tp->snd_cwnd_cnt/ca->cnt;
			tp->snd_cwnd = tp->snd_cwnd+ai_delta;
			tp->snd_cwnd_cnt = tp->snd_cwnd_cnt - ai_delta*ca->cnt;
	}else{
			tp->snd_cwnd_cnt = tp->snd_cwnd_cnt + acked; 
	}

}

// from the latest Kernel
static void bictcp_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	debug_fun();
	struct tcp_sock *tp = tcp_sk(sk);
	struct bictcp *ca = inet_csk_ca(sk);
	
	if (!tcp_is_cwnd_limited(sk)) {
		return;
	}

	if (tcp_in_slow_start(tp)) {
		acked = tcp_slow_start(tp, acked);
		if (!acked)
			return;
	}
	
	bictcp_update(ca, tp->snd_cwnd, acked);
	tcp_cong_avoid_ai(tp, ca->cnt, acked);
	//printf("ca->epoch_start=%u, ca->cnt=%u, ca->ack_cnt=%u, ca->tcp_cwnd=%u, tp->snd_cwnd=%u\n", ca->epoch_start, ca->cnt, ca->ack_cnt, ca->tcp_cwnd, tp->snd_cwnd);

}

struct cubic_agg{
	u32 last_ack; 		// last acknowledgement number
	u32 cwnd_at_rtt; 	// cwnd at the beginning of RTT
	u32 last_ack_time; 	// arrival time of the last ACK
	u32 total_acked;	// total number of ACKed
};

static void bictcp_cong_avoid_agg(struct sock *sk, u32 ack, struct cubic_agg *agg_info)
{
	debug_fun();
	struct tcp_sock *tp = tcp_sk(sk);
	struct bictcp *ca = inet_csk_ca(sk);

	if (!tcp_is_cwnd_limited(sk)) {
		return;
	}

	u32 acked = agg_info->total_acked;
	if (tcp_in_slow_start(tp)) {
		acked = tcp_slow_start_agg(tp, acked);
		if (!acked)
			return;
	}
	
	bictcp_update_and_tcp_cong_avoid_agg(tp, ca, agg_info->cwnd_at_rtt, agg_info->last_ack_time, acked);
}

static u32 bictcp_recalc_ssthresh(struct sock *sk)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	struct bictcp *ca = inet_csk_ca(sk);
	
	debug_fun();

	ca->epoch_start = 0;	/* end of epoch */

	/* Wmax and fast convergence */
	if (tp->snd_cwnd < ca->last_max_cwnd && fast_convergence)
		ca->last_max_cwnd = (tp->snd_cwnd * (BICTCP_BETA_SCALE + beta))
			/ (2 * BICTCP_BETA_SCALE);
	else
		ca->last_max_cwnd = tp->snd_cwnd;

	return max((tp->snd_cwnd * beta) / BICTCP_BETA_SCALE, 2U);
}

//static u32 bictcp_undo_cwnd(struct sock *sk)
//{
//	struct bictcp *ca = inet_csk_ca(sk);
//
//	return max(tcp_sk(sk)->snd_cwnd, ca->loss_cwnd);
//}

static void bictcp_state(struct sock *sk, u8 new_state)
{
	debug_fun();
	if (new_state == TCP_CA_Loss) {
		bictcp_reset(inet_csk_ca(sk));
		bictcp_hystart_reset(sk);
	}
}

static void hystart_update(struct sock *sk, u32 delay)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct bictcp *ca = inet_csk_ca(sk);
	
	debug_fun();

	if (ca->found & hystart_detect)
		return;

	if (hystart_detect & HYSTART_ACK_TRAIN) {
		u32 now = bictcp_clock();

		/* first detection parameter - ack-train detection */
		if ((s32)(now - ca->last_ack) <= hystart_ack_delta) {
			ca->last_ack = now;
			if ((s32)(now - ca->round_start) > ca->delay_min >> 4) {
				ca->found |= HYSTART_ACK_TRAIN;
				NET_INC_STATS(sock_net(sk),
					      LINUX_MIB_TCPHYSTARTTRAINDETECT);
				NET_ADD_STATS(sock_net(sk),
					      LINUX_MIB_TCPHYSTARTTRAINCWND,
					      tp->snd_cwnd);
				tp->snd_ssthresh = tp->snd_cwnd;
			}
		}
	}

	if (hystart_detect & HYSTART_DELAY) {
		/* obtain the minimum delay of more than sampling packets */
		if (ca->sample_cnt < HYSTART_MIN_SAMPLES) {
			if (ca->curr_rtt == 0 || ca->curr_rtt > delay)
				ca->curr_rtt = delay;

			ca->sample_cnt++;
		} else {
			if (ca->curr_rtt > ca->delay_min +
			    HYSTART_DELAY_THRESH(ca->delay_min >> 3)) {
				ca->found |= HYSTART_DELAY;
				NET_INC_STATS(sock_net(sk),
					      LINUX_MIB_TCPHYSTARTDELAYDETECT);
				NET_ADD_STATS(sock_net(sk),
					      LINUX_MIB_TCPHYSTARTDELAYCWND,
					      tp->snd_cwnd);
				tp->snd_ssthresh = tp->snd_cwnd;
			}
		}
	}
}

/* Track delayed acknowledgment ratio using sliding window
 * ratio = (15*ratio + sample) / 16
 */
static void bictcp_acked(struct sock *sk, const struct ack_sample *sample)
{
	const struct tcp_sock *tp = tcp_sk(sk);
	struct bictcp *ca = inet_csk_ca(sk);
	u32 delay;
	
	debug_fun();
	if (DEBUG_INFO) {
		printf ("[Func:bictcp_acked] rtt_us: %d, ca->epoch_start: %u, tcp_jiffies32: %u, HZ: %d\n",
					sample->rtt_us, ca->epoch_start, tcp_jiffies32, HZ);
	}

	/* Some calls are for duplicates without timetamps */
	if (sample->rtt_us < 0)
		return;

	/* Discard delay samples right after fast recovery */
	if (ca->epoch_start && (s32)(tcp_jiffies32 - ca->epoch_start) < HZ)
		return;

	delay = (sample->rtt_us << 3) / USEC_PER_MSEC;
	if (delay == 0)
		delay = 1;

	/* first time call or link delay decreases */
	if (ca->delay_min == 0 || ca->delay_min > delay)
		ca->delay_min = delay;

	/* hystart triggers when cwnd is larger than some threshold */
	if (hystart && tcp_in_slow_start(tp) &&
	    tp->snd_cwnd >= hystart_low_window)
		hystart_update(sk, delay);
}

static struct tcp_congestion_ops cubictcp __read_mostly = {
	.init		= bictcp_init,
	.ssthresh	= bictcp_recalc_ssthresh,
	.cong_avoid	= bictcp_cong_avoid,
	.set_state	= bictcp_state,
//	.undo_cwnd	= bictcp_undo_cwnd,  //latest kernel use reno undo
	.undo_cwnd	= tcp_reno_undo_cwnd,
	.cwnd_event	= bictcp_cwnd_event,
	.pkts_acked     = bictcp_acked,
	.owner		= THIS_MODULE,
	.name		= "cubic",
};

static int __init cubictcp_register(void)
{
	BUILD_BUG_ON(sizeof(struct bictcp) > ICSK_CA_PRIV_SIZE);

	debug_fun();

	/* Precompute a bunch of the scaling factors that are used per-packet
	 * based on SRTT of 100ms
	 */

	beta_scale = 8*(BICTCP_BETA_SCALE+beta) / 3
		/ (BICTCP_BETA_SCALE - beta);

	cube_rtt_scale = (bic_scale * 10);	/* 1024*c/rtt */

	/* calculate the "K" for (wmax-cwnd) = c/rtt * K^3
	 *  so K = cubic_root( (wmax-cwnd)*rtt/c )
	 * the unit of K is bictcp_HZ=2^10, not HZ
	 *
	 *  c = bic_scale >> 10
	 *  rtt = 100ms
	 *
	 * the following code has been designed and tested for
	 * cwnd < 1 million packets
	 * RTT < 100 seconds
	 * HZ < 1,000,00  (corresponding to 10 nano-second)
	 */

	/* 1/c * 2^2*bictcp_HZ * srtt */
	cube_factor = 1ull << (10+3*BICTCP_HZ); /* 2^40 */
//	cube_factor = 1099511627776;

	/* divide by bic_scale and by constant Srtt (100ms) */
	do_div(cube_factor, bic_scale * 10);

	return tcp_register_congestion_control(&cubictcp);
}

static void __exit cubictcp_unregister(void)
{
	tcp_unregister_congestion_control(&cubictcp);
}

module_init(cubictcp_register);
module_exit(cubictcp_unregister);

MODULE_AUTHOR("Sangtae Ha, Stephen Hemminger");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("CUBIC TCP");
MODULE_VERSION("2.3");

#endif
