#ifndef script_property
#define script_property
#include "share.h"

#include "tcp_cubic.c"
extern struct tcp_congestion_ops cubictcp;
extern struct tcp_congestion_ops tcp_reno;
		
u32 __VERIFIER_assert(char *msg, int cond)
{
	char printmsg[1024];
	if (cond){
		//sprintf (printmsg, "\x1B[32m%s passed! \x1B[0m\n", msg);
		//important_msg(printmsg);
		return 0;
	} else {
		sprintf (printmsg, "\x1B[31m%s violated!\x1B[0m\n", msg);
		important_msg(printmsg);
		return 1;
	}
}


void __VERIFIER_nondet_u32(u32 *variable, char *name, u32 min_value, u32 max_value) 
{
#ifdef KLEE
	klee_make_symbolic (variable, sizeof(u32), name);
	klee_assume ((*variable) >= min_value);
	if (klee_is_symbolic(min_value))
		printf("[Symbolic value] %-25s : min=some symbolic variable\n", name);
	else
		printf("[Symbolic value] %-25s : min=%u\n", name, min_value);
	klee_assume ((*variable) <= max_value);
	if (klee_is_symbolic(max_value))
		printf("[Symbolic value] %-25s : max=some symbolic variable\n", name);
	else
		printf("[Symbolic value] %-25s : max=%u\n", name, max_value);
#else
	(*variable) = max_value;
	printf("[Concrete value] %s = %u\n", name, max_value);
#endif
}

void copy_tcp(struct tcp_sock *tpa, struct tcp_sock *tpb)
{
	tpb->snd_cwnd = tpa->snd_cwnd;
	tpb->snd_ssthresh = tpa->snd_ssthresh;
	tpb->snd_cwnd_cnt = tpa->snd_cwnd_cnt;
}

void print_tcp(char *msg, struct tcp_sock *tp){

	printf("%s: cwnd %u, snd_cwnd_cnt %u\n", msg,
		tp->snd_cwnd, tp->snd_cwnd_cnt);
}


int main( int argc, char *argv[] ) 
{
///////// Parameter initilization ///////////////////////

	bool ENABLE_AGG; // 0: normal verification 1: aggregation verification

	u32 MIN_RTT;
	u32 MAX_RTT;

	u32 MIN_CWND;
	u32 MAX_CWND;

	u32 MIN_DELAY_MIN;		
	u32 MAX_DELAY_MIN;		
	
	u32 MIN_BIC_K;			// min_K must be graeter than zero
	u32 MAX_BIC_K;		

	u32 TIME_TO_TEST;

	if( argc == 7 ) {
		if (argv[1][0]=='A')
			ENABLE_AGG = true;
		else
			ENABLE_AGG = false;
		MIN_RTT = atoi(argv[2]);
		MAX_RTT = atoi(argv[3]);
		MIN_CWND = atoi(argv[4]);
		MAX_CWND = atoi(argv[5]);
		TIME_TO_TEST = atoi(argv[6]);
   	} else {
		ENABLE_AGG = false;
		MIN_RTT = 1;
		MAX_RTT = 5;
		MIN_CWND = 2;
		MAX_CWND = 10;
		TIME_TO_TEST = 10;
   	}

	MIN_DELAY_MIN = MIN_RTT << 3; // assume CUBIC calls msecs_to_jiffies and ca->delay_min in ms
	MAX_DELAY_MIN = MAX_RTT << 3;

    cubictcp_register();				// necessary to initalize cubic parameters
	MIN_BIC_K =  cubic_root(cube_factor * MIN_CWND * (BICTCP_BETA_SCALE - beta)/BICTCP_BETA_SCALE)+1;
	MAX_BIC_K =  cubic_root(cube_factor * MAX_CWND * (BICTCP_BETA_SCALE - beta)/BICTCP_BETA_SCALE)+1;

	printf("[Parameter] Agg %u, RTT (%u, %u) ms, CWND (%u, %u), TIME_TO_TEST %u\n", 
		ENABLE_AGG, MIN_RTT, MAX_RTT, MIN_CWND, MAX_CWND, TIME_TO_TEST);


///////// TCP initilization ///////////////////////
	struct tcp_sock tcp_sock_struct1; 
	struct sock *sk1 = (struct sock *)&tcp_sock_struct1;
	struct tcp_sock *tp1 = &tcp_sock_struct1;
	struct inet_connection_sock *icsk1 = inet_csk(sk1);

	memset(&tcp_sock_struct1, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk1, &tcp_reno); 
	struct net net_namespace1;
	memset(&net_namespace1, 0, sizeof(struct net));
	tp1->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace1;
	tcp_sk_init(&net_namespace1);
	tcp_init_sock(sk1);
	if (icsk1->icsk_ca_ops->init)
		icsk1->icsk_ca_ops->init(sk1);
	tcp_ca_event(sk1, CA_EVENT_TX_START);

	struct tcp_sock tcp_sock_struct2; 
	struct sock *sk2 = (struct sock *)&tcp_sock_struct2;
	struct tcp_sock *tp2 = &tcp_sock_struct2;
	struct inet_connection_sock *icsk2 = inet_csk(sk2);
	struct bictcp *ca2 = inet_csk_ca(sk2);

    //cubictcp_register();				// necessary to initalize cubic parameters

	memset(&tcp_sock_struct2, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk2, &cubictcp); 
	struct net net_namespace2;
	memset(&net_namespace2, 0, sizeof(struct net));
	tp2->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace2;
	tcp_sk_init(&net_namespace2);
	tcp_init_sock(sk2);
	if (icsk2->icsk_ca_ops->init)
		icsk2->icsk_ca_ops->init(sk2);
	tcp_ca_event(sk2, CA_EVENT_TX_START);

///////// Symbolic Variable Initilization ///////////////////////
	u32 sym_delay_min;			// ca
	u32 sym_bic_K;				// ca
	
	u32 EPOCH_START = 1;

	__VERIFIER_nondet_u32(&sym_delay_min, "DELAY_MIN", MIN_DELAY_MIN, MAX_DELAY_MIN);
	__VERIFIER_nondet_u32(&sym_bic_K, "bic_K", MIN_BIC_K, MAX_BIC_K);

	// Initialize cubic state
	important_msg("********** Initialize AIMD and CUBIC States **********\n");	
	ca2->epoch_start = EPOCH_START;	
	ca2->delay_min = sym_delay_min;		
	ca2->bic_K = sym_bic_K;
	tp2->snd_ssthresh = 1;
	bictcp_update_test_init(tp2, ca2, 0);

	// Initialize reno state
	tp1->snd_cwnd = ca2->last_max_cwnd;
	tp1->snd_ssthresh = 1;
	tp1->snd_cwnd_cnt = 0;
	tp1->snd_cwnd = tcp_reno_ssthresh(sk1);

///////// Verify Property /////////////////////////////
	important_msg("********** Check AIMD and CUBIC Friendliness **********\n");	
	u32 reno_agg_info;
	struct cubic_agg cubic_agg_info;

	u32 i, j;
	u32 cwnd_at_rtt;
	u32 aimd_sum, cubic_sum;
	aimd_sum = tp1->snd_cwnd;
	cubic_sum = tp2->snd_cwnd;
#ifndef KLEE
	printf("%10s %10s %10s %10s %10s %10s\n", "RTT", "aimd cwnd", "cubic cwnd", "tcp cwnd", "aimd avg", "cubic avg");
	printf("%10u %10u %10u %10u %10u %10u\n", 0, ca2->last_max_cwnd, ca2->last_max_cwnd, ca2->last_max_cwnd, 0, 0);
	printf("%10u %10u %10u %10u %10u %10u\n", 1, tp1->snd_cwnd, tp2->snd_cwnd, ca2->tcp_cwnd, aimd_sum, cubic_sum);
#endif	

	u32 NUM_RTT_TO_TEST = TIME_TO_TEST;
	for(i=1; i<NUM_RTT_TO_TEST; i++){

		jiffies = EPOCH_START + (s32)msecs_to_jiffies(((i)*sym_delay_min) >> 3);		

		cwnd_at_rtt = tp1->snd_cwnd;
		if (ENABLE_AGG){
			reno_agg_info = cwnd_at_rtt;
			tcp_reno_cong_avoid_agg(sk1, 0, reno_agg_info);
		} else {
			for(j=0; j<cwnd_at_rtt; j++)
				tcp_reno_cong_avoid(sk1, 0, 1);
		}

		cwnd_at_rtt = tp2->snd_cwnd;
		if (ENABLE_AGG){
			cubic_agg_info.cwnd_at_rtt = cwnd_at_rtt;
			cubic_agg_info.last_ack_time = jiffies;
			cubic_agg_info.total_acked = cwnd_at_rtt;
			bictcp_cong_avoid_agg(sk2, 0, &cubic_agg_info);
		} else {
			for(j=0; j<cwnd_at_rtt; j++)
				bictcp_cong_avoid(sk2, 0, 1);
		}

		aimd_sum += tp1->snd_cwnd;
		cubic_sum += tp2->snd_cwnd;
#ifndef KLEE
		printf("%10u %10u %10u %10u %10u %10u\n", i+1, tp1->snd_cwnd, tp2->snd_cwnd, ca2->tcp_cwnd, aimd_sum/(i+1), cubic_sum/(i+1));
#endif
	}
	
	u32 diff_result = __VERIFIER_assert("[Verifier] Condition aimd <= cubic", aimd_sum <= (cubic_sum+NUM_RTT_TO_TEST/2));


///////// Print Path Information /////////////////////////////
#ifdef KLEE
	if (diff_result>0) {
		klee_print_range("[Diff result] Range of delay_min:        ", sym_delay_min);
		klee_print_range("[Diff result] Range of bic_K:            ", sym_bic_K);
		klee_print_range("[Diff result] Range of last_max_cwnd:    ", ca2->last_max_cwnd);
	}
#endif

	return 0;
}
#endif
