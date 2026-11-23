#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

typedef struct {
    int speed;
    int rpm;
    float steering_angle;
} VehicleData;

void send_udp() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_port = htons(PORT) };
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    VehicleData data = {0, 800, 0.0};
    while(1) {
        data.speed = (data.speed + 1) % 120;
        data.rpm = 800 + (data.speed * 20);
        data.steering_angle = (rand() % 20) - 10.0;
        
        sendto(sock, &data, sizeof(data), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        printf("[UDP] Transmitted: Speed=%d km/h, RPM=%d, Steering=%.1f\n", data.speed, data.rpm, data.steering_angle);
        sleep(1);
    }
    close(sock);
}

void send_tcp() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = { .sin_family = AF_INET, .sin_port = htons(PORT) };
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("TCP Connection Failed (Dashboard ECU offline?)");
        exit(EXIT_FAILURE);
    }

    VehicleData data = {0, 800, 0.0};
    while(1) {
        data.speed = (data.speed + 1) % 120;
        data.rpm = 800 + (data.speed * 20);
        data.steering_angle = (rand() % 20) - 10.0;

        if (send(sock, &data, sizeof(data), 0) < 0) {
            perror("TCP Transmission Failed");
            break;
        }
        printf("[TCP] Transmitted: Speed=%d km/h, RPM=%d, Steering=%.1f\n", data.speed, data.rpm, data.steering_angle);
        sleep(1);
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "tcp") != 0 && strcmp(argv[1], "udp") != 0)) {
        printf("Usage: %s <tcp|udp>\n", argv[0]);
        return 1;
    }
    srand(time(NULL));
    if (strcmp(argv[1], "udp") == 0) send_udp();
    else send_tcp();
    return 0;
}
