/*
 * Copyright 1996-2021 Cyberbotics Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Description:  A simple client program to connect to the TCP/IP
 */

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <winsock.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

static void close_socket(int fd) {
#ifdef _WIN32
  closesocket(fd);
#else
  close(fd);
#endif
}

int main(int argc, char *argv[]) {
  struct sockaddr_in address;
  struct hostent *server;
  int fd, rc;
  char buffer[256];
  int port = 10003;
  char host[256];  // localhost

  sprintf(host, "127.0.0.1");
  if (argc > 1) {
    if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
      printf("Usage: client <IP:port> or <IP> or <port>\n");
      return 0;
    }
    const char *n = strchr(argv[1], ':');
    if (n > 0) {
      port = atoi(n + 1);
      strncpy(host, argv[1], sizeof(host) - 1);
      host[n - argv[1]] = '\0';
    } else if (strchr(argv[1], '.') || !isdigit(argv[1][0]))
      strncpy(host, argv[1], sizeof(host) - 1);
    else
      port = atoi(argv[1]);
  }
#ifdef _WIN32
  WSADATA info;
  rc = WSAStartup(MAKEWORD(2, 2), &info);  // Winsock 2.2
  if (rc != 0) {
    fprintf(stderr, "Cannot initialize Winsock\n");
    return 1;
  }
#endif
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    fprintf(stderr, "Cannot create socket\n");
    return 1;
  }
  memset(&address, 0, sizeof(struct sockaddr_in));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  server = gethostbyname(host);

  if (server)
    memcpy((char *)&address.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
  else {
    fprintf(stderr, "Cannot resolve server name: %s\n", host);
    close_socket(fd);
    return 1;
  }
  rc = connect(fd, (struct sockaddr *)&address, sizeof(struct sockaddr));
  if (rc == -1) {
    fprintf(stderr, "Cannot connect to %s:%d\n", host, port);
    close_socket(fd);
    return 1;
  }
  printf("Connected %s:%d\n", host, port);
  for (;;) {
    printf("Enter command: ");
    fflush(stdout);
    fgets(buffer, sizeof(buffer), stdin);
    if (strncmp(buffer, "exit", 4) == 0)
      break;
    int n = strlen(buffer);
    send(fd, buffer, n - 1, 0);
    n = recv(fd, buffer, 256, 0);
    buffer[n] = '\0';
    printf("Answer is: %s\n", buffer);
  }
  close_socket(fd);
  return 0;
}