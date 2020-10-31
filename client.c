/*
  Kimberly Low
  lowk2@rpi.edu
  Assignment 7
  Client

 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h> 
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int check_error(int argc, char* argv[]) {
  if (argv[1] == NULL){
    printf("You didn't provide a URL.\n");
    return 1;
  }
  if (argc < 2) {
    printf("Wrong argument number.\n");
    return 1;
  }
  if (argc > 4){
    printf("Wrong argument number.\n");
    return 1;
  }

  if (argv[2] != NULL && argv[3] != NULL) {
    int d;
    for(d=0; d<strlen(argv[3]); d++){
      if (isdigit(argv[3][d]) == 0) {
	printf("I don't understand that port number.\n");
	return 1;
      }
    }
  }
  return 0;
}

int server_stuff(int* sock_fd, struct sockaddr_in* server, int port, char* address,struct hostent** connect_to){
  *sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  (*server).sin_family = AF_INET;
  (*server).sin_port = htons(port);
  (*connect_to)=gethostbyname(address);
  if (connect_to == NULL) {
    printf("Sorry, can't find that host.\n");
    return 1;
  }
  bcopy((char *)(*connect_to)->h_addr, (char *)&(*server).sin_addr, (*connect_to)->h_length);


  return 0;
}

int main(int argc, char *argv[])
{
  if (check_error(argc,argv)==1) {
    return 1;
  }

  char* url=malloc(256);
  char* filename=malloc(256);
  int port;

  strcpy(url,argv[1]);
  if (argv[2] == NULL){
    strcpy(filename,"/");
    port =80;
  }
  else if (argv[2] != NULL && argv[3]==NULL) {
    strcpy(filename,argv[2]);
    port  =80;
  }
  else {
    strcpy(filename,argv[2]);
    port =atoi(argv[3]);
  }
  //at this point we should have our url, etc variables.

  int sock_fd;
  struct sockaddr_in server;
  struct hostent* connect_to;
  server_stuff(&sock_fd,&server,port,url,&connect_to);

  int connected=connect(sock_fd,(struct sockaddr *)&server, sizeof(server));
  if (connected ==0) {
    printf("Connecting.\n");
  }
  else {
    printf("Connect failed.\n");
    return 1;
  }

  //read and write
  char* get_req=malloc(1024);
  strcpy(get_req,"GET ");
  strcat(get_req,filename);
  strcat(get_req,"HTTP/1.1\n\n");
  write(sock_fd, get_req, strlen(get_req));

  char* rec_data=malloc(1024);
  read(sock_fd, rec_data, 1024);
  rec_data[1024]='\0';

  FILE* logging=fopen("webscript","w");

  fprintf(logging,"%s",rec_data);
  close(sock_fd);

  return 0;
}


