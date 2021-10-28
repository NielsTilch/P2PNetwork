#ifndef PROJET_RSA_TILCH_THOMAS_CLIENT_H
#define PROJET_RSA_TILCH_THOMAS_CLIENT_H

#include "common.h"

/*
Interface client
 */
int clientfctn();

int search (char *fichier, char* type_fichier, char* mots,char* ip_serveur);

int demandeTCPFichier (char* fichier, char* ip_send);

char* getmyIP();

int creationsocket();
/* 
Fonction publish 
*/
int publish (char *fichier, char* type_fichier, char* mots, char* ip_serveur);

int recevoirDemandeTCP(char* fichier, char* ip_send);



#endif //PROJET_RSA_TILCH_THOMAS_CLIENT_H
