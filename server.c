/*

  Simple server
  Issues: Can't figure out threads, uses process forking to make SOMETHING work.
  may need to compile with -lnsl and -lsocket flags depending on the machine
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>

int setup_first_socket(int* socket_fd, int* port, struct sockaddr_in* server, int* server_size){
  (*socket_fd) = socket(AF_INET, SOCK_STREAM, 0);

  (*server).sin_family=AF_INET;
  (*server).sin_addr.s_addr=INADDR_ANY;
  (*server).sin_port=htons(*port);

  (*server_size)=sizeof(*server);

  bind((*socket_fd),(struct sockaddr *)server,(*server_size));

  return 0;
}

char* request_item(char* buffer) {
  char* tok = malloc(256);
  char* docname=malloc(256);

  tok=strtok(buffer," ");
  while (tok != NULL) {
    if (strncmp(tok,"GET",3)==0) {
      tok = strtok(NULL," ");
      strcpy(docname,tok);
      break;
    }
    else {
      tok = strtok(NULL," ");
    }
  }
  return docname;
}


char*  is_slash(char* outbuf) {
  FILE* dump_ptr=fopen("templs","w+");
  system("ls > templs");

  char* body=malloc(2048);  
  strcpy(body,"");
  char* filename_buf=malloc(256);
  while(fgets(filename_buf, 256, dump_ptr) != 0){
    if (strstr(filename_buf,".htm") != NULL) {
      strcat(body,filename_buf);
    }
  }
  system("rm templs");

  strcat(outbuf,"HTTP/1.1 200 OK\n");
  strcat(outbuf,"Content-Type: text/plain\n");
  strcat(outbuf,"Content-Length: ");

  char* length_str=malloc(256);
  sprintf(length_str,"%d",sizeof(outbuf) + sizeof(body) + 64);

  strcat(outbuf,length_str);
  strcat(outbuf,"\n\n");
  strcat(outbuf,body);
  strcat(outbuf,"\n\n");
 
  return outbuf;
}


char* is_file(char* outbuf, char* request_name) {
  char* request2=malloc(256);

  int i;
  for (i=0;i<strlen(request_name);i++){
    request2[i]=request_name[i+1];
  }
  strcpy(request_name,request2);

  FILE* dump_ptr=fopen("templs","w+");
  system("ls > templs");

  int found=1;

  char* filename_buf=malloc(256);
  while(fgets(filename_buf, 256, dump_ptr) != 0){
    if (strstr(filename_buf,request_name) != NULL) {
      found = 0;
      break;
    }
  }
  system("rm templs");

  if (found==0){

    char* body=malloc(2048);
    char* linebuf=malloc(256);
    FILE* to_read=fopen(request_name,"r");
    
    while(fgets(linebuf,256,to_read)!= NULL){
      strcat(body,linebuf);
    }

    strcat(outbuf,"HTTP/1.1 200 OK\n");
    strcat(outbuf,"Content-Type: text/html\n");
    strcat(outbuf,"Content-Length: ");
    char* length_str=malloc(256);
    sprintf(length_str,"%d",sizeof(outbuf) + sizeof(body) + 64);
    strcat(outbuf,length_str);
    strcat(outbuf,"\n\n");
    strcat(outbuf,body);

  }
  else {
    strcat(outbuf,"HTTP/1.1 404 NOT FOUND\n");
    strcat(outbuf,"Content-Type: text/plain\n");
    strcat(outbuf,"Content-Length: ");
    char* length_str=malloc(256);
    sprintf(length_str,"%d",sizeof(outbuf)+ 64);
    strcat(outbuf,length_str);
    strcat(outbuf,"\n\n");
  }

  return outbuf;
}


int main(int argc, char *argv[])
{
  //file stuff
  FILE* log_ptr=fopen("weblog","a");


  int socket_fd=0;
  int port = atoi(argv[1]);
  struct sockaddr_in server;
  int server_size=0;
  setup_first_socket(&socket_fd, &port, &server, &server_size);


  //Okay, now we wait for the connection
  listen(socket_fd,5);
  int comm_fd;
  struct sockaddr_in from;
  int from_size=sizeof(from);

  while (1==1) {
    comm_fd=accept(socket_fd, (struct sockaddr *)&from, &from_size);
    int pid=fork();
    
    if (pid==0){
      
      //recieve bytes, then null terminate, then print to file
      char* buffer=malloc(1024);
      int bytes_received = recv(comm_fd,buffer,1023,0);
      buffer[bytes_received]='\0';
      fprintf(log_ptr,"%s\n%s\n\n",inet_ntoa((struct in_addr)from.sin_addr),buffer);
      
      char* request_name=malloc(256);
      strcpy(request_name,request_item(buffer));
      
      char* outbuf=malloc(2048);
      strcpy(outbuf,"");
      
      if (strncmp(request_name,"/",1) == 0 && strlen(request_name)==1){
	is_slash(outbuf);
      }
      else {
	is_file(outbuf,request_name);
      }

      bytes_received = send(comm_fd, outbuf, strlen(outbuf),0);
      close(comm_fd);
      exit(0);
    }
    else {
      close(comm_fd);
    }
  }
  return 0;
}

