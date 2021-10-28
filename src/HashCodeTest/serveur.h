#ifndef PROJET_RSA_TILCH_THOMAS_SERVEUR_H
#define PROJET_RSA_TILCH_THOMAS_SERVEUR_H

#include "common.h"

/*
Interface client
 */
int serveurfctn();


int creationsocketServeur();
/* 
Fonction publish 
*/
int publish_ack (int socket, struct sockaddr_in cliaddr);

// Fonction pour enregistrer un document dans la base de données
void publish_response(int sockfd, struct sockaddr_in cliaddr);

// Fonction pour chercher un document parmi ceux déjà enregistrés
Data_t* search_response(Data_t* data);

// Fonction pour extraire IP et nom du fichier
char* extraireIpEtNom(char* ligne);

#endif //PROJET_RSA_TILCH_THOMAS_SERVEUR_H