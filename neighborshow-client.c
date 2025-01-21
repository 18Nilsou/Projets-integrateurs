#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define PORT 1339
#define MESSAGE "Hello, sous-réseau !"
#define BUFFER_SIZE 1024

// Fonction pour récupérer l'adresse de broadcast de l'interface réseau
void get_broadcast_address(const char *interface, char *broadcast_ip) {
    int sock;
    struct ifreq ifr;

    // Création du socket temporaire pour interroger l'interface réseau
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    // Copier le nom de l'interface réseau
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

    // Récupérer l'adresse de broadcast
    if (ioctl(sock, SIOCGIFBRDADDR, &ifr) < 0) {
        perror("Erreur ioctl");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Convertir en une chaîne lisible
    struct sockaddr_in *brd = (struct sockaddr_in *)&ifr.ifr_broadaddr;
    strcpy(broadcast_ip, inet_ntoa(brd->sin_addr));

    close(sock);
}

// Fonction pour envoyer un message UDP en broadcast
void send_udp_broadcast(const char *broadcast_ip) {
    int sock;
    struct sockaddr_in broadcastAddr;
    int broadcastEnable = 1;

    // Création du socket UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    // Activer l'option SO_BROADCAST
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        perror("Erreur setsockopt (SO_BROADCAST)");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse de broadcast
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(PORT);
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcast_ip);

    // Envoi du message
    if (sendto(sock, MESSAGE, strlen(MESSAGE), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr)) < 0) {
        perror("Erreur lors de l'envoi du message");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("✅ Message envoyé à %s:%d\n", broadcast_ip, PORT);

    close(sock);
}

// Fonction pour recevoir les réponses UDP
void receive_udp_response() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    socklen_t server_len = sizeof(server_addr);

    // Création du socket UDP pour recevoir les réponses
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Erreur socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur pour écouter sur le port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Lier le socket au port
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Attendre la réponse du serveur
    printf("🕐 Attente de la réponse...\n");
    ssize_t n = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &server_len);
    if (n < 0) {
        perror("Erreur lors de la réception de la réponse");
        close(sock);
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0';  // Ajouter un terminant null pour afficher la chaîne de caractères
    printf("✅ Réponse reçue : %s\n", buffer);

    close(sock);
}

int main() {
    char broadcast_ip[INET_ADDRSTRLEN];

    // Remplacez "eth0" par le nom de votre interface réseau (ex: "wlan0" pour Wi-Fi)
    get_broadcast_address("en0", broadcast_ip);
    printf("Adresse de broadcast détectée : %s\n", broadcast_ip);

    // Envoyer le message UDP en broadcast
    send_udp_broadcast(broadcast_ip);

    // Attendre et afficher la réponse
    receive_udp_response();

    return 0;
}
