#define PORTNO 2523
#define BUFSIZE 256
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>

struct link {
  char* ip;
  char* msg;
  struct link* next;
};

struct link* root = 0;

void error (char* msg) {
  printf("%s\n", msg);
  exit(1);
}

void appendToList (char* ip, char* msg) {
  struct link* use = root;
  if (use == 0) {
    root = malloc(sizeof(struct link));
    use = root;
  } else {
    while (use->next == 0) use = use->next;
    use->next = malloc(sizeof(struct link));
    use = use->next;
  }
  use->ip = ip;
  use->msg = msg;
  use->next = 0;
}

void freeList () {
  while (root != 0) {
    struct link* old = root;
    root = root->next;
    free(old);
  }
}

void print_msg (char* ip, char* msg) {
  int nicklen = ntohl(*((int*)msg));
  if (nicklen < 0 || nicklen > 248) error("Buffer overflow averted.");
  char nick[nicklen+1];
  strncpy(nick, &msg[4], nicklen);
  nick[nicklen] = 0;
  int bodylen = ntohl(*((int*)&msg[4+nicklen]));
  if (nicklen+bodylen > 248 || bodylen < 0) error("Buffer overflow averted.");
  char body[bodylen+1];
  strncpy(body, &msg[nicklen+8], bodylen);

  FILE* fp = fopen("/dev/pts/1", "w");
  if (fp < 0) error("Error opening terminal for writing.");
  fprintf(fp, "\n*** MSGSaucer ***\n<%s [%s]> %s\n*** Done ***\n", nick, ip, body);
  fclose(fp);
}



void process_msg (char* ip, char* msg) {
  if (access("/dev/pts/1", F_OK) != -1)
    print_msg(ip, msg);
  else
    appendToList(ip, msg);
}

void* pestering (void* args) {
  while (1) {
    if (access("/dev/pts/1", F_OK) != -1) {
      struct link* tmp;
      while (root != 0) {
        print_msg(root->ip, root->msg);
        tmp = root;
        root = root->next;
        free(tmp);
      }
    }
  }
}

int main () {
  pthread_t tid;
  pthread_create(&tid, NULL, pestering, NULL);

  int sockfd;
  int clilen;
  struct sockaddr_in srv_addr;
  struct sockaddr_in cli_addr;
  struct hostent* hostp;
  int reusefast = 1;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) error("Failed to open socket.");
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reusefast, sizeof(int));
  memset((char*)&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  srv_addr.sin_port = htons(PORTNO);
  if (bind(sockfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0)
    error("Could not bind socket.");
  printf("Listener start/running.\n");
  daemon(0, 0);
  clilen = sizeof(cli_addr);
  char buffer[BUFSIZE];
  while (1) {
    memset(buffer, 0, BUFSIZE);
    if (recvfrom(sockfd, buffer, BUFSIZE, 0, (struct sockaddr*)&cli_addr, &clilen) < 0)
      error("Could not read data.");
    char ip[16];
    ip[15] = 0;
    if (inet_ntop(AF_INET, (const void*)&cli_addr.sin_addr, ip, 16) < 0)
      error("Could not read client address.");
    process_msg(ip, buffer);
  }
  pthread_join(tid, NULL);
  freeList();
  exit(0);
}
