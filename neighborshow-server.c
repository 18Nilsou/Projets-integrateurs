#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 1338  // Port d'écoute pour les requêtes UDP
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t client_len = sizeof(client_addr);
    char host_name[BUFFER_SIZE];
    
    // Obtenir le nom d'hôte de la machine
    if (gethostname(host_name, sizeof(host_name)) == -1) {
        perror("gethostname");
        exit(1);
    }

    // Créer un socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // Initialiser l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // Écoute sur toutes les interfaces
    server_addr.sin_port = htons(PORT);

    // Lier le socket à l'adresse du serveur
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    printf("Serveur UDP en écoute sur le port %d...\n", PORT);

    // Boucle pour recevoir des requêtes et répondre avec le nom d'hôte
    while (1) {
        // Réception des données
        ssize_t n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&client_addr, &client_len);
        if (n < 0) {
            perror("recvfrom");
            exit(1);
        }

        // Répondre avec le nom d'hôte
        printf("Requête reçue de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        sendto(sockfd, host_name, strlen(host_name), 0, (const struct sockaddr *)&client_addr, client_len);
    }

    close(sockfd);
    return 0;
}
