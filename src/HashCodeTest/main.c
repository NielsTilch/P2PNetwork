#include <stdio.h>
#include "client.h"
#include "serveur.h"

#include <stdlib.h>
//#include <stdio.h>
#include <string.h>

int main(){
printf(" __  __       ____ ____  ____  \n");
printf("|  \\/  |_   _|  _ \\___ \\|  _ \\ \n");
printf("| |\\/| | | | | |_) |__) | |_) |\n");
printf("| |  | | |_| |  __// __/|  __/ \n");
printf("|_|  |_|\\__, |_|  |_____|_|    \n");
printf("        |___/                  \n\n\n");

                                                        
	printf("-----------Interface myP2P-----------\n");
    printf("|'Client' pour etre un client       |\n");
    printf("|'Serveur' pour etre un serveur     |\n");
    printf("-------------------------------------\n\n\n\n");
    char commande[200];




/*
On enclenche la commande
*/

    for(;;){
    	
        printf("\nMy P2P> ");
        scanf("%s",commande);
        if (!strcmp(commande,"quit")){
        	break;
        }
        else if (!strcmp(commande,"Client") || !strcmp(commande,"client")){
        	clientfctn();
        	printf("\n\n\n\n\n\n");
        	printf(" __  __       ____ ____  ____  \n");
			printf("|  \\/  |_   _|  _ \\___ \\|  _ \\ \n");
			printf("| |\\/| | | | | |_) |__) | |_) |\n");
			printf("| |  | | |_| |  __// __/|  __/ \n");
			printf("|_|  |_|\\__, |_|  |_____|_|    \n");
			printf("        |___/                  \n\n\n");
			printf("-----------Interface myP2P-----------\n");
		    printf("|'Client' pour etre un client       |\n");
		    printf("|'Serveur' pour etre un serveur     |\n");
		    printf("-------------------------------------\n\n\n\n\n\n\n\n");
        }
        else if(!strcmp(commande,"Serveur") || !strcmp(commande,"serveur")){
        	serveurfctn();
        	printf("\n\n\n\n\n\n");
        	printf(" __  __       ____ ____  ____  \n");
			printf("|  \\/  |_   _|  _ \\___ \\|  _ \\ \n");
			printf("| |\\/| | | | | |_) |__) | |_) |\n");
			printf("| |  | | |_| |  __// __/|  __/ \n");
			printf("|_|  |_|\\__, |_|  |_____|_|    \n");
			printf("        |___/                  \n\n\n");
			printf("-----------Interface myP2P-----------\n");
		    printf("|'Client' pour etre un client       |\n");
		    printf("|'Serveur' pour etre un serveur     |\n");
		    printf("-------------------------------------\n\n\n\n\n\n\n\n");
        }
        else{
        	printf("\nCommande invalide :( \n\n");
        }
    }


    return 0;
}