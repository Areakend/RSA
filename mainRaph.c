//
// Created by Zaven on 15/04/2018.
//

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
#include<string.h>

#include     <netinet/ip.h>
#include     <netinet/ip_icmp.h>
#include     <netinet/udp.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <unistd.h> //fork
#include <sys/wait.h> //wait

/****
  **** STRUCTURE ICMP
  ****/
#define BUFSIZE 1500  /* Taille du MTU */
#define MAXLINE 80
int pid = -1;

struct packetICMP
{
    struct icmphdr hdr;
    char message[BUFSIZE-sizeof(struct icmphdr)];
};

unsigned short checksum(void *b, int len)
{	unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;

    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

char PrintIp()
{
    struct hostent * host;
    struct in_addr addr;
    char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) == 0)
        printf("%s\n", hostname);
    if ((host = gethostbyname(hostname)) != NULL)
    {
        int i;

        for(i = 0; host->h_addr_list[i] != NULL; i++)
        {
            memcpy(&addr.s_addr, host->h_addr_list[i], sizeof(addr.s_addr));
            printf("IP : %s\n", inet_ntoa(addr));
        }
    }
    else
        printf("La fonction gethostbyname a echoue.\n");
}

void sender(struct sockaddr_in *addr) {
    const int data = 255;
    int i;
    int conteur = 1;
    int sendRawSocket;
    struct sockaddr_in receiver_addr;
    struct packetICMP packet;
    printf("\n SENDER");
    fflush(stdout);


    if ((sendRawSocket = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("erreur send socket");
        fflush(stdout);
        exit(1);
    }

    if (setsockopt(sendRawSocket, SOL_IP, IP_TTL, &data, sizeof(data)) != 0) {
        perror("Set TTL option");
        fflush(stdout);
        exit(1);
    }

    for (;;) {
    int len = sizeof(receiver_addr);

    printf("Msg #%d\n", conteur);
/*    if (recvfrom(rcvRawSocket, &packet, sizeof(packet), 0, (struct sockaddr *) &receiver_addr, &len) > 0) {
        printf("***Got message!***\n");
        fflush(stdout);

    } else {
        printf("non envoyé");
        fflush(stdout);
    } */


    bzero(&packet, sizeof(packet)); //On met le paquet à 0
    packet.hdr.type = ICMP_ECHO;    //Type Echo
    packet.hdr.un.echo.id = pid;
    for (i = 0; i < sizeof(packet.message) - 1; i++) {
        packet.message[i] = i + '0';    //On envoie une suite de 0
    }
    packet.message[i] = 0;          //On marque la fin du message
    packet.hdr.un.echo.sequence = conteur++;


    packet.hdr.checksum = checksum(&packet, sizeof(packet));
    if (sendto(sendRawSocket, &packet, sizeof(packet), 0, (struct sockaddr *) addr, sizeof(*addr)) <= 0) {
          perror("Error send");
        fflush(stdout);
    }
else {
        printf("Sent !");
        fflush(stdout);
    }
        sleep(1);
   }
}

void receiver(void) {
    int rawSocket, n;
    struct sockaddr_in fromAddr;
    socklen_t len;
    char source[BUFSIZE], rcvbuffer[BUFSIZE];;
    struct ip *ip, *ip2;
    struct icmp *icmp;
    struct udphdr *udp;

    if ((rawSocket = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        perror("erreur receive socket");
        fflush(stdout);

        exit(1);
    }


    for (;;) {
        len = sizeof(fromAddr);
        bzero(rcvbuffer, sizeof(rcvbuffer));
        printf("\n Envoi");
        fflush(stdout);
        if ((n = recvfrom(rawSocket, rcvbuffer, BUFSIZE, 0,
                          (struct sockaddr *) &fromAddr, &len)) < 0) {
            printf("erreur recvfrom");
            fflush(stdout);
            exit(1);
        } else {
            printf("\n Socket recu bg");
            printf("\n Adresse : ");
            printf(inet_ntoa(fromAddr.sin_addr));
            fflush(stdout);

        }

        /*       if ( inet_ntop (PF_INET, (const void *)&fromAddr.sin_addr,source,sizeof(source)) <0) {
                   printf ("erreur inet_ntop");
                   exit (1); }
               printf( "%d octets ICMP de %s: \n", n, source);

               ip =  (struct ip*)rcvbuffer;                           // debut entete IP
               lenIPHeader = ip->ip_hl * 4 ;       // ip->ip_hl longueur  en mot de 32 bits

               icmp = (struct icmp*) (rcvbuffer + lenIPHeader) ;             // debut entete ICMP
               ip2 =  (struct ip*) icmp + 8;               // debut en-tete IP contenu dans ICMP
               lenIPHeader2 = ip->ip_hl * 4 ;       // longueur  en-tete IP

               if (ip2->ip_p == IPPROTO_UDP) {
                   udp = (struct udphdr *) (rcvbuffer + lenIPHeader + 8 + lenIPHeader2);         // debut en-tete UDP
                   sport = ntohs(udp->source);
                   dport = ntohs(udp->dest) ;
                   printf (" port source = %d et port destination = %d \n", sport, dport);
               }
               switch (icmp->icmp_type) {
                   case ICMP_UNREACH: {
                       printf ("destination unreachable \n");
                       switch (icmp->icmp_code){
                           case ICMP_UNREACH_PORT:
                               printf (" bad port \n");
                               break;
                           default:
                               printf ("type %d, code = %d\n", icmp->icmp_type,
                                       icmp->icmp_code);
                               break;
                       } }
                       break;
                   case ICMP_ECHO:
                       printf (" echo service  \n");
                       break;
                   case ICMP_ECHOREPLY :
                       printf (" echo reply  \n");
                       break;
                   case ICMP_TIMXCEED :
                       printf (" Time Exceed \n");
                       break;
                   default:
                       printf ("type %d, code = %d\n", icmp->icmp_type,
                               icmp->icmp_code);
               }
       */
    }
}
int main (int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
PrintIp();
        pid = getpid();
        bzero(&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = PF_INET;
        serv_addr.sin_addr.s_addr = inet_addr("192.168.56.10");  //Adresse internet  #192.168.1.19 , 192.168.56.1
        serv_addr.sin_port = 0;  //Port number
        if (fork() == 0) {
            printf("hello receiver");
            fflush(stdout);
            receiver();
        }
        else {
            printf("Hello sender");
            fflush(stdout);
            sender(&serv_addr);
        }

        wait(0);


    return 0;
}

