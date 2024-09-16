/*************************************************************************
    > File Name: simple_inet_sock.h
  > Author: Wei Sun
  > Mail:sunweiflyus@gmail.com 
  > Created Time: Fri 23 Jun 2017 12:20:20 PM CDT
  > Comments: only contains the minimal data structs needed by TCP CA
 ************************************************************************/

#ifndef __SIMPLE_INET_SOCK_H__
#define __SIMPLE_INET_SOCK_H__
struct inet_sock {
/* sk and pinet6 has to be the first two members of inet_sock */
	struct sock		sk;
};
#endif
