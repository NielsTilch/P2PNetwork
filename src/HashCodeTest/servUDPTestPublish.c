// Server side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
  
#define PORT     6666
#define MAXLINE 1024

#include <openssl/sha.h>

typedef struct data{
    int typeData;
    char ip[20];
    char dossier[100];
    char nom_dossier[100];
    char type[10];
    char hash[SHA_DIGEST_LENGTH*2];
    char motclef[200];
}Data_t;
  
// Driver code
int main() {
    int sockfd;

    struct sockaddr_in servaddr, cliaddr;
      
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
      
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
      
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
      
    int len, n;
  
    len = sizeof(cliaddr);  //len is value/resuslt
  

    char num[1];
    int p1=recvfrom(sockfd, num, sizeof(num), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Num : %s\n",num );

    char ip[20];
    int p2=recvfrom(sockfd, ip, sizeof(ip), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("IP : %s\n",ip );

    char type[5];
    int p4=recvfrom(sockfd, type, sizeof(type), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Type : %s\n",type );

    char motclef[200];
    int p6=recvfrom(sockfd, motclef, sizeof(motclef), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Motclef : %s\n",motclef );

    char dossier[100];
    int p3=recvfrom(sockfd, dossier, sizeof(dossier), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Dossier : %s\n",dossier );
    
    char hashcode1[41];
    int p5=recvfrom(sockfd, hashcode1, sizeof(char)*41, MSG_WAITALL,( struct sockaddr *)&cliaddr,&len);
    printf("hash : %s\n",hashcode1 );


    fprintf(stderr,"RECU !!!!!");


    /*--------------ACK--------------*/


    sleep(1);

    char ack[2] = "0";
    ack [2]= '\0';
    sendto(sockfd, ack, sizeof(ack),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("ACK envoyé\n");

      
    return 0;
}