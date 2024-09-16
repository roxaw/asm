/*
 * File Name: simple_sk_buff.h
 * Comments: adapted from include/linux/skbuff.h
 */

#ifndef __SIMPLE_SKBUFF_H__
#define __SIMPLE_SKBUFF_H__

typedef unsigned int sk_buff_data_t;

/**
 *	struct sk_buff - socket buffer
 *	@next: Next buffer in list
 *	@prev: Previous buffer in list
 *	@tstamp: Time we arrived/left
 *	@rbnode: RB tree node, alternative to next/prev for netem/tcp
 *	@sk: Socket we are owned by
 *	@dev: Device we arrived on/are leaving by
 *	@cb: Control buffer. Free for use by every layer. Put private vars here
 *	@_skb_refdst: destination entry (with norefcount bit)
 *	@sp: the security path, used for xfrm
 *	@len: Length of actual data
 *	@data_len: Data length
 *	@mac_len: Length of link layer header
 *	@hdr_len: writable header length of cloned skb
 *	@csum: Checksum (must include start/offset pair)
 *	@csum_start: Offset from skb->head where checksumming should start
 *	@csum_offset: Offset from csum_start where checksum should be stored
 *	@priority: Packet queueing priority
 *	@ignore_df: allow local fragmentation
 *	@cloned: Head may be cloned (check refcnt to be sure)
 *	@ip_summed: Driver fed us an IP checksum
 *	@nohdr: Payload reference only, must not modify header
 *	@pkt_type: Packet class
 *	@fclone: skbuff clone status
 *	@ipvs_property: skbuff is owned by ipvs
 *	@offload_fwd_mark: Packet was L2-forwarded in hardware
 *	@offload_l3_fwd_mark: Packet was L3-forwarded in hardware
 *	@tc_skip_classify: do not classify packet. set by IFB device
 *	@tc_at_ingress: used within tc_classify to distinguish in/egress
 *	@tc_redirected: packet was redirected by a tc action
 *	@tc_from_ingress: if tc_redirected, tc_at_ingress at time of redirect
 *	@peeked: this packet has been seen already, so stats have been
 *		done for it, don't do them again
 *	@nf_trace: netfilter packet trace flag
 *	@protocol: Packet protocol from driver
 *	@destructor: Destruct function
 *	@tcp_tsorted_anchor: list structure for TCP (tp->tsorted_sent_queue)
 *	@_nfct: Associated connection, if any (with nfctinfo bits)
 *	@nf_bridge: Saved data about a bridged frame - see br_netfilter.c
 *	@skb_iif: ifindex of device we arrived on
 *	@tc_index: Traffic control index
 *	@hash: the packet hash
 *	@queue_mapping: Queue mapping for multiqueue devices
 *	@xmit_more: More SKBs are pending for this queue
 *	@pfmemalloc: skbuff was allocated from PFMEMALLOC reserves
 *	@active_extensions: active extensions (skb_ext_id types)
 *	@ndisc_nodetype: router type (from link layer)
 *	@ooo_okay: allow the mapping of a socket to a queue to be changed
 *	@l4_hash: indicate hash is a canonical 4-tuple hash over transport
 *		ports.
 *	@sw_hash: indicates hash was computed in software stack
 *	@wifi_acked_valid: wifi_acked was set
 *	@wifi_acked: whether frame was acked on wifi or not
 *	@no_fcs:  Request NIC to treat last 4 bytes as Ethernet FCS
 *	@csum_not_inet: use CRC32c to resolve CHECKSUM_PARTIAL
 *	@dst_pending_confirm: need to confirm neighbour
 *	@decrypted: Decrypted SKB
 *	@napi_id: id of the NAPI struct this skb came from
 *	@secmark: security marking
 *	@mark: Generic packet mark
 *	@vlan_proto: vlan encapsulation protocol
 *	@vlan_tci: vlan tag control information
 *	@inner_protocol: Protocol (encapsulation)
 *	@inner_transport_header: Inner transport layer header (encapsulation)
 *	@inner_network_header: Network layer header (encapsulation)
 *	@inner_mac_header: Link layer header (encapsulation)
 *	@transport_header: Transport layer header
 *	@network_header: Network layer header
 *	@mac_header: Link layer header
 *	@tail: Tail pointer
 *	@end: End pointer
 *	@head: Head of buffer
 *	@data: Data head pointer
 *	@truesize: Buffer size
 *	@users: User count - see {datagram,tcp}.c
 *	@extensions: allocated extensions, valid if active_extensions is nonzero
 */
 
struct sk_buff {
	 union {
		struct {
			/* These two members must be first. */
			struct sk_buff		*next;
			struct sk_buff		*prev;

	   /*     union {*/
				//ktime_t		tstamp;
				//struct skb_mstamp skb_mstamp;
			/*};*/
		};
		//struct rb_node	rbnode; [> used in netem & tcp stack <]
	};
	struct sock		*sk;

	union {
		//struct net_device	*dev;
		/* Some protocols might use this space to store information,
		 * while device pointer would be NULL.
		 * UDP receive path is one user.
		 */
		unsigned long		dev_scratch;
	};
	/*
	 * This is the control buffer. It is free to use for every
	 * layer. Please put your private variables there. If you
	 * want to keep them across layers you have to do a skb_clone()
	 * first. This is owned by whoever has the skb queued ATM.
	 */
	char cb[48] __aligned(8);
	
	__u16			inner_transport_header;
	//__u16			inner_network_header;
	//__u16			inner_mac_header;
	__be16			protocol;
	__u16			transport_header;
	//__u16			network_header;
	//__u16			mac_header;
	
	/* These elements must be at the end, see alloc_skb() for details.  */
	//sk_buff_data_t	tail;
	sk_buff_data_t	end;
	unsigned char	*head,
					*data;
	//unsigned int	truesize;
	//refcount_t		users;
};

struct skb_mstamp{
	union{
		u64 v64;
	struct {
		u32 stamp_us;
		u32 stamp_jiffies;
	};
};
};

// <M>
/* This data is invariant across clones and lives at
 * the end of the header data, ie. at skb->end.
 */
struct skb_shared_info {
	__u8		__unused;
	__u8		meta_len;
	__u8		nr_frags;
	__u8		tx_flags;
	unsigned short	gso_size;
	/* Warning: this field is not always filled in (UFO)! */
	unsigned short	gso_segs;
	struct sk_buff	*frag_list;
	//struct skb_shared_hwtstamps hwtstamps;
	unsigned int	gso_type;
	u32		tskey;
	/*
	 * Warning : all fields before dataref are cleared in __alloc_skb()
	 */
	//atomic_t	dataref;
	/* Intermediate layers must ensure that destructor_arg
	 * remains valid until skb destructor */
	void *		destructor_arg;
	/* must be last field, see pskb_expand_head() */
	//skb_frag_t	frags[MAX_SKB_FRAGS];
};

static inline unsigned char *skb_transport_header(const struct sk_buff *skb)
{
	return skb->head + skb->transport_header;
}

static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->head + skb->end;
}
static inline unsigned int skb_end_offset(const struct sk_buff *skb)
{
	return skb->end;
}

/* Internal */
#define skb_shinfo(SKB)	((struct skb_shared_info *)(skb_end_pointer(SKB)))
#endif
