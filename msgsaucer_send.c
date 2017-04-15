#define PORTNO 2523
#define NICK_MAX_SIZE 256
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pwd.h>

void error (char* msg) {
  printf("Error: %s\n", msg);
  exit(1);
}

int main (int argc, char** argv) {
  if (argc < 3) {
    printf("Usage: %s <destination> <message>\n", argv[0]);
    exit(1);
  }

  int msglen = argc-3, i;
  for (i = 2; i < argc; i++) {
    msglen += strlen(argv[i]);
  }
  char msg[msglen+1];
  msg[0] = 0;
  for (i = 2; i < argc; i++) {
    strcat(msg, argv[i]);
    if (i < argc-1) strcat(msg, " ");
  }
  struct passwd* pw = getpwuid(getuid());
  char nickfile[strlen(pw->pw_dir)+10];
  strcpy(nickfile, pw->pw_dir);
  strcat(nickfile, "/.msgnick");
  FILE* nickfp = fopen(nickfile, "r");
  char nickbuff[NICK_MAX_SIZE];
  int nicklen;
  if (nickfp != 0) {
    fgets(nickbuff, NICK_MAX_SIZE-1, nickfp);
    nicklen = strlen(nickbuff);
    while (nickbuff[nicklen-1] == 0x0a || nickbuff[nicklen-1] == 0x20) nicklen--;
    nickbuff[nicklen] = 0;
    fclose(nickfp);
  } else {
    nicklen = 3;
    strcpy(nickbuff, "bob");
  }
  printf("Sending as %s\n", nickbuff);
 
  char buffer[msglen+nicklen+9];
  *((int*)buffer) = htonl(nicklen);
  strncpy(&buffer[4], nickbuff, nicklen);
  *((int*)&buffer[nicklen+4]) = htonl(msglen);
  strncpy(&buffer[nicklen+8], msg, msglen);
  buffer[msglen+nicklen+8] = 0;

  struct hostent* dest = gethostbyname(argv[1]);
  if (dest == 0) error("Could not reach destionation.");
  struct sockaddr_in dest_addr;
  dest_addr.sin_family = AF_INET;
  memcpy(&dest_addr.sin_addr.s_addr, dest->h_addr, dest->h_length);
  dest_addr.sin_port = htons(PORTNO);
  
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sendto(fd, buffer, msglen+nicklen+8, 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) error("Message could not be delivered.");
  close(fd);

  exit(0);
}
