#include "download_app.h"

int sockfd;



int main(int argc, char **argv) {

    char reply[500];

    url_info url_info;
    struct sockaddr_in server_addr;

    if (argc != 2){
        printf("Provide URL please!\n");
        return -1;
    }

    parse_url(argv[1], &url_info);

    printf("Username: %s\n",url_info.username);
    printf("Password: %s\n",url_info.password);
    printf("Host: %s\n",url_info.host);
    printf("Filepath: %s\n",url_info.url_path);

    struct hostent* ip_address = get_ip(url_info.host);

    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) ip_address->h_addr)));

    // server address handling
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *) ip_address->h_addr)));    //32 bit Internet address network byte ordered
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

    printf("Successfully connected to server!\n");

    read_from_server(reply);
    read_from_server(reply);


    char* reply_code="";
    strncpy(reply,reply_code,3);

    if(strcmp(reply_code,AUTH_READY)){
        printf("Ready for authentication\n");
    }
    else{
        exit(-1);
    }


    if(login(url_info.username,url_info.password) != 0){
        exit(-1);
    }
    




    //enter passive mode and get data socket
    char pasv_cmd[7];
    sprintf(pasv_cmd,"pasv\r\n");
    write_to_server(pasv_cmd);

    read_from_server(reply);
    printf("Server reply to pasv:%s\n",reply);

    printf("PASV CMD: %s\n",pasv_cmd);

    reply_code = strndup(reply,3);



    if (strcmp(reply_code, PASV_ON)) {
        printf("Error entering passive mode\n");
        exit(-1);
    }

    printf("Entered passive mode\n");

    unsigned char data_socket_ip_list[6];
    unsigned char data_socket_port_list[2];

    char * pasv_tk = strtok(reply, "(");
    pasv_tk++;


    for (int i = 0; i< 5; i++){
        data_socket_ip_list[i] = atoi(pasv_tk);
        pasv_tk = strtok(NULL, ",");
    }

    pasv_tk++;

    for(int j = 0; j<2; j++){
        data_socket_port_list[j] = atoi(pasv_tk);
        pasv_tk = strtok(NULL, ",");
    }

    printf("DATA SOCKET IP 1: %i\n",data_socket_ip_list[0]);
    printf("DATA SOCKET PORT 1: %i\n",data_socket_port_list[0]);

    



    //int data_socket_ip[4];
    //int data_socket_port[2];

    char buf[1024];

    read_from_server(buf);

    printf("%s\n",buf);






    if (close(sockfd)<0) {
        perror("close()");
        exit(-1);
    }

    /** 
        

    
    
    // send a string to the server
    bytes = write(sockfd, buf, strlen(buf));
    if (bytes > 0)
        printf("Bytes escritos %ld\n", bytes);
    else {
        perror("write()");
        exit(-1);
    }

    
    */
    return 0;
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

    }

    return 0;   
}


struct hostent* get_ip(char* host) {
    struct hostent *h;

/**
 * The struct hostent (host entry) with its terms documented

    struct hostent {
        char *h_name;    // Official name of the host.
        char **h_aliases;    // A NULL-terminated array of alternate names for the host.
        int h_addrtype;    // The type of address being returned; usually AF_INET.
        int h_length;    // The length of the address in bytes.
        char **h_addr_list;    // A zero-terminated array of network addresses for the host.
        // Host addresses are in Network Byte Order.
    };

    #define h_addr h_addr_list[0]	The first address in h_addr_list.
*/
    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    return h;
}

int login(char* username, char* password){

    char reply[1024];

    char user_cmd[7+strlen(username)];
    sprintf(user_cmd, "USER %s\r\n", username);
    printf("Writing user cmd: %s\n",user_cmd);

    write_to_server(user_cmd);
    
    read_from_server(reply);
    
    char* reply_code = strndup(reply,3);
    printf("Reply: %s\n",reply);
    printf("Reply code: %s\n",reply_code);


    if (strcmp(reply_code, USERNAME_OK)!=0 && strstr(reply,USERNAME_OK)==NULL) {
        printf("User not accepted\n");
        exit(-1);
    }

    printf("Writing password: %li\n",7+strlen(password));

    
    
    char pass_cmd[7+strlen(password)];
    sprintf(pass_cmd, "pass %s\r\n", password);

    write_to_server(pass_cmd);
    read_from_server(reply);

    reply_code = strndup(reply,3);

    if (strcmp(reply_code, LOGIN_OK) != 0 ) {
        printf("Password not accepted\n");
        exit(-1);
    }

    free(reply_code);

    return 0;
}

void write_to_server(char* cmd){
    size_t bytes_written = write(sockfd, cmd, strlen(cmd));

    if (bytes_written > 0)
        printf("Written command is %ld bytes long\n", bytes_written);
    else {
        perror("write()");
        exit(-1);
    }

    usleep(10000);
    
}

void read_from_server(char* buf){
    usleep(10000);

    size_t bytes_read = read(sockfd,buf,1024);

    printf("Bytes read: %li\n",bytes_read);

    if (bytes_read <= 0) {
            printf("Read error!\n");
            exit(-1);
        }



    while(bytes_read == 1024){

        bytes_read = read(sockfd,buf,1024);
    
    }

    printf("AAAAAAAAAAAA\n");

}




