#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <cstring>

#include "server.h"


#define IP "0.0.0.0"
#define PORT 8181
int main(int argc, char* argv[])
{
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;


    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(server_address));
    
    server_init(server_socket, server_address, IP, PORT);
    
    while(1)
    {
      // accept();
       //thread();
    }
    return -1;
}
