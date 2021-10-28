# include <sys/types.h>
# include <sys/socket.h>
# include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <stdbool.h>

#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/timeb.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "common.h"


//Dans le cas où l'OS est un MAC, il faut rajouter des variables préprocesseur.
#ifdef __APPLE__
//#define NULL __DARWIN_NULL
#define INADDR_ANY (u_int32_t)0x00000000
#define EXIT_FAILURE 1
#define MSG_CONFIRM	0x800 // TODO : chercher la valeur
#endif

//Variable pré-pro. on ajoute le segment maximal pour l'envoie de TCP
#define MAXLINE 2048
//Variale pré-pro. on ajoute le numéro du port dont on va utiliser.
//On utilise seuleument le port 6666 pour myP2P coté serveur
#define PORT 6666




/*
* Fonciton permettant au serveur de faire les actions suivantes dès lors de la réception d'un publish:
*	- Enregistrer la donnée recu dans la ase de données (le fichier texte)
*	- On recalcul le hashcode avec les données recues
*	- On envoie le hascode vers le client comme acquittement
*	- On attend l'acquittement du client afin de voir si l'acquittement hashcode du serveur a bien été recu
*	- On attend 5s pour voir si on ne retourne pas un acquittement nous disant que les données sauvegardées sont fausses
*	 --> fork + signal (comme projet RS Système).
*/
int publish_response(int sockfd, struct sockaddr_in cliaddr) {


    socklen_t len = sizeof(cliaddr);

    //On prend l'ensemble des informations arrivant sur le réseau
    
    //On prend l'adresse ip
    char ip[20];
    int p2=recvfrom(sockfd, ip, sizeof(ip), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("IP : %s\n",ip );

    //On prend le type du fichier
    char type[5];
    int p4=recvfrom(sockfd, type, sizeof(type), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Type : %s\n",type );

    //On prend les mots clés
    char motclef[200];
    int p6=recvfrom(sockfd, motclef, sizeof(motclef), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Motclef : %s\n",motclef );

    //On prend le nom du fichier
    char dossier[100];
    int p3=recvfrom(sockfd, dossier, sizeof(dossier), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Dossier : %s\n",dossier );
    
    //On prend le hashcode
    char hashcode[41];
    int p5=recvfrom(sockfd, hashcode, sizeof(char)*41, MSG_WAITALL,( struct sockaddr *)&cliaddr,&len);
    printf("hash : %s\n",hashcode);





    //On concatène les données recus pour les mettre dans la bases de données
    //On sépare les données par "&" pour bien voir la séparation lors de la relecture des données
    //Le tout est enregistrer dans le fichier : fichier.txt
    char ligne[5000];
    strcat(ligne,ip);
    strcat(ligne, "&");
    strcat(ligne, dossier);
    strcat(ligne, "&");
    strcat(ligne, type);
    strcat(ligne, "&");
    strcat(ligne, hashcode);
    strcat(ligne, "&");
    strcat(ligne, motclef);
    strcat(ligne, "\n");

    printf("Ceci est la ligne : %s\n",ligne );


    FILE* file = fopen("fichier.txt", "a+");
    int fd = fileno(file);
    fputs(ligne, file);
    fclose(file);


    printf("Publish Done\n");
    printf("Envoie de l'aquittement\n");

    char ack[2] = "0";
    ack [1]= '\0';
    sendto(sockfd, ack, sizeof(ack),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));

    printf("ACK envoyé\n");


    //Mettre l'aquittement dans la fonction 
    //publish_ack(sockfd, cliaddr);

    return 0;

}


/*
*Fonction permettant au serveur de faire les actions suivantes dès de la réception d'un search :
*	- Réception du mot clé
*	- Recherche dans la base de données si une ligne contient ce mot clé :
*		- Si oui, on enregistre toute la ligne Data_t dans une liste de Data_t
*		- Sinon, on ne fait rien
*	- On envoie la liste liste de Data_t au client
*Fin
*
*Prend en entrée un char* (le mot clé)
*/


Data_t* newData(char* line) {
    /* On prend chaque élément de la ligne.
    On les range ensuite dans les variables appropriées de la struct doc.
    */

    printf("COucou\n");
    Data_t* doc = (Data_t*)malloc(sizeof(Data_t));

    char* word;

    // Pour séparer les mots d'une même ligne
    char* separator = "&";
    printf("COucou\n");
    // Adresse ip, sans oublier la conversion du char* en int
    // On prend ici le premier mot
    word = strtok(line, separator);
    doc->ip = word;

    printf("COucou\n");
    // Nom du dossier
    word = strtok(NULL, separator);
    doc->dossier = word;
    printf("COucou\n");
    // Type de document
    word = strtok(NULL, separator);
    doc->type = word;
    printf("COucou\n");
    // Hashcode
    word = strtok(NULL, separator);
    doc->hashcode = word;
    printf("COucou\n");
    // Mot-clef
    word = strtok(NULL, separator);
    doc->motclef = word;
    printf("COucou\n");

    return doc;
                     
}


int search_response(int sockfd, struct sockaddr_in cliaddr){

    socklen_t len = sizeof(cliaddr);
    char motclef[200];
    recvfrom(sockfd, motclef, sizeof(motclef), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Motclef : %s\n",motclef );

    FILE* file = fopen("fichier.txt", "r");

    char* searchedLine;
    int k = 0;

    char ack[2] = "0";
    ack [1]= '\0';

    char ackFin[2] = "1";
    ackFin[1]= '\0';

    char fichier_envoi[100];
    char ip_envoi[15];

    char* buffer = (char*)malloc(sizeof(char)*512);
    char* bufferTest = (char*)malloc(sizeof(char)*512);
    // Tant que le document n'est pas entièrement parcouru
    while (fgets(buffer, sizeof(char)*512 , file) != NULL) {


        printf("Buffer p1 : %s\n",buffer);
        // On recherche le motclef dans la ligne courante
        strcpy(bufferTest,buffer);
        searchedLine = strstr(bufferTest, motclef);
        //S'il y a 
        if (searchedLine != NULL) {
            printf("%s\n",searchedLine );
            printf("Buffer p2 : %s\n",buffer);
            // Extraction des noms de fichiers et IP


            char* word;
            char ip[15];

            // Pour séparer les mots d'une même ligne
            char* separator = "&";
            printf("COucou\n");

            // On prend prend l'IP de la base de données
            word = strtok(buffer, separator);
            printf("COucou\n");
            strcpy(ip,word);
            printf("Word pour dossier : %s\n",word );
            printf("COucou\n");



            // Nom du dossier
            word = strtok(NULL, separator);
            char dossier[100];
            printf("Word pour dossier : %s\n",word );
            printf("COucou\n");
            strcpy(dossier,word);
            printf("COucou\n");
            strcat(dossier,".");


            // Type de document
            word = strtok(NULL, separator);
            strcat(dossier,word);
            printf("COucou\n");


            strcpy(ip_envoi,ip);
            strcpy(fichier_envoi, dossier);

            
            sendto(sockfd, ack, sizeof(ack),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));

            sleep(0.5);

            sendto(sockfd, fichier_envoi, sizeof(fichier_envoi),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));

            sendto(sockfd, ip_envoi, sizeof(ip_envoi),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));

            printf("Fin de transmission d'une réponse\n");
        }

            

    }
    

    sendto(sockfd, ackFin, sizeof(ackFin),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));


    return 0;
}


/*
Data_t** search_response(int sockfd, struct sockaddr_in cliaddr) {

    static Data_t* liste[1024];

    //On alloue de la mémoire pour les lignes que l'on va récupérer dans le fichier
    char* buffer = (char*)malloc(sizeof(char)*512);

    socklen_t len = sizeof(cliaddr);
    char motclef[200];
    recvfrom(sockfd, motclef, sizeof(motclef), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

    printf("Motclef : %s\n",motclef );

    FILE* file = fopen("fichier.txt", "r");


    char* searchedLine;
    int k = 0;

    // Tant que le document n'est pas entièrement parcouru
    while (fgets(buffer, sizeof(file), file) != NULL) {

        // On recherche le motclef dans la ligne courante
        searchedLine = strstr(buffer, motclef);

        // Si le motclef est trouvé
        if (searchedLine != NULL) {

            liste[k] = newData(searchedLine);
            k += 1;


            Data_t* data = newData(searchedLine);

            char* ip = data->ip;
            char* nom = data->dossier;
            char* type = data->type;

            strcpy(buffer, ip);
            strcat(buffer, " : ");
            strcat(buffer, nom);
            strcat(buffer, ".");
            strcat(buffer, type);


            char num[1] = "1";
            sendto(sockfd, buffer, sizeof(buffer), 1, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));

            free(data);

        // Créer fonction pour extraire nom fichier et adresse IP (deux fonctions éventuellement), puis 2 sendto, un pour le fichier, un pour l'IP
        // À la fin du sendto, renvoi d'un ack de -1 pour dire au client qu'on a fini. L'ack est un sendto qui envoie juste le char* "-1"

        // Pour indiquer qu'on a trouvé le document
        //et aussi mettre fin à la boucle while
        
        }

        // On passe à la ligne suivante
        searchedLine = strtok(NULL, motclef);

    }

    free(buffer);

    // On ferme le fichier
    fclose(file);

    printf("Fin des résultats.\n");
    
    return liste;

}

*/

/*
FOnction permettant de créer la socket de connexion
*/
int creationsocketServeur(){

    int clientSocket;

    //PF = Protocole Family
    if ((clientSocket = socket(PF_INET, SOCK_STREAM,0))<0){

        perror("Erreur de création socket");
        exit(1);
    }
    return clientSocket;
}







void serveurfctn(){

printf(" ____                                       \n");
printf("/ ___|   ___  _ __ __   __ ___  _   _  _ __ \n");
printf("\\___ \\  / _ \\| '__|\\ \\ / // _ \\| | | || '__|\n");
printf(" ___) ||  __/| |    \\ V /|  __/| |_| || |   \n");
printf("|____/  \\___||_|     \\_/  \\___| \\__,_||_|   \n\n\n\n");

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
      
    int n;
  
    socklen_t len = sizeof(cliaddr);  //len is value/resuslt
  

    char num[1];

    while(true){
        printf("\n\n\n\n\nAttente d'un signal ... \n\n\n");
        int p1=recvfrom(sockfd, num, sizeof(num), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

        printf("Num : %s\n",num );


        if (!strcmp(num,"1")){
            printf("Publish response signal\n");
            publish_response(sockfd, cliaddr);
        }
        else if (!strcmp(num,"0")){
            printf("Search response signal\n");
            search_response(sockfd, cliaddr);


            //On a un signal search
        }
    }    

    fprintf(stderr,"RECU !!!!!");


    /*--------------ACK--------------*/


    sleep(1);

    char ack[2] = "0";
    ack [2]= '\0';
    sendto(sockfd, ack, sizeof(ack),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));

    printf("ACK envoyé\n");
    


}



