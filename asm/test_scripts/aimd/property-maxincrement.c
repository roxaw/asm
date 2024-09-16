#ifndef script_property
#define script_property
#include "share.h"

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

void print_tcp(char *msg, struct tcp_sock *tp){

	printf("%s: cwnd %u, snd_cwnd_cnt %u\n", msg,
		tp->snd_cwnd, tp->snd_cwnd_cnt);
}


int main( int argc, char *argv[] ) 
{
///////// Parameter initilization ///////////////////////

	bool ENABLE_AGG; // 0: normal verification 1: aggregation verification

	u32 MIN_CWND;
	u32 MAX_CWND;
	u32 MIN_SSTHRESH;
	u32 MAX_SSTHRESH;

	if( argc == 6 ) {
		if (argv[1][0]=='A')
			ENABLE_AGG = true;
		else
			ENABLE_AGG = false;
		MIN_CWND = atoi(argv[2]);
		MAX_CWND = atoi(argv[3]);
		MIN_SSTHRESH = atoi(argv[4]);
		MAX_SSTHRESH = atoi(argv[5]);
   	} else {
		ENABLE_AGG = false;
		MIN_CWND = 9;
		MAX_CWND = 9;
		MIN_SSTHRESH = 20;
		MAX_SSTHRESH = 20;
   	}

	printf("[Parameter] AGG %u, CWND(%u, %u), SSTHRESH(%u, %u)\n", 
		ENABLE_AGG, MIN_CWND, MAX_CWND, MIN_SSTHRESH, MAX_SSTHRESH);

///////// TCP initilization ///////////////////////
	struct tcp_sock tcp_sock_struct; 
	struct sock *sk = (struct sock *)&tcp_sock_struct;
	struct tcp_sock *tp = &tcp_sock_struct;
	struct inet_connection_sock *icsk = inet_csk(sk);

	memset(&tcp_sock_struct, 0, sizeof(struct tcp_sock));
	tcp_set_congestion_control(sk, &tcp_reno); 
	struct net net_namespace;
	memset(&net_namespace, 0, sizeof(struct net));
	tp->inet_conn.icsk_inet.sk.sk_net.net = &net_namespace;
	tcp_sk_init(&net_namespace);
	tcp_init_sock(sk);
	if (icsk->icsk_ca_ops->init)
		icsk->icsk_ca_ops->init(sk);
	tcp_ca_event(sk, CA_EVENT_TX_START);


///////// Symbolic Variable Initilization ///////////////////////
	u32 sym_cwnd;			// tp
	u32 sym_ssthresh;		// tp
	u32 sym_cwnd_cnt;		// tp

	__VERIFIER_nondet_u32(&sym_cwnd, "cwnd", MIN_CWND, MAX_CWND);
	__VERIFIER_nondet_u32(&sym_ssthresh, "ssthresh", MIN_SSTHRESH, MAX_SSTHRESH);
	__VERIFIER_nondet_u32(&sym_cwnd_cnt, "cwnd_cnt", 0, sym_cwnd-1); 

	// Initialize reno state
	tp->snd_cwnd = sym_cwnd;
	tp->snd_ssthresh = sym_ssthresh;
	tp->snd_cwnd_cnt = sym_cwnd_cnt;

///////// Verify Property /////////////////////////////
	u32 diff_result = 0;
	u32 agg_total_acked;

	if (ENABLE_AGG){
		agg_total_acked = sym_cwnd;
		tcp_reno_cong_avoid_agg(sk, 0, agg_total_acked);
	} else {
		int i;
		for(i=0; i<sym_cwnd; i++){
			tcp_reno_cong_avoid(sk, 0, 1);
		}
	}

	if (sym_cwnd < sym_ssthresh)
		diff_result = __VERIFIER_assert("[Verifier] Condition tp->snd_cwnd <= 2*sym_cwnd", tp->snd_cwnd <= 2*sym_cwnd);
	else
		diff_result = __VERIFIER_assert("[Verifier] Condition tp->snd_cwnd <= sym_cwnd+1", tp->snd_cwnd <= sym_cwnd+1);

///////// Print Path Information /////////////////////////////
#ifdef KLEE
	if (diff_result>0) {
		klee_print_range("[Done] Range of cwnd:     ", sym_cwnd);
		klee_print_range("[Done] Range of ssthresh: ", sym_ssthresh);
		klee_print_range("[Done] Range of cwnd_cnt: ", sym_cwnd_cnt);
	}
#endif

	return 0;
}
#endif
