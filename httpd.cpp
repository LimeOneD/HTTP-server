#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LISTENADDR "0.0.0.0"

std::string error_message;
struct HTTPREQ
{
    char request[8];
    char URL[128];
};


struct File
{
    char filename[64];
    char *fc; // file content
    int size;
};


int sendfile(int client_socket, char* contenttype, struct File *file)
{
    if(file==NULL)
    {
        return 0;
    }
    char buffer[512];
    char *p;
    int n, x; /* n==remaining bytes x=bytes_sent */
    memset(buffer, 0, 512);
    
    snprintf(buffer, 511, 
            "Content-Type: %s\n"
            "Content-Length: %d\n\n",
            contenttype, file->size);
    if(write(client_socket, buffer, strlen(buffer))<0)
        return 0;

    
    n = file->size;
    p = file->fc;
    
    while(n>0)
    {
        int chunk_size = (n<512)? n:512;

        x = write(client_socket, p, chunk_size);
        if(x<1) return 0;
        n-=x;
        p+=x;

    }

    return 1;


}

struct File *readfile(char *filepath)
{
    char buffer[512];
    char *p;
    int n, x, fd;
    struct File *f;
    std::cout<<"read path is "<<filepath<< std::endl;

    fd = open(filepath, O_RDONLY);
    if(fd<0) return 0;

    if((f = (struct File*) malloc(sizeof(struct File)))==NULL)
    {
        close(fd);
        return 0;
    }
    strncpy(f->filename, filepath, 63);//copying the filename
    f->fc = (char*)malloc(512);
  
    x=0;

    while(1)
    {
        //memset(buffer, 0, 512);
        n = read(fd, buffer, 512);
        if(n==0)
            break;
        else if(n==-1)
        {
            close(fd);
            free(f->fc);
            free(f);
            return 0;
        }
        f->fc = (char *) realloc(f->fc, (n+x));
        memcpy((f->fc)+x , buffer , n);
        x += n;

    }
    f->size = x;
    close(fd);
    return f;
    
    
}


int server_init(int port_no)
{
    int opt =1;
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        fprintf(stderr, "reuse error");
        close(server_socket);
        return -1;
    }

    
    if(server_socket<0) 
    {
        error_message = "socket() error";
        return -1;
    }

   
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr =INADDR_ANY; // inet_addr(LISTENADDR);
    server_address.sin_port = htons(port_no);
    
   if( bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address))<0)
   {
       error_message = "bind() error";
       close(server_socket);
       return -1;
   }

   if(listen(server_socket, 5)<0)
   {
       close(server_socket);
       error_message = "listen() error";
       return -1;
   }
   std::cout<< "server socket is: "<< server_socket<<std::endl;
   std::cout<<"server address is: "<<  server_address.sin_addr.s_addr<<std::endl;
   return server_socket;

}


/*
int client_accept(int server_socket)
{
    int client_socket;
    socklen_t address_len;
    struct sockaddr_in client_address;

    memset(&client_address, 0, sizeof(client_address));

    if( (client_socket = accept(server_socket, (struct sockaddr *)&client_address, &address_len))<0)
    {
        error_message = "accept() error";
        return -1;
    }

    return client_socket;

}
*/


struct HTTPREQ* parse_http(char *str)
{
    char *p;
    struct HTTPREQ *httpreq = (struct HTTPREQ*)malloc(sizeof(struct HTTPREQ));



    for(p=str; p&&*p!=' '; p++); 
    if(*p==' ')
        *p=0;
    else
    {
        error_message = "parse error";
        free(httpreq);
        return 0;
    }

    strncpy(httpreq->request, str, 7);
   
    for(str=++p; p&&*p!=' '; p++); 
    if(*p==' ')
        *p=0;
    else
    {
        error_message = "parse error second part";
        free(httpreq);
        return 0;
    }

    strncpy(httpreq->URL, str, 127);

    return httpreq;

}



/*
char* client_read(int client_socket)
{
    char buffer[1024];
    memset(&buffer, 0, 1024);
    read(client_socket, buffer, 1024);
    if((read(client_socket, buffer, 1024))<0)
    {
        error_message = "read_error";
        return 0;
    
    write(1, buffer, 1024);
    std::cout<<"i am here\n";
    return NULL;//buffer;



}
*/




/*
void client_connect_handle(int server_socket, int client_socket)
{

    struct HTTPREQ *req; //= (struct HTTPREQ*) malloc ( sizeof(struct HTTPREQ));
    char *client_text;

    client_text = client_read(client_socket);
    std::cout<<"i am here";

    std::cout<<client_text<<std::endl;
    req = parse_http(client_text);

    if(!req)
    {
        close(client_socket);
        std::cout<<error_message;
        return;
    }
    printf("%s \n %s \n",req->request, req->URL);

    free(req);
    close(client_socket);
    


}

*/


void http_response(int client_socket,  const char *contenttype, const char *data)
{
    char buffer[512];
    int len;
    len=strlen(data);
    memset(buffer, 0, 512);
    snprintf(buffer, 511, 
            "Content-Type: %s\n"
            "Content-Length: %d\n"
            "\n%s\n",
            contenttype, len, data);
    len = strlen(buffer);
    write(client_socket, buffer, len);

}




void http_header(int client_socket, int code)
{
    char buffer[512]; 
    int length;
    memset(buffer, 0, 512);
    snprintf(buffer, 511, 
        "HTTP/1.0 %d OK\n"
        "Date: Fri, 30 Aug 2024 03:37:48 GMT\n"
        "Server: httpd.cpp\n"
        "Cache-Control: no-store, no-cache, max-age=0, private\n"
        "Content-Language: en\n"
        "Expires: -1\n"
        "X-Frame-Options: SAMEORIGIN\n", 
        code);
    length = strlen(buffer);
    write(client_socket, buffer, length);

}

int main(int argc, char** argv)
{   
    struct sockaddr_in client_address;
    socklen_t client_address_len=sizeof(client_address);
    memset(&client_address, 0, sizeof(client_address));
    char buffer[1024]; 
    int server_socket;
    int client_socket;
    char *port;
    std::string response_to_client, text;
    if(argc<2) 
    {
        fprintf(stderr, "Usage: %s <listening port> \n", argv[0]);
        return -1;
    }
    else
        port = argv[1];
            
    server_socket = server_init(atoi(port));
    if(server_socket <0)
    {
        fprintf(stderr, "%s\n", error_message);
        return -1;
    }
    std::cout << "listening on" << " " << LISTENADDR << " "<< port << "\n";
   


    /* while(1)
    {

        if( client_socket = client_accept(server_socket) <0)
        {
            fprintf(stderr, "%s \n", error_message);
            continue;
        }


        //if(!fork())
            client_connect_handle(server_socket, client_socket);
      

    }*/


    while(1)
    {
        if((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_len))<0)
        {
            fprintf(stderr, "accept error");
            return -1;
        }

        memset(&buffer, 0, 1024);
        if(!fork())
        {
            read(client_socket, buffer, 1024);
            
            char data[] = "httpd v1.0\n";
            write(client_socket, data, strlen(data));
            struct File *f;
            struct HTTPREQ *req;
            req = parse_http(buffer);
            
            if(!req)
            {
                close(client_socket);
                std::cout<<error_message;
                return -1;
            }
            char str[96];
            if(!strcmp(req->request, "GET") && !strncmp(req->URL, "/img/", 5))
            {
                //do something
                memset(str, 0, 96);
                snprintf(str,95, ".%s", req->URL);
                std::cout <<"requested url is: "<< req->URL << std::endl;
                f = readfile(str);
                if(f==NULL)
                {
                    response_to_client = "File not found";
                    text = "text/plain";
                    http_header(client_socket, 404);
                    http_response(client_socket,text.c_str(), response_to_client.c_str());


                }

                else
                {
                    http_header(client_socket, 200);
                    if(!sendfile(client_socket, "image/png", f))
                    {
                        //checking the return values in the sendfile function
                        response_to_client = "HTTP server error";
                        text = "text/plain";
                        http_header(client_socket, 500);
                        http_response(client_socket,text.c_str(), response_to_client.c_str());
                    }

                    
                }

            }
            
            if(!strcmp(req->request, "GET") && !strcmp(req->URL, "/app/webpage"))
            {
                response_to_client = "<http> <h2>hello my frient</h2><img src='/img/test2.png' /></http>";
                text = "text/html";
                http_header(client_socket, 200);
                http_response(client_socket, text.c_str(), response_to_client.c_str());
            }
            else
            {
                response_to_client = "File not found";
                text = "text/plain";
                http_header(client_socket, 404);
                http_response(client_socket,text.c_str(), response_to_client.c_str());
            }
            
            free(req);
            close(client_socket);
       }

    }

    return -1;
}






