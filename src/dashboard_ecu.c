#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "vehicle_protocol.h"

void receive_udp() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(PORT) };
    bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    VehicleData data;
    printf("Dashboard ECU Listening on UDP port %d...\n", PORT);
    fflush(stdout);
    while(1) {
        recvfrom(sock, &data, sizeof(data), 0, NULL, NULL);
        printf("[DASHBOARD UDP] Speed: %d km/h | RPM: %d | Steering: %.1f deg\n", data.speed, data.rpm, data.steering_angle);
        fflush(stdout);
    }
    close(sock);
}

void receive_tcp() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = { .sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY, .sin_port = htons(PORT) };
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    printf("Dashboard ECU Listening on TCP port %d...\n", PORT);
    fflush(stdout);
    int new_socket = accept(server_fd, NULL, NULL);
    
    VehicleData data;
    while(recv(new_socket, &data, sizeof(data), 0) > 0) {
        printf("[DASHBOARD TCP] Speed: %d km/h | RPM: %d | Steering: %.1f deg\n", data.speed, data.rpm, data.steering_angle);
        fflush(stdout);
    }
    close(new_socket);
    close(server_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "tcp") != 0 && strcmp(argv[1], "udp") != 0)) {
        printf("Usage: %s <tcp|udp>\n", argv[0]);
        return 1;
    }
    if (strcmp(argv[1], "udp") == 0) receive_udp();
    else receive_tcp();
    return 0;
}
