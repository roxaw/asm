#ifndef simple_tcp
#define simple_tcp
long sysctl_tcp_mem[3] __read_mostly;
int sysctl_tcp_wmem[3] __read_mostly;
int sysctl_tcp_rmem[3] __read_mostly;

EXPORT_SYMBOL(sysctl_tcp_mem);
EXPORT_SYMBOL(sysctl_tcp_rmem);
EXPORT_SYMBOL(sysctl_tcp_wmem);



/* Address-family independent initialization for a tcp_sock.
 *
 * NOTE: A lot of things set to zero explicitly by call to
 *       sk_alloc() so need not be done here.
 */
void tcp_init_sock(struct sock *sk)
{
	struct inet_connection_sock *icsk = inet_csk(sk);
	struct tcp_sock *tp = tcp_sk(sk);

	/*tp->out_of_order_queue = RB_ROOT;*/
	/*tcp_init_xmit_timers(sk);*/
	/*tcp_prequeue_init(tp);*/
	/*INIT_LIST_HEAD(&tp->tsq_node);*/

	/*icsk->icsk_rto = TCP_TIMEOUT_INIT;*/
	/*tp->mdev_us = jiffies_to_usecs(TCP_TIMEOUT_INIT);*/
	/*minmax_reset(&tp->rtt_min, tcp_time_stamp, ~0U);*/

	/* So many TCP implementations out there (incorrectly) count the
	 * initial SYN frame in their delayed-ACK and congestion control
	 * algorithms that we must have the following bandaid to talk
	 * efficiently to them.  -DaveM
	 */
	tp->snd_cwnd = TCP_INIT_CWND;

	/* There's a bubble in the pipe until at least the first ACK. */
	tp->app_limited = ~0U;

	/* See draft-stevens-tcpca-spec-01 for discussion of the
	 * initialization of these values.
	 */
	tp->snd_ssthresh = TCP_INFINITE_SSTHRESH;
	tp->snd_cwnd_clamp = ~0;
	tp->mss_cache = TCP_MSS_DEFAULT;

	tp->reordering = sock_net(sk)->ipv4.sysctl_tcp_reordering;
	tcp_assign_congestion_control(sk);

	tp->tsoffset = 0;

	sk->sk_state = TCP_CLOSE;

	/*sk->sk_write_space = sk_stream_write_space;*/
	sock_set_flag(sk, SOCK_USE_WRITE_QUEUE);

	//icsk->icsk_sync_mss = tcp_sync_mss;

	sk->sk_sndbuf = sysctl_tcp_wmem[1];
	sk->sk_rcvbuf = sysctl_tcp_rmem[1];

	/*sk_sockets_allocated_inc(sk);*/
}
EXPORT_SYMBOL(tcp_init_sock);




#endif /* ifndef simple_tcp

















 */

