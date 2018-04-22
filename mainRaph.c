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
#include <ifaddrs.h>
#include <unistd.h> //fork
#include <sys/wait.h> //wait

/****
  **** STRUCTURE ICMP
  ****/
#define BUFSIZE 1500  /* Taille du MTU */
#define MAXLINE 80
int pid = -1;
char broadcastMin[20] = "";
char broadcastMax[20] = "";
char IPlist[2000] = "";
char myIP[20] = "";
char** tabIP;


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

char** str_split (char *s, const char *ct)
{
    char **tab = NULL;

    if (s != NULL && ct != NULL)
    {
        int i;
        char *cs = NULL;
        size_t size = 1;

        for (i = 0; (cs = strtok (s, ct)); i++)
        {
            if (size <= i + 1)
            {
                void *tmp = NULL;

                size <<= 1;
                tmp = realloc (tab, sizeof (*tab) * size);
                if (tmp != NULL)
                {
                    tab = tmp;
                }
                else
                {
                    fprintf (stderr, "Memoire insuffisante\n");
                    free (tab);
                    tab = NULL;
                    exit (EXIT_FAILURE);
                }
            }
            tab[i] = cs;
            s = NULL;
        }
 //       tab[i] = NULL;
    }
    return tab;
}

int relevant(char** tab) {
    for (int i=0; i<4; i++) {
        if ((strcmp(tab[i], "255")!=0)) {
            return i;
        }
    }
}

void setBroadcastAddr(char* ip, char* mask) {
    char** tabIP = str_split(ip, ".");
    char** tabMask = str_split(mask, ".");

    int relevantNb = relevant(tabMask);
    for (int j = 0; j< relevantNb; j++) {

        strcat(broadcastMin, tabIP[j]);
        strcat(broadcastMin, ".");
        strcat(broadcastMax, tabIP[j]);
        strcat(broadcastMax, ".");

    }
    for (int i = relevantNb; i < 4; i++) {
        strcat(broadcastMin, "0");
        strcat(broadcastMax, "255");
    }
    printf("\n Broadcast address :");
    fflush(stdout);
    printf(broadcastMin);
    fflush(stdout);
    printf("\n Max accessible IP address :");
    fflush(stdout);
    printf(broadcastMax);
    fflush(stdout);
}

char getIP ()
{
    struct ifaddrs *ifa2, *ifa;
    struct sockaddr_in *sa;
    char *addr, *addr2;
    char addressOutputBuffer[INET_ADDRSTRLEN];


    getifaddrs (&ifa2);
    for (ifa = ifa2; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_netmask;
            addr = inet_ntoa(sa->sin_addr);
            addr2 = inet_ntop(ifa->ifa_addr->sa_family, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr, addressOutputBuffer, sizeof(addressOutputBuffer));
            if (strcmp(ifa->ifa_name,"wlan0")==0) {  //A MODIFIER EN FONCTION DE LA CONNECTION (ex : wifi0)
                printf("\n Adresse ip sur wifi0 du PC : ");
                fflush(stdout);
                printf(addr2);
                fflush(stdout);
                printf("\n Masque sous réseau sur wifi0 du PC : ");
                fflush(stdout);
                printf(addr);
                fflush(stdout);
                strcpy(myIP,addr2);
                setBroadcastAddr(addr2, addr);

                return addr;
            }
        }
    }

    freeifaddrs(ifa2);
    return 0;
}

void sender(struct sockaddr_in *addr) {
    const int data = 255;
    int i;
    int compteur = 1;
    int sendRawSocket;
    struct sockaddr_in receiver_addr;
    struct packetICMP packet;
  //  printf("\n SENDER");
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
    int timer;
    for (timer=0; timer<3;timer++) {
        int len = sizeof(receiver_addr);

        bzero(&packet, sizeof(packet)); //On met le paquet à 0
        packet.hdr.type = ICMP_ECHO;    //Type Echo
        packet.hdr.un.echo.id = pid;
        for (i = 0; i < sizeof(packet.message) - 1; i++) {
            packet.message[i] = i + '0';    //On envoie une suite de 0
        }
        packet.message[i] = 0;          //On marque la fin du message
        packet.hdr.un.echo.sequence = compteur++;


        packet.hdr.checksum = checksum(&packet, sizeof(packet));
        if (sendto(sendRawSocket, &packet, sizeof(packet), 0, (struct sockaddr *) addr, sizeof(*addr)) <= 0) {
      //      perror("Error send");
      //      fflush(stdout);
        } else {
      //      printf("Sent !");
      //      fflush(stdout);
        }
        sleep(0.3);
    }
   return;
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

        if ((n = recvfrom(rawSocket, rcvbuffer, BUFSIZE, 0,
                          (struct sockaddr *) &fromAddr, &len)) < 0) {
            printf("erreur recvfrom");
            fflush(stdout);
            exit(1);
        } else {
            if (strcmp(inet_ntoa(fromAddr.sin_addr),myIP)==0) {

            }
            else {
  //            if (strcmp(IPlist[0],"NULL") == 0) {
  //              strcpy(IPlist[0], inet_ntoa(fromAddr.sin_addr));
  //              printf(IPlist[0] );
  //            }
  //            else {
  //              for (int k = 0; k<sizeof(IPlist)/sizeof(IPlist[0]); k++) {
  //                if (strcmp(IPlist[k], inet_ntoa(fromAddr.sin_addr))!=0) {
                    strcat(IPlist, strcat(inet_ntoa(fromAddr.sin_addr),"/"));

  //                }
  //              }
    //          }
    //            printf("\n Socket recu bg");
                printf("\n Adresse ayant envoyé le paquet : ");
                printf(inet_ntoa(fromAddr.sin_addr));
                fflush(stdout);
    //            printf(IPlist);
    //            fflush(stdout);
            }

        }
    }

}
int main (int argc, char *argv[]){
    getIP();
    pid = getpid();
    char** tabBas = str_split(broadcastMin, ".");
    char** tabMaxIP = str_split(broadcastMax, ".");

    char str1[4];
    char str2[4];
    char str3[4];
    char str4[4];



    int val1 = atoi(tabBas[0]);
    int val2 = atoi(tabBas[1]);
    int val3 = atoi(tabBas[2]);
    int val4 = atoi(tabBas[3]);

    int valMAX1 = atoi(tabMaxIP[0]);
    int valMAX2 = atoi(tabMaxIP[1]);
    int valMAX3 = atoi(tabMaxIP[2]);
    int valMAX4 = atoi(tabMaxIP[3]);
    if (fork() == 0) {
 /*       printf("\nReceveur\n");
        fflush(stdout);
  */      receiver();
    }
    else {

        for (int i1 = val1; i1 <= valMAX1; i1++) {
            for (int i2 = val2; i2 <= valMAX2; i2++) {
                for (int i3 = val3; i3 <= valMAX3; i3++) {
                    for (int i4 = val4+1; i4 <= valMAX4; i4++) {
                        sprintf(str1, "%d", i1);
                        sprintf(str2, "%d", i2);
                        sprintf(str3, "%d", i3);
                        sprintf(str4, "%d", i4);
                        strcat(str1, ".");
                        strcat(str1, str2);
                        strcat(str1, ".");
                        strcat(str1, str3);
                        strcat(str1, ".");
                        strcat(str1, str4);

                        struct sockaddr_in serv_addr;

                        bzero(&serv_addr, sizeof(serv_addr));
                        serv_addr.sin_family = PF_INET;
                        serv_addr.sin_addr.s_addr = inet_addr(str1);  //Adresse internet
                        serv_addr.sin_port = 0;  //Port number
    /*                    printf("\n Current send : ");
                        fflush(stdout);
                        printf(str1);
                        fflush(stdout);
*/

                        sender(&serv_addr);
                    }
                }
            }
        }

/*        for (int i=0; i<sizeof(IPlist)/sizeof(IPlist[0]);i++) {
          printf(IPlist[i]);
          fflush(stdout);
          printf("\n");
          fflush(stdout);
        }*/
  //      printf(IPlist);
  //      fflush(stdout);

  //      char** tabIP = str_split(IPlist, "/");

        wait(0);
    }



    return 0;
}
