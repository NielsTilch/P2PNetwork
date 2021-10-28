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


#include <openssl/sha.h>

#include "common.h"

//Dans le cas où l'OS est un MAC, il faut rajouter des variables préprocesseur.
#ifdef __APPLE__
#define NULL __DARWIN_NULL
#define INADDR_ANY (u_int32_t)0x00000000
#define EXIT_FAILURE 1
#define SHA_DIGEST_LENGTH 20
#endif

#define MAXLINE 2048

//Port dédié aux demandes UDP
#define PORT 6666

//Port d'attente d'un signal pour une demande TCP
#define PORT_WAIT_TCP 4269

//Port dédié pour le transfert des fichiers TCP.
#define PORT_TRANSFERT_TCP 7777



typedef struct dataRecv{
	char* fichier;
	char* ip;
}DataRecv_t;




/*
On créé la socket de connexion
*/
int creationsocket(){

    int serverSocket;
    serverSocket = socket(PF_INET, SOCK_STREAM,0);
    if ((serverSocket = socket(PF_INET, SOCK_STREAM,0))<0){
    	printf("COucouERROR");
        perror("Erreur de création socket");
        exit(1);
    }
    return serverSocket;
}



char* getmyIP(){
  FILE *fp;

  //On excécute la commande 'hostname -I' qui nous donne l'adresse IP dont on est connecté puis on on l'a met dans le fichier ip.txt.
  if (fork()==0){
  	execlp("sh", "sh", "-c", "hostname -I > ip.txt",NULL);
  	exit(0);
  	}
  

  //On charge le fichier ip.txt pour récupérer l'adresse ip
  FILE *file = fopen("ip.txt","r");

  char* IP = (char *)malloc(sizeof(char)*16);

  fgets(IP,16,file);



  //On enlève le \n en fin de phrase de l'IP que l'on a recu
  char* IPsend = (char *)malloc(sizeof(char)*16);

  for (int i=0; i<16;i++){
  	if (IP[i] == '\n' || IP[i] == ' '){
  		break;
  	}
  	IPsend[i] = IP[i];
  }

  return IPsend;
}

int demandeTCPFichier (char* fichier, char* ip_send){

	/*
	Partie réception du fichier après demande via UDP :

	socket()
	bind()
	listen()
	accept()

	Boucle while jusqu'à attendre une valeur -1 montrant la fin du fichier{}
		recv()
	}

	Aquittement 0;
	close()
	
	Ce que l'on peut faire en plus:
	- Sécutité dès le lors que la connexion entre le client A et le client B ne fonctionne pas --> Timeout

	*/


	/*On crée la socket*/
	int socket_fd;
    if ( (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }


    int ttl=1;
	if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) <0){
		perror ("setsockopt");
		exit (1);
	}



    /*Configuration de l'adresse d'envoi*/
    struct sockaddr_in  serv_addr;
    memset(&serv_addr,0, sizeof(serv_addr) );

  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr = inet_addr((const char*)ip_send);
  	serv_addr.sin_port = htons(PORT_WAIT_TCP);

  	sleep(1);
  	//On envoie l'aquittement de demande de transmission TCP
  	char num[2] = "1";
  	num[2]='\0';
  	sendto(socket_fd, num, sizeof(num),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));


  	char ack[2];
  	int len = sizeof(serv_addr);
    int p5=recvfrom(socket_fd, ack, sizeof(ack), 0,( struct sockaddr *)&serv_addr,&len);
    sleep(1);
    printf("ACK demande : %s\n",ack);
    if (!strcmp(ack,"0")){
    	printf("On fait l'envoie fichier : %s\n",fichier);
    	char fichierEnvoie [110];
    	strcpy(fichierEnvoie,fichier);
    	sendto(socket_fd, fichierEnvoie, sizeof(fichierEnvoie),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

    	char ipToTcp[15];
    	char* ip = getmyIP();
    	strcpy(ipToTcp,ip);
    	printf("On fait l'envoie IP\n");
    	sendto(socket_fd, ipToTcp, sizeof(ipToTcp),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));


    	char ackFichier[3] ="-1";
    	//recvfrom(socket_fd, ack, sizeof(ack), 0,( struct sockaddr *)&serv_addr,&len);
    	

    	if (!strcmp(ackFichier,"-2")){
    		printf("Le fichier n'existe plus (selon client).\n");
    	}
    	else if (!strcmp(ackFichier,"-1")){
    		//On débute la réception du fichier TCP
    		/*On crée la socket tcp */
			int socket_tcp;
		    if ( (socket_tcp = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
		        perror("socket creation failed");
		        exit(EXIT_FAILURE);
		    }
		    struct sockaddr_in servtcp_addr;

		    servtcp_addr.sin_family = AF_INET;
  			servtcp_addr.sin_addr.s_addr = inet_addr((const char*)ip_send);
  			servtcp_addr.sin_port = htons(PORT_TRANSFERT_TCP);

  			// Bind the socket with the server address
		    if ( bind(socket_tcp, (const struct sockaddr *)&servtcp_addr, 
		            sizeof(servtcp_addr)) < 0 )
		    {
		        perror("bind failed");
		        exit(EXIT_FAILURE);
		    }
		    //On attend le signal TCP pour le transfert du fichier

		    if (listen(socket_tcp,SOMAXCONN)==0){
		    	printf("En attente de la donnée d'initialisation ...\n");
		    }else{
		    	perror("Listening fail");
		    	exit(EXIT_FAILURE);
		    }

		    struct sockaddr_in  new_addr;

		    socklen_t addr_size = sizeof(new_addr);
		    int new_socket = accept(socket_tcp, (struct sockaddr*)&new_addr, &addr_size);
		    //On va écrire les fichiers
		    char subfichier[110];
		    strcpy(subfichier,fichier);
		    char name_file;
		    char subname_file[110];
		    name_file=strtok(fichier,"/");

		    FILE* target = fopen(subname_file, "w");

		    int donn;
		    int p5;

		    //On copie les données dans le fichier
		    printf("Writing ...\n");
		    while(1){

    			recv(new_socket, donn, sizeof(donn),0);
 				//printf("Sending ...\n");
 				if(donn != -1){
 					printf("%d\n",donn);
 					fputc(donn,target);

 				}else{ //S'il on recoit la valeur -1, la transmission est terminée

 					break;

 				}


 			}
 			fclose(target);
 			close(socket_tcp);


    	}

    }



/*

	//À trier le contenu !
	FILE *source, *target;
	int i;
	source = fopen("./../Documents/untitled_1.odt", "rb"); 

	if( source == NULL ) { printf("Press any key to exit...\n");} //exit(EXIT_FAILURE); 

	fseek(source, 0, SEEK_END);
	int length = ftell(source);

	fseek(source, 0, SEEK_SET);
	target = fopen("./test.odt", "wb"); 

	if( target == NULL ) { fclose(source); } //exit(EXIT_FAILURE);

	printf("Longueur : %d\n",length);

	int cool;

	for(i = 0; i < length; i++){

		cool = fgetc(source);

		printf("cool : %d\n",cool);

	    fputc(cool, target);
	}

	printf("File copied successfully.\n"); 
	fclose(source); 
	fclose(target);
	*/


}

int recevoirDemandeTCP(char* fichier, char* ip_send){

	/*
	Réception de la demande par UDP puis envoie du fichier par TCP --> Faire un fork au début pour avoir un client dynamique:
	(Attente d'une transmission comme le serveur fait (un chiffre si possible sur un port différent de 6666))

	Partie envoie du client:
	
	socket()
	connect()
	send()

	Aquittement -1 pour dire que le fichier est terminer
	Attente aquittement 0 pour dire que l'aquittement -1 est bien recu 

	close()



	Ce que l'on peut faire en plus:
	- Sécutité dès le lors que la co entre le client A et le client B ne marche pas --> Timeout

	*/
	sleep(0.5);
	int socket_tcp;
	if ( (socket_tcp = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
	    perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in servtcp_addr;

	memset(&servtcp_addr,0, sizeof(servtcp_addr) );
	servtcp_addr.sin_family = AF_INET;
	servtcp_addr.sin_addr.s_addr = inet_addr((const char*)ip_send);
	servtcp_addr.sin_port = htons(PORT_TRANSFERT_TCP);


	if (connect(socket_tcp, (struct sockaddr*)&servtcp_addr, sizeof(servtcp_addr)) <0){
		perror("Error connect");
		exit(EXIT_FAILURE);
	}

	//À trier le contenu !
	FILE *source;
	source = fopen(fichier, "rb"); 

	fseek(source, 0, SEEK_END);
	int length = ftell(source);

	fseek(source, 0, SEEK_SET);

	int data;
	sleep(1);
	printf("Sending ...\n");
	for(int i = 0; i < length; i++){

		data = fgetc(source);

		send(socket_tcp, data, sizeof(data),0);
		//printf("Sending ...\n");

	}

	data = -1;

	send(socket_tcp, data, sizeof(data),0);

	printf("Transfert de %s est terminé.\n",fichier);

	
	fclose(source);
	close(socket_tcp);

}


/*
Fonction opermettant de chercher les informations nécessaire pour trouver un fichier spécifique 
selon les demandes de l'utilisateur
*/
int search (char* mots,char* ip_serveur){

    /*On créé la socket*/

	int socket_fd;
    if ( (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }


    int ttl=1;
	if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) <0){
		perror ("setsockopt");
		exit (1);
	}



    /*Configuration de l'adresse d'envoie*/
    struct sockaddr_in  serv_addr;
    memset(&serv_addr,0, sizeof(serv_addr) );
  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr = inet_addr((const char*)ip_serveur);
  	serv_addr.sin_port = htons(PORT);


  	//On envoie la donnée
  	int n,len;

  	char hashcode[40];

  	char ip_send[15];

  	char fichier_envoi[100];
    char ip_envoi[15];

  	while(true){
  		char num[2] = "0";
  		num[1] = '\0';
  		sendto(socket_fd, num, sizeof(num),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

  		sleep(1);

	    //On envoie les mots clé
	    char motclef[200];
	    strcpy(motclef,mots);
	    sendto(socket_fd, motclef, sizeof(motclef), 0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));


  		struct sockaddr_in cliaddr;
  		memset(&cliaddr, 0, sizeof(cliaddr));

  		int i=0;

  		//On créé liste des propositions 
  		struct dataRecv listProp[100]; 
    	

    	while(true){

    		char ack[2];
    		recvfrom(socket_fd, ack, sizeof(ack), 0,( struct sockaddr *)&serv_addr,&len);

    		printf("ACK : %s\n",ack);


    		if (!strcmp(ack,"0")){
    			printf("Attente Donnee fichier\n");
    			recvfrom(socket_fd, fichier_envoi, sizeof(fichier_envoi), 0,( struct sockaddr *)&serv_addr,&len);

    			recvfrom(socket_fd, ip_envoi, sizeof(ip_envoi), 0,( struct sockaddr *)&serv_addr,&len);

    			printf("Proposition %d\n",i);
    			printf("Fichier : %s\n",fichier_envoi );
    			printf("IP : %s\n\n",ip_envoi );
    			listProp[i].fichier = fichier_envoi;
    			listProp[i].ip=ip_envoi;

    			i++;
    		}
    		else{
    			break;
    		}

    		
    		//Pour chaque réception (nom du fichier et adresse ip client qu'il a) -> un numéro, char* nom du fichier et un char* ip
    		//À chaque fois qu'un arrive , --> mettre cette information dans une liste



    		//Fin marqué un "-1"
    	}
    	//L'utilisateur fait un choix entre les infos recu (entre 1 et n)
    	int commande;
    	while(true){
    		if(i==0){
    			printf("Pas de fichier ayant le mot clef ...\n");
    			return 0;
    		}

    		printf("\nProposition choisi > ");
        	scanf("%d",&commande);

        	printf("\n proposition : %d\n",commande);
        	printf("Max : %d\n", i);
        	if (commande >-1 && commande<i){
        		printf("\n");
        		char* fichiertcp,iptcp;
        		fichiertcp = listProp[commande].fichier;
        		iptcp = listProp[commande].ip;
        		break;
        	}
        	printf("Erreur dans la proposition\n");


        }

 		break;
		
  	}


  	printf("On fait la demande TCP avec %s et %s\n",fichier_envoi,ip_envoi);
  	//On fait la requete TCP
  	demandeTCPFichier(fichier_envoi, ip_envoi);

    return 0;

}





int publish (char *fichier, char* type_fichier, char* mots, char* ip_serveur){
	
	//On fait une concaténation pour calculer le hashcode après
	char concat[500];
	strcpy(concat, fichier);
	strcat(concat, type_fichier);
	strcat(concat, mots);
	strcat(concat,ip_serveur);
	
	/*Calcul du hashcode*/
	int i = 0;
	unsigned char temp[SHA_DIGEST_LENGTH];
	char buf[SHA_DIGEST_LENGTH*2];
	 
	 
	memset(buf, 0x0, SHA_DIGEST_LENGTH*2);
	memset(temp, 0x0, SHA_DIGEST_LENGTH);
	 
	unsigned char* test = SHA1((unsigned char *)concat, strlen(concat), temp);
	 
	for (i=0; i < SHA_DIGEST_LENGTH; i++) {
	    sprintf((char*)&(buf[i*2]), "%02x", temp[i]);
	}



	/*On prend l'adresse IP client*/
	char* IP = getmyIP();


    /*On créé la socket*/
	int socket_fd;
    if ( (socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }


    int ttl=1;
	if (setsockopt(socket_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) <0){
		perror ("setsockopt");
		exit (1);
	}



    /*Configuration de l'adresse d'envoie*/
    struct sockaddr_in  serv_addr;
    memset(&serv_addr,0, sizeof(serv_addr) );
  	serv_addr.sin_family = AF_INET;
  	serv_addr.sin_addr.s_addr = inet_addr((const char*)ip_serveur);
  	serv_addr.sin_port = htons(PORT);



  	int len;

    printf("%s\n",ip_serveur);

    printf("Début de la tranmission ...\n");
  	while(true){


  		char num[1] = "1";
  		sendto(socket_fd, num, sizeof(num),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

  		sleep(1);

  		//On envoie l'adresse IP
	    char ip[20];
	    strcpy(ip,IP);
	    sendto(socket_fd, ip, sizeof(ip),0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

	    //On envoie le type du fichier
	    char type[5];
	    strcpy(type,type_fichier);
	    sendto(socket_fd, type, sizeof(type), 0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

	    //On envoie les mots clé
	    char motclef[200];
	    strcpy(motclef,mots);
	    sendto(socket_fd, motclef, sizeof(motclef), 0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

	    //On envoie le nom du fichier
	    char dossier[100];
	    strcpy(dossier,fichier);
		sendto(socket_fd, dossier, sizeof(dossier), 0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

		//On envoie le hashcode
	    buf[41]='\0';
	    sendto(socket_fd, buf, sizeof(char)*41, 0,(const struct sockaddr *)&serv_addr, sizeof(serv_addr));

	    char ack[2];
    	int p5=recvfrom(socket_fd, ack, sizeof(ack), 0,( struct sockaddr *)&serv_addr,&len);


    	printf("ACK : %s\n",ack);

 		break;
		
  	}

 	close(socket_fd);

    return 0;

}




/*
Interface Client
*/
int clientfctn(){


	printf("  ____  _  _               _   \n");
	printf(" / ___|| |(_)  ___  _ __  | |_ \n");
	printf("| |    | || | / _ \\| '_ \\ | __|\n");
	printf("| |___ | || ||  __/| | | || |_ \n");
	printf(" \\____||_||_| \\___||_| |_| \\__|\n\n\n");


    printf("-----------Interface client-----------\n");
    printf("|Les choix possibles :               |\n");
    printf("|\'publish [nom du fichier]\' pour pu- |\n");
    printf("|blier un fichier.                   |\n");
    printf("|\'search [nom du fichier]' pour cher-|\n");
    printf("|cher un fichier                     |\n");
    printf("|'quit client' pour quitter          |\n");
    printf("|l\'interface client                  |\n");
    printf("--------------------------------------\n\n\n\n\n\n\n\n");

    char commande[200];
    char commande2[200];

    char type[15];
    char file[60];


/*
On enclenche la commande
*/
    pid_t pid=fork();

    if (pid){
    	//PARTIE ATTENTE D'UNE DEMANDE TCP (RECEPTION SIGNAL)

	    int sockfd;

	    struct sockaddr_in servaddr, cliaddr;
	      
	    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	        perror("socket creation failed");
	        exit(EXIT_FAILURE);
	    }
	      
	    memset(&servaddr, 0, sizeof(servaddr));
	    memset(&cliaddr, 0, sizeof(cliaddr));
	      
	    
	    servaddr.sin_family    = AF_INET; // IPv4
	    servaddr.sin_addr.s_addr = INADDR_ANY;
	    servaddr.sin_port = htons(PORT_WAIT_TCP);

	    
	    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
	            sizeof(servaddr)) < 0 )
	    {
	        perror("bind failed");
	        exit(EXIT_FAILURE);
	    }
	    
	    int len, n;
	  
	    len = sizeof(cliaddr);
	  


	    while(true){
	    	char num[2];
			recvfrom(sockfd, num, sizeof(num), 0,( struct sockaddr *)&cliaddr, &len);

	        printf("Receive\n");

	        printf("%d\n",num );


	        //if (strcmp(num,"1")){
	        	printf("Aquittement cote receveur\n");
	            char ack[2] = "0";
    			ack[2]= '\0';

    			//On envoie un aquittement
	            sendto(sockfd, ack, sizeof(ack),MSG_WAITALL,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));
	            

	            //On attend le nom du fichier
	            printf("Réception fichier\n");
	            char fichier[110];
	            int p2=recvfrom(sockfd, fichier, sizeof(fichier), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);
	            printf("Fichier : %s\n",fichier);
	            printf("Réception ip\n");
	            char ip[15];
	            int p10=recvfrom(sockfd, ip, sizeof(ip), MSG_WAITALL,( struct sockaddr *)&cliaddr, &len);

	            printf("\n\n\nfichier : %s, Ip : %s\n\n\n",fichier,ip );

	            FILE* file = fopen(fichier,"rb");
	            printf("%p\n",file );
	            if (file == NULL){
	            	char ackNF[3] = "-2";
    				ackNF[3]= '\0';
    				printf("F pas de fichier\n");
    				//On envoie un aquittement de fichier n'étant pas dans la base de donnée du client
	            	sendto(sockfd, ackNF, sizeof(ackNF),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));
	            
	            	sleep(1);
	            	exit(0);
	            }else{
	            	printf("RecevoirDemandeTCP\n");
	            	char ackNF[3] = "-1";
    				ackNF[3]= '\0';

    				//On envoie un aquittement de fichier étant dans la base de donnée du client
	            	//sendto(sockfd, ackNF, sizeof(ackNF),0,(const struct sockaddr *)&cliaddr, sizeof(cliaddr));
	            


	            	//On passe à la fonction de transfert de fichier TCP
	            	sleep(1);
	            	printf("RecevoirDemandeTCP\n");
	            	recevoirDemandeTCP(fichier,ip);
	            }


	        //}else{
	        //	printf("fat\n");
	            


    	//}

    	exit(0);
	        }
    }

    if(pid == 0){

    	/*PARTIE INTERFACE UTILISATEUR*/

	    for(;;){
	    	
	        printf("\nMy P2P> ");
	        scanf("%s %s",commande,commande2);

	        //On quitte le client dès lors que 'quit' est en commande
	        if (!strcmp(commande,"quit")){
	        	break;
	        }
	        printf("%s\n",commande);

	        

	/*
	On récupère les variables:
	- publish ou search si l'utilisateur veut publier ou récupérer un fichier
	- le fichier avec son format (exemple : test.txt)

	Exemple de commande :

	MyP2P > publish test.txt

	--> Dans ce cas, l'utilisateur veut "publier" le fichier 'test.txt'.
	*/
	        char* dossier;
	        char* typedos;
	        
	        char strSplit[200];

	        strcpy(strSplit, commande2);


	        dossier = strtok(strSplit,".");
		    typedos = strtok(NULL," ");

		    printf("type : %s, doss : %s, typedos : %s, dossier complet : %s",commande,dossier,typedos,commande2);
		
			//Dans le cas où l'utilisateur veut "publier" un fichier
		        if (!strcmp(commande,"publish")&&strcmp(dossier,"")&&strcmp(typedos,"")){
		            FILE *fileTest = fopen(commande2,"r");

		        //On regarde si le fichier existe
		            if (fileTest){
		            	fclose(fileTest);
		                
		                
		                char motclef[200] = "ppp";
		            /*
		            On demande les mots clés du fichier
		            On attend jusqu'à que l'utilisateur écrit au moins un mot clé.
		            */ 
		                while(true){

		                	printf("\nListe des mots clef (les séparés par une virgule) : ");
		                	scanf("%s",motclef);

		                	if(strcmp(motclef,"")){
		                		
		               
		                		
		                		break;
		                	}else{
		                		printf("---- Attention ! ----\nManque de mots clef\n");
		                	}
		                }

		                /*On demande l'adresse IP du serveur d'où l'on veut envoyer l'information*/
		                /*On attend jusqu'à ce l'on a une adresse IP valable*/
		                char ip_serveur[20];
		                char splitIP[20];
		                char* splitIP2;
		                int test; //Valeur de test

		                int number_of_digits;

		                while(true){
		                	test=0;
		                	number_of_digits =0;
		                	printf("Adresse IP du serveur : \n");
		                	scanf("%s",ip_serveur);
		                	strcpy(splitIP, ip_serveur);
		                	if(strcmp(splitIP,"")){

		                		strtok(splitIP,".");
		                		number_of_digits =1;

		                		//On compte les autres nombres de l'adresse IP donnée
			                	while((splitIP2 = strtok(NULL,"."))!=NULL){
			                		printf("%s\n",splitIP2);
			               			number_of_digits++;
			               		}

			                }

			                	if(test==0 && number_of_digits == 4){

			                		publish(dossier,typedos,motclef,ip_serveur);
			                		break;

			                	}else{
			                		printf("---- Attention ! ----\nIP non conforme\n");
			                	}
		                	}
		            }else{
		            	fclose(fileTest);
		                printf("---- Attention ! ----\n Fichier '%s' introuvé !\n",file);
		            }
		        }


				else if (!strcmp(commande,"search")&&strcmp(dossier,"")){
		            		              
		            /*
		            On demande les mots clés du fichier
		            On attend jusqu'à que l'utilisateur écrit au moins un mot clé.
		            */ 

		                /*On demande l'adresse IP du serveur d'où l'on veut envoyer l'information*/
		                /*On attend jusqu'à ce l'on a une adresse IP valable*/
		                char ip_serveur[20];
		                char splitIP[20];
		                char* splitIP2;
		                int test; //Valeur de test

		                int number_of_digits;

		                while(true){
		                	test=0;
		                	number_of_digits =0;
		                	printf("Adresse IP du serveur : ");
		                	scanf("%s",ip_serveur);
		                	strcpy(splitIP, ip_serveur);
		                	if(strcmp(splitIP,"")){

		                		strtok(splitIP,".");
		                		number_of_digits =1;

		                		//On compte les autres nombres de l'adresse IP donnée
			                	while((splitIP2 = strtok(NULL,"."))!=NULL){
			                		printf("%s\n",splitIP2);
			               			number_of_digits++;
			               		}

			                }

			                if(test==0 && number_of_digits == 4){

			                		search(dossier,ip_serveur);
			                		break;

			                }else{
			                		printf("---- Attention ! ----\nIP non conforme\n");
			                }
		                }
		            
		        }

	    }


	    return 0;

	}


}


