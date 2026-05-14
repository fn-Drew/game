#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <raylib.h>
#include "network.h"

static struct addrinfo *peerinfo;
static struct addrinfo *clientinfo;
static SOCKET s;

void NetworkInit(char *port_client, char *port_peer) {
  printf("network init\n");
  WSADATA wsaData;
  // Load winsock2 dll into memory, if unsuccesful, shut down
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    fprintf(stderr, "WSAStartup failed.\n");
    exit(1);
  }

  // Check for correct version 2.2, if incorrect, disable connections, shut down
  if(LOBYTE(wsaData.wVersion) !=2 || HIBYTE(wsaData.wVersion) != 2){
    fprintf(stderr, "Version 2.2 of Winsock not available.\n");
    WSACleanup();
    exit(2);
  }

  char *ip_client = "127.0.0.1";
  char *ip_peer= "127.0.0.1";
  int status;
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints); // make sure struct is empty
  hints.ai_family = AF_UNSPEC; // don't care if v4 or v6
  hints.ai_socktype = SOCK_DGRAM; // UDP 
  hints.ai_flags = 0; // remove passive flag

  // get information about (this address, this port or service, use this configuration (flags), put the info here)
  if ((status = getaddrinfo(ip_peer, port_peer, &hints, &peerinfo)) != 0 ){
    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
    exit(1);
  }

  printf("INIT peerinfo = %p\n", (void*)peerinfo);
  printf("IP resolved successfully\n");
  // servinfo now points to a non-empty linked list of addrinfos which contain a sockaddr

  // now client local port bind

  hints.ai_flags = AI_PASSIVE;  // fill in user IP
  // get information about (this address, this port or service, use this configuration (flags), put the info here)
  if ((status = getaddrinfo(ip_client, port_client, &hints, &clientinfo)) != 0 ){
    fprintf(stderr, "gai error: %s\n", gai_strerror(status));
    exit(1);
  }

  s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (s == INVALID_SOCKET) {
    fprintf(stderr, "socket error: %d\n", WSAGetLastError());
  }
  
  // servinfo now points to a non-empty linked list of addrinfos which contain a sockaddr

  if (bind(s, clientinfo->ai_addr, clientinfo->ai_addrlen) == SOCKET_ERROR) {
    fprintf(stderr, "bind error: %d\n", WSAGetLastError());
    closesocket(s);
    WSACleanup();
    exit(1);
  }

  // stops from waiting for packet to continue
  u_long mode = 1;
  ioctlsocket(s, FIONBIO, &mode);

  printf("INIT peerinfo = %p\n", (void*)peerinfo);
  printf("IP resolved successfully\n");

}

bool NetworkUpdate(GamePacket *receive_packet, GamePacket *send_packet) {
  int sent_bytes= sendto(
    s,
    (char *)send_packet,
    sizeof(*send_packet),
    0,
    (struct sockaddr*)peerinfo->ai_addr,
    (int)peerinfo->ai_addrlen
  );

  if (sent_bytes == SOCKET_ERROR){
    printf("Send error: %d \n", WSAGetLastError());
  } else {
    // printf("Sent: %d \n", sent_bytes);
  }

  int addr_len = sizeof(struct sockaddr_storage);
  struct sockaddr_storage peer_addr;

  // clear old packets
  GamePacket temp;
  bool got_packet = false;
  while (true) {
    int bytes = recvfrom(s, (char*)&temp, sizeof(temp), 0, (struct sockaddr*)&peer_addr, &addr_len);
    if (bytes == SOCKET_ERROR) break;  // WSAEWOULDBLOCK = buffer empty
    *receive_packet = temp;            // keep overwriting with newer data
    got_packet = true;
  }

  return got_packet;
}

void NetworkShutdown(void) {
  printf("network shutdown\n");
  freeaddrinfo(peerinfo); // free the linked list
  closesocket(s);
  WSACleanup();
}

