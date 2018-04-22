#ifndef ICMPSOCKRAW

#define ICMPSOCKRAW
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

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#include <unistd.h>
#include <string.h>

#define BUFSIZE 1500  /* Taille du MTU */
#define MAXLINE 80

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

void checkPorts();
void sendMsg(int , struct sockaddr_in, char * );

#endif
