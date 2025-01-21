#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>

#define PORT 1337
#define BUFFER_SIZE 1024

void list_interfaces(char *output, size_t size) {
    struct ifaddrs *ifaddr, *ifa;
    char buffer[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        snprintf(output, size, "Erreur lors de la récupération des interfaces.\n");
        return;
    }

    output[0] = '\0'; // Nettoyage de la sortie
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        void *addr;
        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) { // IPv4
            addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
        } else if (family == AF_INET6) { // IPv6
            addr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
        } else {
            continue;
        }

        inet_ntop(family, addr, buffer, sizeof(buffer));
        snprintf(output + strlen(output), size - strlen(output), "%s: %s\n", ifa->ifa_name, buffer);
    }

    freeifaddrs(ifaddr);
}

void get_interface_info(const char *ifname, char *output, size_t size) {
    struct ifaddrs *ifaddr, *ifa;
    
    char buffer[INET6_ADDRSTRLEN];
    char mask_buffer[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        snprintf(output, size, "Erreur lors de la récupération des informations.\n");
        return;
    }

    output[0] = '\0'; // Nettoyage de la sortie
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || strcmp(ifa->ifa_name, ifname) != 0) continue;

        void *addr, *mask;
        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) { // IPv4
            addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            mask = &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr;
        } else if (family == AF_INET6) { // IPv6
            addr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            mask = &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr;
        } else {
            continue;
        }

        inet_ntop(family, addr, buffer, sizeof(buffer));
        inet_ntop(family, mask, mask_buffer, sizeof(mask_buffer));

        snprintf(output + strlen(output), size - strlen(output), "%s: %s/%s\n", ifa->ifa_name, buffer, mask_buffer);
    }

    if (output[0] == '\0') {
        snprintf(output, size, "Interface %s non trouvée.\n", ifname);
    }

    freeifaddrs(ifaddr);
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (received <= 0) {
        printf("Client déconnecté\n");
        close(client_socket);
        return;
    }
    buffer[received] = '\0';

    printf("Commande reçue : %s\n", buffer);

    char response[BUFFER_SIZE] = {0};
    if (strcmp(buffer, "-a") == 0) {
        list_interfaces(response, BUFFER_SIZE);
    } else if (strncmp(buffer, "-i ", 3) == 0) {
        get_interface_info(buffer + 3, response, BUFFER_SIZE);
    } else {
        snprintf(response, BUFFER_SIZE, "Commande inconnue.\n");
    }

    send(client_socket, response, strlen(response) + 1, 0);
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Erreur lors de la création du socket");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la liaison du socket");
        return EXIT_FAILURE;
    }

    if (listen(server_socket, 5) < 0) {
        perror("Erreur lors de l'écoute");
        return EXIT_FAILURE;
    }

    printf("Serveur en écoute sur le port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Erreur lors de l'acceptation du client");
            continue;
        }

        handle_client(client_socket);
    }

    close(server_socket);
    return 0;
}
