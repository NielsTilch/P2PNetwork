#ifndef PROJET_RSA_TILCH_THOMAS_COMMON_H
#define PROJET_RSA_TILCH_THOMAS_COMMON_H
#include <openssl/sha.h>


/* Structure Data_t,
utilisée pour enregistrer et/ou trouver des données
*/
typedef struct data{
	int typeData;
    char* ip;
    char* dossier;
    char* type;
    char* hashcode;
    char* motclef;
}Data_t;

#endif