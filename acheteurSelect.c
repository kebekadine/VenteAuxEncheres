/*COMPILATION*/
/* gcc -o acheteurselect acheteurselect.c 
/* ./acheteurselect portReceveur machineReceveur */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#define nbclient
int main(int argc, char **argv)
{
int sock, envoye, recu, recu2;
struct sockaddr_in adresseReceveur;
struct sockaddr_in adresseLocale;
char demande[100];
char confirm[100];
int lgadresseReceveur;
int lgadresseLocale;
struct hostent *hote;
char buf[256 ];
int unEntier = 1;
int prix, newprice, fd, etat;
int data=0;
fd_set lect;
fd =0;
int prixactuel=0;
/* creation de la socket */
if ((sock = socket(AF_INET, SOCK_DGRAM, 0 )) == -1) 
{
    perror("socket"); 
    exit(1);
}

/* recherche de l'@ IP de la machine distante */
if ((hote = gethostbyname(argv[2])) == NULL)
{
    perror("gethostbyname"); 
    close(sock); 
    exit(2);
}

/* pr'eparation de l'adresse distante : port + la premier @ IP */
adresseReceveur.sin_family = AF_INET;
adresseReceveur.sin_port = htons(atoi(argv[1]));
bcopy(hote->h_addr, &adresseReceveur.sin_addr, hote->h_length);
printf("L'adresse en notation pointee %s\n", inet_ntoa(adresseReceveur.sin_addr));

/////////////////////ENVOIE DE LA PARTICIPATION///////////////////////////////////

lgadresseReceveur = sizeof(adresseReceveur);
strcpy (demande, "puis je participer");
if ((envoye = sendto(sock, demande, strlen(demande)+1, 0, (struct sockaddr *) &adresseReceveur, lgadresseReceveur)) != strlen(demande)+1) 
{
  perror("sendto participation"); 
  close(sock); 
  exit(1);
}
printf("participation envoye\n");
     
//////////////////RECEPTION DE LA DESCRIPTION DU PRODUIT///////////////////////

    //lgadresseLocale = sizeof(struct sockaddr_in);
if ((recu = recvfrom(sock, demande, sizeof(demande) , 0, (struct sockaddr *) & adresseLocale, &lgadresseLocale )) == -1 ) {
  perror("recvfrom descriptionVente"); 
  exit(1);
}
printf("j'ai recu la description : %s\n", demande);

//////////////////////* reception de l'offre initiale */////////////////////
if ((recu = recvfrom(sock, &prix , sizeof(int) , 0, (struct sockaddr *) & adresseLocale, &lgadresseLocale )) != sizeof(int) )  
{
  perror("recvfrom offreInitiale"); 
  exit(1);
}
printf("j'ai recu le prix %d\n", prix);

printf("entré dans la boucle de vente");

struct timeval tv;
tv.tv_sec=500;
tv.tv_usec = 0;
prix= -1;

printf("%d", prix);

//////////////////////////BOUCLE DE VENTE////////////////////////////////
recu =-1;

printf("entrer si vous voulez quiter ou rester, oui=0, non=1\n");
while(recu !=0)
{
  
  FD_ZERO(&lect);
  FD_SET(sock, &lect);
  FD_SET(fd, &lect);

  int res=select(sock+1, &lect, NULL, NULL, 0);
  if (res==-1)
  {
    perror("select");
    exit(-1);
  }
  if(FD_ISSET(fd, &lect))
  {
    
    scanf("%d", &etat);
    //printf("jai recu %d\n", prix);

    //sile client veut quitter alors qu'il a la meilleur offre
    if (etat==1 && prix == prixactuel)
    {
      printf("vous ne pouvez pas quitter car vous avez le best prix\n");
      etat=-1;
    }

    //si le client poursuit la vente
    if (etat== 0){
      printf ("entrez votre prix\n");
      scanf("%d", &newprice);
      
      //mis a jour eventuel du prix actuelle
      if (newprice > prixactuel)
      {
        prixactuel= newprice;

    //envoi du prix au serveur  
        if ((envoye = sendto(sock, &newprice, sizeof(int), 0, (struct sockaddr *) &adresseLocale, lgadresseLocale)) != sizeof(int)) 
        {
          perror("sendto prix"); 
          close(sock); 
          exit(1);
        } 
        
      }
      //le client ne peut pas proposer un prix inferieur a l'ancien
      else 
      {
        printf("impossible de proposer un prix inferieur a l'ancien\n");
        //printf("entrer si vous voulez quiter ou rester");
        //scanf("%d", &etat);
        
      }
    
    }
    else if (etat== 1)
    {
      printf("vous  quittez la vente\n ");
      close (sock);
      exit(-1);
    }
    printf("entrer si vous voulez quiter ou rester, oui=0, non=1\n");    
  }

  //reception du meilleur prix retenu par le serveur
  if (FD_ISSET(sock, &lect))
  {
    lgadresseLocale = sizeof(adresseLocale);
    if ((recu = recvfrom(sock, &prix , sizeof(int) , 0, (struct sockaddr *) & adresseLocale, &lgadresseLocale )) ==-1 )  
    {
      perror("recvfrom prix enchere");
      exit(1);
    }
    printf("jai recu %d\n", prix);
  }
        
  
}
//RECEPTION DE LA DERNIERE OFFRE//
printf("reception de la fin: \n");
if ((recu2 = recvfrom(sock, &prix , sizeof(int) , 0, (struct sockaddr *) & adresseLocale, &lgadresseLocale )) != sizeof(int) )  
{
  perror("recvfrom prix fin"); 
  exit(1);
}
printf("le prix retenu %d\n", prix); 
//FIN DE LA VENTE ET RESULTAT

printf("fin de la vente, resultat:\n");
if (newprice == prix)
  printf("j'ai gagne\n");
else
  printf("j'ai perdu\n"); 

close(sock);
}
