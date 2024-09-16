#ifndef script_equivalence_check
#define script_equivalence_check
#include "share.h"

#include "tcp_cubic.c"

extern struct tcp_congestion_ops cubictcp;
		
u32 __VERIFIER_assert_u32(char *msg, u32 val1, u32 val2)
{
	char printmsg[1024];
	if (val1==val2){
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



   if( argc == 5 ) {
		MIN_RTT = atoi(argv[1]);
		MAX_RTT = atoi(argv[2]);
		MIN_CWND = atoi(argv[3]);
		MAX_CWND = atoi(argv[4]);
   } else {
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


	printf("[Parameter] RTT (%u, %u) ms, CWND (%u, %u)\n", 
		MIN_RTT, MAX_RTT, MIN_CWND, MAX_CWND);

///////// TCP initilization ///////////////////////
	struct tcp_sock tcp_sock_struct0; // 0:initialization
	struct sock *sk0 = (struct sock *)&tcp_sock_struct0;
	struct tcp_sock *tp0 = &tcp_sock_struct0;
	struct inet_connection_sock *icsk0 = inet_csk(sk0);
	struct bictcp *ca0 = inet_csk_ca(sk0);

	struct tcp_sock tcp_sock_struct1; // 1:original
	struct sock *sk1 = (struct sock *)&tcp_sock_struct1;
	struct tcp_sock *tp1 = &tcp_sock_struct1;
	struct inet_connection_sock *icsk1 = inet_csk(sk1);
	struct bictcp *ca1 = inet_csk_ca(sk1);

	struct tcp_sock tcp_sock_struct2; // 2:aggregation
	struct sock *sk2 = (struct sock *)&tcp_sock_struct2;
	struct tcp_sock *tp2 = &tcp_sock_struct2;
	struct inet_connection_sock *icsk2 = inet_csk(sk2);
	struct bictcp *ca2 = inet_csk_ca(sk2);

    //cubictcp_register();				// necessary to initalize cubic parameters

	memset(&tcp_sock_struct0, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk0, &cubictcp); 
	struct net net_namespace0;
	memset(&net_namespace0, 0, sizeof(struct net));
	tp0->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace0;
	tcp_sk_init(&net_namespace0);
	tcp_init_sock(sk0);
	if (icsk0->icsk_ca_ops->init)
		icsk0->icsk_ca_ops->init(sk0);
	tcp_ca_event(sk0, CA_EVENT_TX_START);

	memset(&tcp_sock_struct1, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk1, &cubictcp); 
	struct net net_namespace1;
	memset(&net_namespace1, 0, sizeof(struct net));
	tp1->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace1;
	tcp_sk_init(&net_namespace1);
	tcp_init_sock(sk1);
	if (icsk1->icsk_ca_ops->init)
		icsk1->icsk_ca_ops->init(sk1);
	tcp_ca_event(sk1, CA_EVENT_TX_START);

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
    u32 sym_which_rtt;			// how many rtts to test

	u32 sym_ssthresh;			// tp

	u32 sym_epoch_start;		// ca
	u32 sym_delay_min;			// ca
	u32 sym_bic_K;				// ca
	
	__VERIFIER_nondet_u32(&sym_which_rtt, "which_rtt", MIN_WHICH_RTT, MAX_WHICH_RTT);

	__VERIFIER_nondet_u32(&sym_ssthresh, "ssthresh", MIN_SSTHRESH, MAX_SSTHRESH);

	__VERIFIER_nondet_u32(&sym_epoch_start, "epoch_start", MIN_EPOCH_START, MAX_EPOCH_START);
	__VERIFIER_nondet_u32(&sym_delay_min, "DELAY_MIN", MIN_DELAY_MIN, MAX_DELAY_MIN);
	__VERIFIER_nondet_u32(&sym_bic_K, "bic_K", MIN_BIC_K, MAX_BIC_K);

	// Initialize cubic state
	u32 jiffies0;
	important_msg("*** Initialize CUBIC ***\n");	
	ca0->epoch_start = sym_epoch_start;	
	ca0->delay_min = sym_delay_min;		
	ca0->bic_K = sym_bic_K;
	tp0->snd_ssthresh = sym_ssthresh;
	jiffies0 = bictcp_update_test_init(tp0, ca0, sym_which_rtt);
	//print_tcp("0: ", tp0, ca0);

///////// Verify Equivalence /////////////////////////////
	u32 diff_result = 0;
	struct cubic_agg agg_info;
/*struct cubic_agg{
	u32 cwnd_at_rtt; 	// cwnd at the beginning of RTT
	u32 last_ack_time; 	// arrival time of the last ACK
	u32 total_acked;	// total number of ACKed
};*/
	
	// Base step
	u32 acked_b;
	important_msg("*** Base Step ***\n");	
	acked_b	= 1;								// induction variable

	copy_tcp(tp0, ca0, tp1, ca1);	
	jiffies = jiffies0;		
	bictcp_cong_avoid(sk1, 0, acked_b);

	copy_tcp(tp0, ca0, tp2, ca2);	
	agg_info.cwnd_at_rtt = tp2->snd_cwnd;
	agg_info.last_ack_time = jiffies;
	agg_info.total_acked = acked_b;
	bictcp_cong_avoid_agg(sk2, 0, &agg_info);

	//diff_result += __VERIFIER_assert_u32("[Verifier] Check same target...", ca1->bic_target, ca2->bic_target);
	diff_result += __VERIFIER_assert_u32("[Verifier1] Check same cwnd...", 	tp1->snd_cwnd, tp2->snd_cwnd);


	// // Induction step 
	important_msg("*** Induction Step ***\n");	
	u32 acked_i;
	__VERIFIER_nondet_u32(&acked_i, "acked", 2, tp0->snd_cwnd);	// induction variable

	copy_tcp(tp0, ca0, tp1, ca1);	
	jiffies = jiffies0;		
	agg_info.cwnd_at_rtt = tp1->snd_cwnd;
	agg_info.last_ack_time = jiffies;
	agg_info.total_acked = acked_i-1;
	bictcp_cong_avoid_agg(sk1, 0, &agg_info);
	bictcp_cong_avoid(sk1, 0, 1);

	copy_tcp(tp0, ca0, tp2, ca2);	
	agg_info.cwnd_at_rtt = tp2->snd_cwnd;
	agg_info.last_ack_time = jiffies;
	agg_info.total_acked = acked_i;
	bictcp_cong_avoid_agg(sk2, 0, &agg_info);


	//diff_result += __VERIFIER_assert_u32("[Verifier] Check same target...", ca1->bic_target, ca2->bic_target);
	diff_result += __VERIFIER_assert_u32("[Verifier2] Check same cwnd...", 	tp1->snd_cwnd, tp2->snd_cwnd);


///////// Print Path Information /////////////////////////////
#ifdef KLEE
	if (diff_result > 0){
		klee_print_range("[Done] Range of num_of_rtt:       ", sym_which_rtt);
		klee_print_range("[Done] Range of epoch_start:      ", sym_epoch_start);
		klee_print_range("[Done] Range of delay_min:        ", sym_delay_min);
		klee_print_range("[Done] Range of bic_K:            ", sym_bic_K);
		klee_print_range("[Done] Range of acked_i:         ", acked_i);
	}
#endif


	return 0;
}
#endif
