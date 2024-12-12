#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>


#define SERVER_PORT 21

#define REPLY_LENGTH 500

#define START 0
#define READING 1
#define NEW_LINE 2
#define END_LINE 3
#define END 4

#define AUTH_READY 220 //220 server ready for user authentication
#define USERNAME_OK 331 //331 User name okay, need password.
#define LOGIN_OK 230 //230 User logged in, proceed.
#define PASV_ON "227" //227 Entering Passive Mode (h1,h2,h3,h4,p1,p2).

typedef struct {
    char* username;
    char* password;
    char* host;
    char* url_path;
    char* filename;
    char* ip;

} url_info;

int parse_url(char* url_str, url_info* url_info);

struct hostent* get_ip(char* host);

int authentication(char* username, char* password);

void write_to_server(char* cmd);

int read_from_server(char* buf);

int socket_connection(char *ip, int port);