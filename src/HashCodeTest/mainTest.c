#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"serveur.h"



int main() {
    
    char* ligne1 = "1700&fichier1&txt&3c4fg5&motcle1";
    char* ligne2 = "123.98.00.01&fichier second&jpg&2e5a6c7b9d&motcle1 motcle2";

    char* resultat1 = extraireIpEtNom(ligne1);
    char* resultat2 = extraireIpEtNom(ligne2);

    printf("%s", resultat1);
    printf("%s", resultat2);

    return 0;
}