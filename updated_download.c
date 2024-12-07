
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_STR_SIZE 256

void parse_url(const char *url, char *user, char *password, char *host, char *path) {
    // Parse the FTP URL into user, password, host, and file path components
    // Format: ftp://[<user>:<password>@]<host>/<url-path>
    if (sscanf(url, "ftp://%99[^:]:%99[^@]@%99[^/]/%199[^
]",
               user, password, host, path) == 4) {
        printf("Parsed URL: user=%s, password=%s, host=%s, path=%s\n", user, password, host, path);
    } else if (sscanf(url, "ftp://%99[^/]/%199[^
]", host, path) == 2) {
        strcpy(user, "anonymous");
        strcpy(password, "anonymous");
        printf("Parsed URL (anonymous): host=%s, path=%s\n", host, path);
    } else {
        fprintf(stderr, "Invalid URL format.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        puts("Usage: ./download <ftp_url>");
        return 1;
    }

    char user[MAX_STR_SIZE] = {0};
    char password[MAX_STR_SIZE] = {0};
    char host[MAX_STR_SIZE] = {0};
    char path[MAX_STR_SIZE] = {0};

    parse_url(argv[1], user, password, host, path);

    // Placeholder: Establish control connection (to be integrated with clientTCP.c)
    printf("Connecting to FTP server: %s\n", host);
    // Example: sockfd = connect_to_server(host, 21); (functionality to be implemented)

    // Placeholder: Send FTP commands (USER, PASS, PASV, RETR, etc.)
    printf("Authenticating as %s\n", user);
    // Example: send_ftp_command(sockfd, "USER", user);
    // Example: send_ftp_command(sockfd, "PASS", password);

    // Placeholder: Enter passive mode and download the file
    printf("Entering passive mode and retrieving file: %s\n", path);

    // Placeholder: Clean up connections
    printf("Closing connection\n");

    return 0;
}
