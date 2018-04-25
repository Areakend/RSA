////////////////////////////////////////////////////////////
// IP scanning program used to find open ports and IP     //
// on the local network.                                  //
// This program sends ICMP messages through the network.  //
//                                                        //
// Programmed by Jeremy Hynes & Raphael Ohanian           //
// Telecom Nancy - Universite de Lorraine - 2018          //
////////////////////////////////////////////////////////////

#ifndef HEADER_PORTS
#define HEADER_PORTS

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <time.h>
#include <sys/timeb.h>
#include <netdb.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <resolv.h>
#include <ifaddrs.h>
#include <unistd.h> //fork
#include <sys/wait.h> //wait

/**** STRUCTURE ICMP ****/
#define BUFSIZE 1500  /* Taille du MTU */
#define MAXLINE 80

struct packetICMP
{
  struct icmphdr hdr;
  char message[BUFSIZE-sizeof(struct icmphdr)];
};

#endif
