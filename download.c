#include "download.h"

int socket1;

int main(int argc, char *argv[]){
    if (argc != 2){
        printf("Usage: ./download ftp://[<user>:<password>@]<host>/<url>\n");
        exit(-1);
    }

    url_info url_info;



    if(parse_url(argv[1], &url_info) != 0){
        printf("Error parsing URL, correct format: ftp://[<user>:<password>@]<host>/<url>");
    }

    printf("Username: %s\n",url_info.username);
    printf("Password: %s\n",url_info.password);
    printf("Host: %s\n",url_info.host);
    printf("Filepath: %s\n",url_info.url_path);
    printf("IP: %s\n",url_info.ip);


    char reply[REPLY_LENGTH];
    socket1 = socket_connection(url_info.ip, SERVER_PORT);
    if(socket1 == -1){
        printf("Failed connection to socket\n");
        exit(-1);
    }

    int code = read_from_server(reply);

    printf("Code: %d\n%s", code, reply);


    return 0;
}

struct hostent* get_ip(char* host) {
    struct hostent *h;

    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    return h;
}

int parse_url(char* url_str, url_info* url_info){
    char* username;
    char* password;
    char* host;
    char* path;



    if (strstr(url_str, "ftp://") != NULL){ //look for initial part of url (ftp://), if it's there skip it, else throw error
         url_str+=6;
    }
    else {
        printf("This is not a ftp url!\n");
        return -1;
    }


    if(strstr(url_str, "@") != NULL){ //url with user and password: [<user>:<password>@]<host>/<url-path>
        username = strtok(url_str,":");

        if(username == NULL){
            printf("Error parsing username!\n");
            return -1;
        }

        url_info->username = strdup(username);

        password = strtok(NULL, "@");

        if(password == NULL){
            printf("Error parsing password!\n");
            return -1;
        }

        url_info->password = strdup(password);

        host = strtok(NULL, "/");

        if(host == NULL){
            printf("Error parsing host!\n");
            return -1;
        }

        url_info->host = strdup(host);

        path = strtok(NULL, "");

        if(path == NULL){
            printf("Error parsing path!\n");
            return -1;
        }

        url_info->url_path = strdup(path);

        printf("%s\n",url_info->url_path);

        struct hostent* ip_address = get_ip(url_info->host);

        url_info->ip = inet_ntoa(*((struct in_addr *) ip_address->h_addr));


    }


    else { //anonymous user: ftp://<host>/<url-path>

        url_info->username = strdup("anonymous");
        url_info->password = strdup("anonymous");

        host = strtok(url_str, "/");

        if(host == NULL){
            printf("Error parsing host!\n");
            return -1;
        }

        url_info->host = strdup(host);

        path = strtok(NULL, "");

        if(path == NULL){
            printf("Error parsing path!\n");
            return -1;
        }

        url_info->url_path = strdup(path);

        struct hostent* ip_address = get_ip(url_info->host);

        url_info->ip = inet_ntoa(*((struct in_addr *) ip_address->h_addr));

    }

    return 0;   
}

int socket_connection(char *ip, int port){
    int sockfd;
    struct sockaddr_in server_addr;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);    //32 bit Internet address network byte ordered
    server_addr.sin_port = htons(SERVER_PORT);        //server TCP port must be network byte ordered

    // open a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    // connect to the server
    if (connect(sockfd,
                (struct sockaddr *) &server_addr,
                sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    printf("Connection successful!\n");

    return sockfd;

}

int read_from_server(char* buf) {


    char* code_str;
    int code = 0;
    int state = START;
    char line_header[4];
    char curr_byte;

    
    int buf_i = 0;
    while (state != END) {
        switch (state)
        {
        case START:
            read(socket1, line_header, 4);
            code_str = strndup(line_header, 3);
            state = READING;

            if (line_header[3] == ' ') {
                state = END_LINE;
            }
            break;
        
        case READING:
            read(socket1, &curr_byte, 1);

            buf[buf_i] = curr_byte;
            buf_i++;

            if (curr_byte == '\n') {
                state = NEW_LINE;
            }
            break;

        case NEW_LINE:
            read(socket1, line_header, 4);

            if (line_header[3] == ' ') {
                state = END_LINE;
                break;
            }
            state = READING;
            break;

        case END_LINE:
            read(socket1, &curr_byte, 1);


            buf[buf_i] = curr_byte;
            
            buf_i++;
            

            if (buf[buf_i-1] == '\n') {
                state = END;
            }
            break;

        default:
            break;
        }
    }
    

    sscanf(code_str, "%d", &code);
    return code;

}


