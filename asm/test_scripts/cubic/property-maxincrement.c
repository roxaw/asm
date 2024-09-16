#ifndef script_property
#define script_property
#include "share.h"

#include "tcp_cubic.c"

extern struct tcp_congestion_ops cubictcp;
		
u32 __VERIFIER_assert(char *msg, int cond)
{
	char printmsg[1024];
	if (cond){
		sprintf (printmsg, "\x1B[32m%s passed! \x1B[0m\n", msg);
		important_msg(printmsg);
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

void copy_tcp(struct tcp_sock *tpa, struct bictcp *caa, struct tcp_sock *tpb, struct bictcp *cab)
{
	tpb->snd_cwnd = tpa->snd_cwnd;
	tpb->snd_ssthresh = tpa->snd_ssthresh;
	tpb->snd_cwnd_cnt = tpa->snd_cwnd_cnt;

	cab->epoch_start = caa->epoch_start;	
	cab->last_max_cwnd = caa->last_max_cwnd;	
	cab->delay_min = caa->delay_min;		
	cab->tcp_cwnd = caa->tcp_cwnd; 		
	cab->ack_cnt = caa->ack_cnt;
	cab->bic_K = caa->bic_K;
	cab->bic_origin_point = caa->bic_origin_point;
	cab->bic_target = caa->bic_target;
	cab->cnt = caa->cnt;
}

void print_tcp(char *msg, struct tcp_sock *tp, struct bictcp *ca){

	printf("%s: target %u, cwnd %u, K %u, origin %u, last_max %u, snd_cwnd_cnt %u, cnt %u, last_cwnd %u, last_time %u, tcp_cwnd %u, ack_cnt %u\n", msg,
		ca->bic_target, tp->snd_cwnd, ca->bic_K, ca->bic_origin_point, ca->last_max_cwnd, tp->snd_cwnd_cnt, ca->cnt, ca->last_cwnd, ca->last_time, ca->tcp_cwnd, ca->ack_cnt);
}


int main( int argc, char *argv[] ) 
{
///////// Parameter initilization ///////////////////////

	bool ENABLE_AGG; // 0: normal verification 1: aggregation verification

	u32 MIN_RTT;
	u32 MAX_RTT;

	u32 MIN_CWND;
	u32 MAX_CWND;

	u32 MIN_SSTHRESH;
	u32 MAX_SSTHRESH;

	u32 MIN_WHICH_RTT;
	u32 MAX_WHICH_RTT;
	
	u32 MIN_DELAY_MIN;		
	u32 MAX_DELAY_MIN;		
	
	u32 MIN_BIC_K;			// min_K must be graeter than zero
	u32 MAX_BIC_K;		


   if( argc == 6 ) {
		if (argv[1][0]=='A')
			ENABLE_AGG = true;
		else
			ENABLE_AGG = false;
		MIN_RTT = atoi(argv[2]);
		MAX_RTT = atoi(argv[3]);
		MIN_CWND = atoi(argv[4]);
		MAX_CWND = atoi(argv[5]);
   } else {
		ENABLE_AGG = false;
		MIN_RTT = 1;
		MAX_RTT = 200;
		MIN_CWND = 2;
		MAX_CWND = 10000;
   }

	MIN_SSTHRESH = 1; // does not support slow start yet
	MAX_SSTHRESH = 1; 

	u32 MIN_EPOCH_START = 1; // starting jiffies
	u32 MAX_EPOCH_START = 1;		

	MIN_WHICH_RTT = 0; // check only RTT 0 which has the maximum increment
	MAX_WHICH_RTT = 0;

	MIN_DELAY_MIN = MIN_RTT << 3; // assume CUBIC calls msecs_to_jiffies and ca->delay_min in ms
	MAX_DELAY_MIN = MAX_RTT << 3;


    cubictcp_register();				// necessary to initalize cubic parameters
	MIN_BIC_K =  cubic_root(cube_factor * MIN_CWND * (BICTCP_BETA_SCALE - beta)/BICTCP_BETA_SCALE)+1;
	MAX_BIC_K =  cubic_root(cube_factor * MAX_CWND * (BICTCP_BETA_SCALE - beta)/BICTCP_BETA_SCALE)+1;


	printf("[Parameter] Agg %u, RTT (%u, %u) ms, CWND (%u, %u)\n", 
		ENABLE_AGG, MIN_RTT, MAX_RTT, MIN_CWND, MAX_CWND);


///////// TCP initilization ///////////////////////
	struct tcp_sock tcp_sock_struct; 
	struct sock *sk = (struct sock *)&tcp_sock_struct;
	struct tcp_sock *tp = &tcp_sock_struct;
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct bictcp *ca = inet_csk_ca(sk);

    cubictcp_register();				// necessary to initalize cubic parameters

	memset(&tcp_sock_struct, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk, &cubictcp); 
	struct net net_namespace;
	memset(&net_namespace, 0, sizeof(struct net));
	tp->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace;
	tcp_sk_init(&net_namespace);
	tcp_init_sock(sk);
	if (icsk->icsk_ca_ops->init)
		icsk->icsk_ca_ops->init(sk);
	tcp_ca_event(sk, CA_EVENT_TX_START);

///////// Symbolic Variable Initilization ///////////////////////
    u32 sym_which_rtt;			// which rtt to test

	u32 sym_ssthresh;			// tp

	u32 sym_epoch_start;		// ca
	u32 sym_delay_min;			// ca
	u32 sym_bic_K;				// ca
	
	u32 diff_result;

	__VERIFIER_nondet_u32(&sym_which_rtt, "which_rtt", MIN_WHICH_RTT, MAX_WHICH_RTT);

	__VERIFIER_nondet_u32(&sym_ssthresh, "ssthresh", MIN_SSTHRESH, MAX_SSTHRESH);

	__VERIFIER_nondet_u32(&sym_epoch_start, "epoch_start", MIN_EPOCH_START, MAX_EPOCH_START);
	__VERIFIER_nondet_u32(&sym_delay_min, "DELAY_MIN", MIN_DELAY_MIN, MAX_DELAY_MIN);
	__VERIFIER_nondet_u32(&sym_bic_K, "bic_K", MIN_BIC_K, MAX_BIC_K);

	// Initialize cubic state
	important_msg("********** Initialize CUBIC States **********\n");	
	ca->epoch_start = sym_epoch_start;	
	ca->delay_min = sym_delay_min;		
	ca->bic_K = sym_bic_K;
	tp->snd_ssthresh = sym_ssthresh;
	jiffies = bictcp_update_test_init(tp, ca, sym_which_rtt);

///////// Verify Property /////////////////////////////
	u32 cwnd_at_rtt;
	struct cubic_agg agg_info;
/*struct cubic_agg{
	u32 cwnd_at_rtt; 	// cwnd at the beginning of RTT
	u32 last_ack_time; 	// arrival time of the last ACK
	u32 total_acked;	// total number of ACKed
};*/

	//print_tcp("TCP: ", tp, ca);

	important_msg("********** Check CUBIC Property **********\n");	

	if (ENABLE_AGG){
		cwnd_at_rtt = tp->snd_cwnd;
		agg_info.cwnd_at_rtt = cwnd_at_rtt;
		agg_info.last_ack_time = jiffies;
		agg_info.total_acked = cwnd_at_rtt;
		bictcp_cong_avoid_agg(sk, 0, &agg_info);
		diff_result = __VERIFIER_assert("[Verifier1] Condition tp->snd_cwnd <= 1.5*sym_init_cwnd", tp->snd_cwnd <= (cwnd_at_rtt*3/2));
		diff_result += __VERIFIER_assert("[Verifier1] Condition target <= 1.5*sym_init_cwnd", ca->bic_target <= (cwnd_at_rtt*3/2));
	} else {
		int i;
		cwnd_at_rtt = tp->snd_cwnd;
		for(i=0; i<cwnd_at_rtt; i++){
			//print_tcp("TCP: ", tp, ca);
			bictcp_cong_avoid(sk, 0, 1);
		}
		diff_result = __VERIFIER_assert("[Verifier1] Condition tp->snd_cwnd <= 1.5*sym_init_cwnd", tp->snd_cwnd <= (cwnd_at_rtt*3/2));
		diff_result += __VERIFIER_assert("[Verifier1] Condition target <= 2*sym_init_cwnd", ca->bic_target <= (cwnd_at_rtt*3/2));
	}

///////// Print Path Information /////////////////////////////
#ifdef KLEE
	if (diff_result > 0){
		klee_print_range("[Diff result] Range of num_of_rtt:       ", sym_which_rtt);
		klee_print_range("[Diff result] Range of epoch_start:      ", sym_epoch_start);
		klee_print_range("[Diff result] Range of delay_min:        ", sym_delay_min);
		//klee_print_range("[Diff result] Range of bic_K:            ", sym_bic_K);
		//klee_print_range("[Diff result] Range of snd_cwnd:         ", tp->snd_cwnd);
		//klee_print_range("[Diff result] Range of cwnd_at_rtt:      ", cwnd_at_rtt);
	}
#endif

	//print_tcp("TCP: ", tp, ca);

	return 0;
}
#endif
