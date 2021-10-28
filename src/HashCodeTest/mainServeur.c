#include <stdio.h>
#include "client.h"
#include "serveur.h"

#include <stdlib.h>
#include <string.h>


int main() {

    char* fichier1 = "test";
	char* type_fichier1 = "txt";
	char* mots1 = "coucou,bonjour";
	char* ip_serveur1 = "10.11.48.185";

    char* fichier2 = "test2";
    char* type_fichier2 = "csv";
    char* mots2 = "test,txt,fichier";
    char* ip_serveur2 = "NULL";

    int a1 = publish_ack(fichier1, type_fichier1, mots1);
    int a2 = publish_ack(fichier2, type_fichier2, mots2);

    /* Le test 1 devrait donner quelque chose de correct,
    le test 2 devrait renvoyer une erreur à cause de l'adresse IP nulle.
    */

   Data_t* dataTest = (Data_t*)malloc(sizeof(Data_t*));
   dataTest->dossier = fichier1;
   dataTest->ip = ip_serveur1;
   dataTest->motclef = mots1;
   dataTest->type = type_fichier1;

   publish_response(dataTest);
   if (!publish_response(dataTest)) {
       printf("Erreur sur publish_response\n");
       return 1;
   }

   Data_t* dataBack = search_response(dataTest);
   if (!dataBack) {
       printf("Erreur sur search_response\n");
       return 1;
   }

   return 0;

}