#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_STR_SIZE 1024
#define MAX_CMD_SIZE 1032
#define SERVER_PORT 21

void parse_227_response(const char *line, char *ip_address, int *port) {
    int ip1, ip2, ip3, ip4, p1, p2;
    sscanf(line, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip1, &ip2, &ip3, &ip4, &p1, &p2);
    sprintf(ip_address, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    *port = (p1 * 256) + p2;
}

int read_response(int sockfd, char *ip_address, int *port) {
    char buffer[1024];
    int bytes_received;
    int response_code = 0;
    char code[4];
    int code_len = 2;

    // Loop to continuously read data from the socket
    while ((bytes_received = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received data
        printf("%s", buffer); // Print the received data

        for (size_t i = 0; i < bytes_received; i++) {
            if ((code_len == 0 && buffer[i] == '\r') || (code_len == 1 && buffer[i] == '\n')) {
                code_len++;
            }
            else if (code_len < 5 && isdigit(buffer[i])) {
                code[code_len - 2] = buffer[i];
                code_len++;
            }
            else if (code_len == 5 && buffer[i] == ' ') {
                //got the code
                response_code = ((code[0] - '0') * 100) + ((code[1] - '0') * 10) + (code[2] - '0');
                if (response_code == 227) {
                    parse_227_response(&buffer[i + 1], ip_address, port);
                }
                return response_code;
            }
            else code_len = 0;
        }
    }

    if (bytes_received < 0) {
        perror("recv()");
    }

    return response_code;
}

int main (int argc, char **argv) {
    if (argc == 1) {
        puts("Welcome to our FTP client. Please supply the url of type: \"ftp://[<user>:<password>@]<host>/<url-path>\".");
        return(0);
    }
    if (argc > 2) {
        puts("No need to supply more than 1 argument. Just the URL is fine :)");
        return(0);
    }

    char url[MAX_STR_SIZE];
    
    if (strlen(argv[1]) >= MAX_STR_SIZE) {
        perror("URL too large...\n");
        exit(1);
    }

    strcpy(url, argv[1]);

    char ftp[7];
    memcpy(ftp, &url, 6);
    ftp[6] = '\0';
    
    if (strcmp("ftp://", ftp) != 0) {
        puts("Beggining of url should be : ftp://");
        exit(1);
    }

    char tmp[MAX_STR_SIZE] = {0};
    char user[MAX_STR_SIZE] = {0};
    char passw[MAX_STR_SIZE] = {0};
    char host[MAX_STR_SIZE] = {0};
    char path[MAX_STR_SIZE] = {0};

    int counter = 6; 

    if (strchr(url, '@')) {
        while (url[counter] != ':') {
            if (counter >= strlen(url)) {
                puts("Bad url format: missing ':' between user and password.\nThe url must be ftp://[<user>:<password>@]<host>/<url-path>");
                return(0);
            }
            user[counter - 6] = url[counter];
            counter++;
        }
        user[counter - 6] = '\0';
        counter++;
        int pcounter = 0;
        while (url[counter] != '@') {
            passw[pcounter] = url[counter];
            counter++;
            pcounter++;
        }
        passw[pcounter] = '\0';
        counter++;
    } else {
        strcpy(user, "anonymous");
        strcpy(passw, "anonymous");
    }

    //WE HAVE USER AND PASSWORD

    printf("User: %s\n", (char *)&user);
    printf("Password: %s\n", (char *)&passw);

    int hcounter = 0;

    while (url[counter] != '/') {
        if (counter >= strlen(url)) {
            puts("Bad url format: missing '/' between host and url-path.\nThe url must be ftp://[<user>:<password>@]<host>/<url-path>");
            return(0);
        }
        host[hcounter] = url[counter];
        ++counter;
        ++hcounter;
    }

    ++counter;
    int pathcounter = 0;
    while (url[counter] != '\0') {
        path[pathcounter] = url[counter];
        ++pathcounter;
        ++counter;
    }

    printf("Host: %s\n", host);
    printf("Url-path: %s\n", path);

    struct hostent *h;

    if ((h=gethostbyname(host)) == NULL) {
        printf("Could not get host by name of %s", host);
        exit(1);
    }

    char *ip_address = inet_ntoa(*((struct in_addr *) h->h_addr));

    printf("Host Name : %s\n", h->h_name);
    printf("IP Address : %s\n", ip_address);
    
    int sockfd1;
    struct sockaddr_in server_addr;

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(SERVER_PORT);

    if ((sockfd1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    if (connect(sockfd1, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }

    char passive_ip[16];
    int passive_port;
    char buffer[1024];
    int bytes_received;
    int welcome_message_received = 0;

    int response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 220) {
        printf("Expected response code 220, but got %d\n", response_code);
        exit(-1);
    }

    //send a USER command with our username variable
    char cmd[MAX_CMD_SIZE];
    sprintf(cmd, "USER %s\r\n", user);
    int bytes = write(sockfd1, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("write()");
        exit(-1);
    }

    printf("Wrote: %s\n", cmd);

    response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 331) {
        printf("Expected response code 331, but got %d\n", response_code);
        exit(-1);
    }

    //send a PASS command with our password variable
    sprintf(cmd, "PASS %s\r\n", passw);
    bytes = write(sockfd1, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("write()");
        exit(-1);
    }

    printf("Wrote: %s\n", cmd);

    response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 230) {
        printf("Expected response code 230, but got %d\n", response_code);
        exit(-1);
    }

    sprintf(cmd, "PASV\r\n");
    bytes = write(sockfd1, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("write()");
        exit(-1);
    }

    printf("Wrote: %s\n", cmd);

    response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 227) {
        printf("Expected response code 227, but got %d\n", response_code);
        exit(-1);
    }

    printf("Passive IP: %s\n", passive_ip);
    printf("Passive Port: %d\n", passive_port);

    struct sockaddr_in server_addr2;
    int sockfd2;

    bzero((char *) &server_addr2, sizeof(server_addr2));
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_addr.s_addr = inet_addr(passive_ip);
    server_addr2.sin_port = htons(passive_port);

    if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }

    if (connect(sockfd2, (struct sockaddr *) &server_addr2, sizeof(server_addr2)) < 0) {
        perror("connect()");
        exit(-1);
    }

    sprintf(cmd, "RETR %s\r\n", path);
    bytes = write(sockfd1, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("write()");
        exit(-1);
    }

    printf("Wrote: %s\n", cmd);

    response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 150 && response_code != 125) {
        printf("Expected response code 150, but got %d\nPossibly the <url-path> doesn't exist.\n", response_code);
        exit(-1);
    }

    // Extract the filename from the path
    char *filename = strrchr(path, '/');
    if (filename != NULL) {
        filename++; // Move past the '/'
    } else {
        filename = path; // No '/' found, use the whole path as the filename
    }

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen()");
        exit(-1);
    }

    while ((bytes_received = recv(sockfd2, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        fwrite(buffer, 1, bytes_received, file);
    }

    if (bytes_received < 0) {
        perror("recv()");
    }

    if (close(sockfd2) < 0) {
        perror("close()");
        exit(-1);
    }

    fclose(file);

    response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 226) {
        printf("Expected response code 226, but got %d\n", response_code);
        exit(-1);
    }

    sprintf(cmd, "QUIT\r\n");
    bytes = write(sockfd1, cmd, strlen(cmd));
    if (bytes < 0) {
        perror("write()");
        exit(-1);
    }
    
    printf("Wrote: %s\n", cmd);

    response_code = read_response(sockfd1, passive_ip, &passive_port);
    if (response_code != 221) {
        printf("Expected response code 221, but got %d\n", response_code);
        exit(-1);
    }

    printf("\nSuccessfully downloaded %s and closed control connection.\n", filename);

    if (close(sockfd1) < 0) {
        perror("close()");
        exit(-1);
    }

    return(1);
}