#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 1337
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    
    if (argc < 2) {
        printf("Usage: %s -a | -i ifname\n", argv[0]);
        return 1;
    }

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE] = {0};

    // Construction de la commande à envoyer au serveur
    if (strcmp(argv[1], "-i") == 0) {
        if (argc < 3) {
            printf("Erreur : Veuillez spécifier une interface après -i.\n");
            return 1;
        }
        snprintf(command, sizeof(command), "-i %s", argv[2]); // Construit "-i ifname"
    } else if (strcmp(argv[1], "-a") == 0) {
        strcpy(command, "-a");
    } else {
        printf("Commande inconnue.\n");
        return 1;
    }

    // Création du socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Erreur lors de la création du socket");
        return EXIT_FAILURE;
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connexion au serveur
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la connexion au serveur");
        return EXIT_FAILURE;
    }

    // Envoi de la commande
    send(client_socket, command, strlen(command) + 1, 0);

    // Réception de la réponse
    int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (received > 0) {
        buffer[received] = '\0';
        printf("%s\n", buffer);
    }

    close(client_socket);
    return 0;
}
