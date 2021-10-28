#include <stdio.h>
#include "client.h"
#include "serveur.h"

#include <stdlib.h>
//#include <stdio.h>
#include <string.h>


int main(){
	
	char* fichier = "test";
	char* type_fichier = "txt";
	char* mots = "coucou,bonjour";
	char* ip_serveur = "10.11.50.179";


	int i = publish (fichier,type_fichier,mots,ip_serveur);

	return i;

}