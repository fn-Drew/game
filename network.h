#ifndef NETWORK_H
#define NETWORK_H

enum {
    KEY_COUNT = 4
};

typedef struct {
  Vector2 position;
} GamePacket;

void NetworkInit(char *port_client, char *port_peer);

bool NetworkUpdate(GamePacket *receive_packet, GamePacket *send_packet);

void NetworkShutdown(void);

#endif