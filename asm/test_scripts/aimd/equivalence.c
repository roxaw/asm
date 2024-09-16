#ifndef script_equivalence_check
#define script_equivalence_check
#include "share.h"

extern struct tcp_congestion_ops tcp_reno;

u32 __VERIFIER_assert_u32(char *msg, u32 val1, u32 val2)
{
	char printmsg[1024];
	if (val1==val2){
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
	u32 MIN_CWND;
	u32 MAX_CWND;
	u32 MIN_SSTHRESH;
	u32 MAX_SSTHRESH;

   if( argc == 5 ) {
		MIN_CWND = atoi(argv[1]);
		MAX_CWND = atoi(argv[2]);
		MIN_SSTHRESH = atoi(argv[3]);
		MAX_SSTHRESH = atoi(argv[4]);
   } else {
		MIN_CWND = 10;
		MAX_CWND = 10;
		MIN_SSTHRESH = 1;
		MAX_SSTHRESH = 1;
   }

	printf("[Parameter] CWND(%u, %u), SSTHRESH(%u, %u)\n", 
		MIN_CWND, MAX_CWND, MIN_SSTHRESH, MAX_SSTHRESH);

	
///////// TCP initilization ///////////////////////

	struct tcp_sock tcp_sock_struct0; // 0: initialization
	struct sock *sk0 = (struct sock *)&tcp_sock_struct0;
	struct tcp_sock *tp0 = &tcp_sock_struct0;
	struct inet_connection_sock *icsk0 = inet_csk(sk0);

	struct tcp_sock tcp_sock_struct1; // 1:original
	struct sock *sk1 = (struct sock *)&tcp_sock_struct1;
	struct tcp_sock *tp1 = &tcp_sock_struct1;
	struct inet_connection_sock *icsk1 = inet_csk(sk1);

	struct tcp_sock tcp_sock_struct2; // 2:aggregation
	struct sock *sk2 = (struct sock *)&tcp_sock_struct2;
	struct tcp_sock *tp2 = &tcp_sock_struct2;
	struct inet_connection_sock *icsk2 = inet_csk(sk2);

	memset(&tcp_sock_struct0, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk0, &tcp_reno); 
	struct net net_namespace0;
	memset(&net_namespace0, 0, sizeof(struct net));
	tp0->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace0;
	tcp_sk_init(&net_namespace0);
	tcp_init_sock(sk0);
	if (icsk0->icsk_ca_ops->init)
		icsk0->icsk_ca_ops->init(sk0);
	tcp_ca_event(sk0, CA_EVENT_TX_START);

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

	memset(&tcp_sock_struct2, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk2, &tcp_reno); 
	struct net net_namespace2;
	memset(&net_namespace2, 0, sizeof(struct net));
	tp2->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace2;
	tcp_sk_init(&net_namespace2);
	tcp_init_sock(sk2);
	if (icsk2->icsk_ca_ops->init)
		icsk2->icsk_ca_ops->init(sk2);
	tcp_ca_event(sk2, CA_EVENT_TX_START);
	
///////// Variable Initilization ///////////////////////

	u32 sym_cwnd;			// tp
	u32 sym_ssthresh;		// tp
	u32 sym_cwnd_cnt;		// tp

	__VERIFIER_nondet_u32(&sym_cwnd, "cwnd", MIN_CWND, MAX_CWND);
	__VERIFIER_nondet_u32(&sym_ssthresh, "ssthresh", MIN_SSTHRESH, MAX_SSTHRESH);
	__VERIFIER_nondet_u32(&sym_cwnd_cnt, "cwnd_cnt", 0, sym_cwnd-1);

	// Initialize reno state
	tp0->snd_cwnd = sym_cwnd;
	tp0->snd_ssthresh = sym_ssthresh;
	tp0->snd_cwnd_cnt = sym_cwnd_cnt;

///////// Verify Equivalence /////////////////////////////
	u32 acked;
	u32 diff_result = 0;
	u32 agg_total_acked;

	// Base step
	important_msg("*** Base Step ***\n");	
	acked	= 1;								// induction variable

	copy_tcp(tp0, tp1);	
	tcp_reno_cong_avoid(sk1, 0, acked);

	agg_total_acked = acked;
	copy_tcp(tp0, tp2);	
	tcp_reno_cong_avoid_agg(sk2, 0, agg_total_acked);

	diff_result += __VERIFIER_assert_u32("[Verifier] Check same cwnd...", 	tp1->snd_cwnd, tp2->snd_cwnd);

	// Induction step
	important_msg("*** Induction Step ***\n");
	if 	(tp0->snd_cwnd > 1) {
		__VERIFIER_nondet_u32(&acked, "acked", 2, tp0->snd_cwnd);	// induction variable

		copy_tcp(tp0, tp1);	
		agg_total_acked = acked-1;
		tcp_reno_cong_avoid_agg(sk1, 0, agg_total_acked);
		tcp_reno_cong_avoid(sk1, 0, 1);

		copy_tcp(tp0, tp2);	
		agg_total_acked = acked;
		tcp_reno_cong_avoid_agg(sk2, 0, agg_total_acked);

		diff_result += __VERIFIER_assert_u32("[Verifier] Check same cwnd...", 	tp1->snd_cwnd, tp2->snd_cwnd);
	}

///////// Print Path Information /////////////////////////////
#ifdef KLEE
	if (diff_result>0) {
		klee_print_range("[Done] Range of cwnd:     ", sym_cwnd);
		klee_print_range("[Done] Range of ssthresh: ", sym_ssthresh);
		klee_print_range("[Done] Range of cwnd_cnt: ", sym_cwnd_cnt);
		klee_print_range("[Done] Range of acked:    ", acked);
	}
#endif

	return 0;
}
#endif
