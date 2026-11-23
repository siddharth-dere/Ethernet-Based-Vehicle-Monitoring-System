CC=gcc
CFLAGS=-Wall

all: vehicle_ecu dashboard_ecu

vehicle_ecu: vehicle_ecu.c
	$(CC) $(CFLAGS) -o vehicle_ecu vehicle_ecu.c

dashboard_ecu: dashboard_ecu.c
	$(CC) $(CFLAGS) -o dashboard_ecu dashboard_ecu.c

clean:
	rm -f vehicle_ecu dashboard_ecu
