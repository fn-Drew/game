#ifndef NETWORK_H
#define NETWORK_H

enum {
    KEY_COUNT = 6
};

typedef struct {
  int code;
  int axis;
  int direction;
  int boundary;
} Key;

typedef struct {
  Key keys[KEY_COUNT];
  bool keys_pressed[KEY_COUNT];
  Vector2 mouse_world;
} PlayerInput;

typedef struct {
  PlayerInput inputs_player[KEY_COUNT];
} GamePacket;

void NetworkInit(char *port_client, char *port_peer);

bool NetworkUpdate(GamePacket *receive_packet, GamePacket *send_packet);

void NetworkShutdown(void);

#endif