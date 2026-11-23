#ifndef VEHICLE_PROTOCOL_H
#define VEHICLE_PROTOCOL_H

#define PORT 8080
#define SERVER_IP "127.0.0.1"

typedef struct {
    int speed;
    int rpm;
    float steering_angle;
} VehicleData;

#endif
