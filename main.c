#include     <stdio.h>
#include     <sys/types.h>
#include     <sys/socket.h>
#include     <netinet/in.h>
#include     <arpa/inet.h>
#include     <sys/uio.h>
#include     <time.h>
#include     <sys/timeb.h>
#include     <netdb.h>
#include     <stdlib.h>
#include     <strings.h>

#include     <netinet/in.h>
#include     <netinet/ip.h>
#include     <netinet/ip_icmp.h>
#include     <netinet/udp.h>

#define BUFSIZE 1500  /* Taille du MTU */
#define MAXLINE 80

void checkPorts(){
  int sockfd;
  int on=1;
  if ((sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP))<0) {
    perror("Socket Error: ");
    exit(1);
  }

  if (setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,&on,sizeof(on))<0) {
    perror("SocketOpt Error: ");
    exit(1);
  }

}

int main (int argc, char *argv[]){
  checkPorts();
  printf("Check Ports: Done.\n");
}
