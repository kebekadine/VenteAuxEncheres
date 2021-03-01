/* Lancement d'un serveur :  ./comissaires port */
/* compilation : gcc -o comissaires comissaireSelect.c   */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <arpa/inet.h>
#define nbclient_max 3


struct sockaddr_in adresseEmetteur[nbclient_max];
void arretParControlC(int sig) 
{
printf("terminaison par un Control-C\n");
fflush(stdout);
}

int main (int argc, char **argv)
{
int sockAccueil,sockVente, recu,envoye;
char demande[100], nomh[50];
char desc[200];
int prix;
struct sockaddr_in adresseLocale, adClient;
struct sockaddr_in adresseEmetteur[nbclient_max];
int lgadresseEmetteur, lgadresseLocale, lgadclient;
struct hostent *hote;
struct sigaction action;
int val=0;
int  i,prixI;
int nbclient =0;
int data=0;
int prixrecu;
fd_set lect;
int fd =0;
struct timeval time;
/* handler de signal SIGINT */
action.sa_handler = arretParControlC;
sigaction(SIGINT, &action, NULL);

/* creation de la socket */
if ((sockAccueil = socket(AF_INET, SOCK_DGRAM, 0 )) == -1) 
	{
  	perror("socket"); 
  	exit(1);
  	}
adresseLocale.sin_family = AF_INET;
adresseLocale.sin_port = htons(atoi(argv[1]));
adresseLocale.sin_addr.s_addr = htonl(INADDR_ANY);

/* attachement de la socket a` l'adresse locale */
lgadresseLocale = sizeof(adresseLocale);
if ((bind(sockAccueil, (struct sockaddr *) &adresseLocale,lgadresseLocale)) == -1) 
{
  	perror("bind"); 
  	exit(1);
}

///creation de la socket de vente////////
if ((sockVente = socket(AF_INET, SOCK_DGRAM, 0 )) == -1) 
{
  	perror("socket"); 
  	exit(1);
}
/* récupération du nom de la machine présente */
    if (gethostname(nomh, 50) == -1) {
        perror("gethostbyname");
        exit(1);
    }
    printf("Je m'execute sur %s\n", nomh);

///////////////////////////////AVANT VENTE//////////////////////////////////////
for (int i=0; i <nbclient; i++)
{
    strcpy (desc, "ce ci est un bon produit");
    prix= 100;
    if ((envoye = sendto(sockVente, desc, strlen(desc)+1, 0, (struct sockaddr *) &adresseEmetteur[i], lgadresseEmetteur))!= strlen(desc)+1) 
	    {
	    perror("send description"); 
	    close(sockAccueil); 
	    exit(1);
	    }
	printf("description envoye : %s\n", desc);
	if ((envoye = sendto(sockVente, &prix, sizeof(int), 0, (struct sockaddr *) &adresseEmetteur[i], lgadresseEmetteur))!= sizeof(int)) 
	    {
	    perror("send prix"); 
	    close(sockAccueil); 
	    exit(1);
	    }
	printf("prix envoye %d\n", prix);
}

/* 3 boucle de vente */

val =0;
prix= 100;
//printf("voulez vous continuez ? oui=0, non=1");
while ((val == 0) )
{
    FD_ZERO(&lect);
  	FD_SET(sockAccueil, &lect);
  	FD_SET(fd, &lect);
  	FD_SET(sockVente, &lect);
  	time.tv_sec = 30;
	time.tv_usec = 0;

  	int res=select(sockVente+1, &lect, NULL, NULL, &time);
  	if (res==-1)
  	{
    	perror("select");
    	exit(-1);
  	}

  	
  	if (FD_ISSET(sockAccueil, &lect)){
  		//teste si le serveur peut recevoir des clients
  		if((nbclient< nbclient_max) ){
			lgadresseEmetteur= sizeof(struct sockaddr_in);
	    	if ((recu = recvfrom(sockAccueil, demande, 100, 0, (struct sockaddr *) &adresseEmetteur[nbclient], &lgadresseEmetteur))==-1) 
		    {
			    perror("recvfrom unEntier"); 
			    close(sockAccueil); 
			    exit(1);
		    }
			strcpy (desc, "bague magique");
	    	
	    	if ((envoye = sendto(sockVente, desc, strlen(desc)+1, 0, (struct sockaddr *) &adresseEmetteur[nbclient], lgadresseEmetteur))!= strlen(desc)+1) 
		    {
			    perror("send description"); 
			    close(sockAccueil); 
			    exit(1);
		    }
			printf("description envoye : %s\n", desc);
			//envoi du prix intial si client arrive encours il recoit le max
			if ((envoye = sendto(sockVente, &prix, sizeof(int), 0, (struct sockaddr *) &adresseEmetteur[nbclient], lgadresseEmetteur))!= sizeof(int)) 
		    {
			    perror("send prix"); 
			    close(sockAccueil); 
			    exit(1);
		    }
			printf("prix envoye %d\n", prix);
		
		}
		nbclient ++;
  	}

  	
  	else if (FD_ISSET(sockVente, &lect))
  	{
  		//reeception du prix des clients
  		lgadclient= sizeof(struct sockaddr_in);
  		if ((recu = recvfrom(sockVente, &prixrecu, sizeof(int), 0, (struct sockaddr *) &adClient, &lgadclient))==-1) 
	    {
		    perror("recvfrom prix"); 
		    close(sockAccueil); 
		    exit(1);
	    }
		printf("j'ai recu %d\n", prixrecu);

		//envoi du best prix aux clients
		if (prixrecu > prix )
		    prix = prixrecu;

		if ((envoye = sendto(sockVente, &prix, sizeof(int), 0, (struct sockaddr *) &adClient, lgadclient))!= sizeof(int)) 
		    {
		    perror("send prix"); 
		    close(sockAccueil); 
		    exit(1);
		    }
		printf("j'ai envoye %d\n", prix);


		printf("voulez vous continuez ? 0= oui , 1= non\n");
	    scanf("%d", &val);
	    

	}
	else
	{
		printf("le temps d'attente  est atteint desole\n");
		break;
	}
}


//si on arrete la vente on envoie 0//
for (i =0; i<nbclient; i++){
	        if ((envoye = sendto(sockVente, &data, 0, 0, (struct sockaddr *) &adresseEmetteur[i], lgadresseEmetteur))!= 0) 
		    {
		    perror("send prix"); 
		    close(sockVente); 
		    exit(1);
		    }
		}
printf("datagrame vide envoye %d\n", data);

printf("envoie de la derniere offre\n");
for (i =0; i < nbclient; i++){
 	if ((envoye = sendto(sockVente, &prix, sizeof(int), 0, (struct sockaddr *) &adresseEmetteur[i], lgadresseEmetteur))!= sizeof(int)) 
	    {
	    perror("send prix"); 
	    close(sockAccueil); 
	    exit(1);
	    } 
}

if (prixI== prix)
{
	printf("pas vendu car aucun acheteur\n");


	printf("prix retenux %d\n", prixI);
}
else
	printf("prix retenux %d\n", prix);
close(sockAccueil);
}
